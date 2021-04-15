#include <iostream>
#include <chrono>
#include "Memory.h"

//#define DEBUG_STRESS_GC //Run the GC after every allocation
#define DEBUG_LOG_GC
//Reset every object to white after the marking phase so every GC cycle starts clean. Has no effect on a stop the world
//collector because marked objects will be freed, but can be useful for debugging.
//#define UNMARK_OBJECTS

std::vector<Obj*> Memory::heapObjects = std::vector<Obj*>();
std::stack<Obj*> Memory::grayObjects = std::stack<Obj*>();
size_t Memory::nextGCByteThreshold = 200;
size_t Memory::heapGrowFactor = 1;
size_t Memory::bytesAllocated = 0;

auto Memory::epochTime() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

Obj *Memory::allocateHeapFunction(StringObj *name, Chunk *chunk, int arity, VM *vm) {
#ifdef DEBUG_STRESS_GC
    collectGarbage(vm);
#endif

    auto *obj = new FunctionObj(name, chunk, arity);
    bytesAllocated += calculateObjectSize(obj);
    logAllocation(obj);

    heapObjects.push_back(obj);
    return obj;
}


Obj *Memory::allocateHeapString(std::string str, VM *vm) {
#ifdef DEBUG_STRESS_GC
    collectGarbage(vm);
#endif

    auto *obj = new StringObj(std::move(str));
    bytesAllocated += calculateObjectSize(obj);
    logAllocation(obj);

#ifdef DEBUG_LOG_GC
    std::cout << "[DEBUG] Allocated string " <<  calculateObjectSize(obj) << " " << epochTime() << "\n";
#endif

    heapObjects.push_back(obj);
    return obj;
}

Obj *Memory::allocateHeapClass(StringObj *name, VM *vm) {
#ifdef DEBUG_STRESS_GC
    collectGarbage(vm);
#endif

    auto *obj = new ClassObj(name);
    bytesAllocated += calculateObjectSize(obj);
    logAllocation(obj);

#ifdef DEBUG_LOG_GC
    std::cout << "[DEBUG] Allocated class " <<  calculateObjectSize(obj)  << " " << epochTime() << "\n";
#endif

    heapObjects.push_back(obj);
    return obj;
}

Obj *Memory::allocateHeapInstance(ClassObj *klass, VM *vm) {
#ifdef DEBUG_STRESS_GC
    collectGarbage(vm);
#endif

    auto *obj = new InstanceObj(klass);
    bytesAllocated += calculateObjectSize(obj);
    logAllocation(obj);

#ifdef DEBUG_LOG_GC
    std::cout << "[DEBUG] Allocated instance " <<  calculateObjectSize(obj) << " " << epochTime() << "\n";
#endif

    heapObjects.push_back(obj);
    return obj;
}

Obj *Memory::allocateAllocationObject(size_t kilobytes) {
#ifdef DEBUG_STRESS_GC
    collectGarbage(vm);
#endif

    char* memoryBlock = new char[kilobytes * 1024];
    auto *obj = new AllocationObj(kilobytes, memoryBlock);
    bytesAllocated += calculateObjectSize(obj);
    logAllocation(obj);

#ifdef DEBUG_LOG_GC
    std::cout << "[DEBUG] Allocated allocation " <<  calculateObjectSize(obj) << " " << bytesAllocated << " " << epochTime() << "\n";
#endif

    heapObjects.push_back(obj);
    return obj;
}

void Memory::freeAllHeapObjects() {
    for (Obj *obj : heapObjects){
        bytesAllocated -= calculateObjectSize(obj);
        logDeallocation(obj, true);
        delete obj;
    }
}

void Memory::collectGarbage(VM *vm) {
#ifdef DEBUG_LOG_GC
    std::cout << "[DEBUG] GC begin\n";
#endif

    if (vm == nullptr){
#ifdef DEBUG_LOG_GC
        std::cout << "[DEBUG] GC end\n";
#endif
        return;
    }

    markRoots(vm);
    traceReferences();
    sweep();
    nextGCByteThreshold = bytesAllocated * heapGrowFactor;

#ifdef UNMARK_OBJECTS
    for (Obj *obj : heapObjects){
        obj->marked = false;
    }
#endif

#ifdef DEBUG_LOG_GC
    std::cout << "[DEBUG] GC end\n";
#endif

}

void Memory::markRoots(VM *vm) {
    for (CLoxLiteral &obj : vm->stack){
        markObject(obj);
    }

    for (auto it = vm->globals.begin(); it != vm->globals.end(); it++){
        markObject(it->second);
    }
}

void Memory::traceReferences() {
    while (!grayObjects.empty()){
        Obj *obj = grayObjects.top();
        grayObjects.pop();

        blackenObject(obj);
    }
}

void Memory::markObject(CLoxLiteral &literal) {
    if (!literal.isObj()){ //value in the stack not an object. Could be an int or boolean for example
        return;
    }

    Obj* obj = literal.getObj();
    if (obj == nullptr){ //literal will not always contain an object
        return;
    }

    markObject(obj);
}

void Memory::markObject(Obj *obj) {
    if (obj->marked){
        return; //Avoid cycles
    }

    obj->marked = true;
    grayObjects.push(obj);

#ifdef DEBUG_LOG_GC
    std::cout << "[DEBUG] Marking object " << CLoxLiteral(obj) << " address " << obj << "\n";
#endif
}


void Memory::blackenObject(Obj *obj) {
#ifdef DEBUG_LOG_GC
    std::cout << "[DEBUG] Blackened object " << CLoxLiteral(obj) << " address " << obj << "\n";
#endif
    switch (obj->type) {
        case ObjType::STRING:
            break;
        case ObjType::FUNCTION: {
            auto *function = dynamic_cast<FunctionObj *>(obj);
            markObject(function->name);
            for (CLoxLiteral &literal : function->chunk->constants){
                markObject(literal);
            }
            break;
        }
        case ObjType::CLASS: {
            auto *klass = dynamic_cast<ClassObj*>(obj);
            markObject(klass->name);
            break;
        }
        case ObjType::INSTANCE: {
            auto *instance = dynamic_cast<InstanceObj *>(obj);
            markObject(instance->klass);
            for (auto it = instance->fields.begin(); it != instance->fields.end(); it++) {
                markObject(it->second);
            }
            break;
        }
        case ObjType::ALLOCATION:
            break;
    }
}

void Memory::sweep() {
    auto it = heapObjects.begin();
    while (it != heapObjects.end()){
        Obj *obj = *it;
        if (obj->marked) {
            obj->marked = false;
            ++it;
        } else {
#ifdef DEBUG_LOG_GC
            std::cout << "[DEBUG] Sweeped " << CLoxLiteral(obj) << " address " << obj << "\n";
#endif
            bytesAllocated -= calculateObjectSize(obj);
            logDeallocation(obj);
            it = heapObjects.erase(it); //constant time because we are using a linked list
            delete obj;
        }
    }
}



/*Calculates estimates of the memory usage of every object. Their implementations depend on each other to avoid
 * double counting. For example, the size of an instance on the heap is the size instance object itself + the size of the
 * heap allocated fields of the instance. The size of those heap allocated objects is already accounted for, because the corresponding
 * calculateObjectSize has already been called and accounted for when the VM allocated that object (and before adding it
 * to the instance), so we shouldn't count the size of that field again when calculating the size of the instance object.
 * */
size_t Memory::calculateObjectSize(const Obj *obj) {
    if (const auto* str = dynamic_cast<const StringObj*>(obj)){
        return sizeof(*str) + str->str.size() * sizeof(std::string::value_type);
    } else if (const auto* klass = dynamic_cast<const ClassObj*>(obj)) {
        return sizeof(*klass);
    } else if (const auto* instance = dynamic_cast<const InstanceObj*>(obj)) {
        return sizeof(*instance);
    } else if (const auto* function = dynamic_cast<const FunctionObj*>(obj)) {
        return sizeof(*function);
    } else if (const auto* allocation = dynamic_cast<const AllocationObj*>(obj)) {
        return allocation->kilobytes * 1024;
    }

    throw std::runtime_error("Unreachable");
}

void Memory::logAllocation(const Obj *obj) {
    if (const auto* str = dynamic_cast<const StringObj*>(obj)){
        std::clog << "Allocated string " << str->str << " " <<  calculateObjectSize(str)  << " " << bytesAllocated << " " << epochTime() << "\n";
    } else if (const auto* klass = dynamic_cast<const ClassObj*>(obj)) {
        std::clog << "Allocated class " << klass->name->str << " " <<   calculateObjectSize(klass)  << " " << bytesAllocated << " " << epochTime() << "\n";
    } else if (const auto* instance = dynamic_cast<const InstanceObj*>(obj)) {
        std::clog << "Allocated instance " << instance->klass->name->str << " " <<  calculateObjectSize(instance) << " " << bytesAllocated << " " << epochTime() << "\n";
    } else if (const auto* function = dynamic_cast<const FunctionObj*>(obj)) {
        std::clog << "Allocated function " << function->name->str << " " << calculateObjectSize(function)  << " " << bytesAllocated << " " << epochTime() << "\n";
    } else if (const auto* allocation = dynamic_cast<const AllocationObj*>(obj)) {
        std::clog << "Allocated allocation [noname] " << calculateObjectSize(allocation)  << " " << bytesAllocated << " "  <<  epochTime() << "\n";
    }
}

void Memory::logDeallocation(const Obj *obj, bool finalCleanup) {
    //sweeped: GC collected. Deallocated: freed when program terminates (not by gc)
    std::string message = finalCleanup ? "Deallocated" : "Sweeped";

    if (const auto* str = dynamic_cast<const StringObj*>(obj)){
        std::clog << message << "  string " << str->str << " " <<  calculateObjectSize(str) << " " << bytesAllocated << " " << epochTime() << "\n";
    } else if (const auto* klass = dynamic_cast<const ClassObj*>(obj)) {
        std::clog << message << " class " << klass->name->str << " " << calculateObjectSize(klass) << " " << bytesAllocated << " " << epochTime() << "\n";
    } else if (const auto* instance = dynamic_cast<const InstanceObj*>(obj)) {
        std::clog << message << " instance " << instance->klass->name->str << " " << calculateObjectSize(instance) << " " << bytesAllocated << " " << epochTime() << "\n";
    } else if (const auto* function = dynamic_cast<const FunctionObj*>(obj)) {
        std::clog << message << " function [noname] " << calculateObjectSize(function)  << " " << bytesAllocated << " " <<  epochTime() << "\n";
    } else if (const auto* allocation = dynamic_cast<const AllocationObj*>(obj)) {
        std::clog << message << " allocation [noname] " << calculateObjectSize(allocation)  << " " << bytesAllocated << " " <<  epochTime() << "\n";
    }
}


