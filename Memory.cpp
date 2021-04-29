#include <iostream>
#include <chrono>
#include "Memory.h"

//#define DEBUG_STRESS_GC //Run the GC after every allocation
#define DEBUG_LOG_GC
//Reset every object to white after the marking phase so every GC cycle starts clean. Has no effect on a stop the world
//collector because marked objects will be freed, but can be useful for debugging.
//#define UNMARK_OBJECTS

std::vector<Obj*> Memory::newObjects = std::vector<Obj*>();
std::vector<Obj*> Memory::oldObjects = std::vector<Obj*>();
std::stack<Obj*> Memory::grayObjects = std::stack<Obj*>();
size_t Memory::bytesAllocatedNewGen = 0;
size_t Memory::bytesAllocatedOldGen = 0;
size_t Memory::newGenGCByteThreshold = 1024;
size_t Memory::oldGenGCByteThreshold = 2048;
size_t Memory::newGenGrowFactor = 1;
size_t Memory::oldGenGrowFactor = 1;

auto Memory::epochTime() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

Obj *Memory::allocateHeapFunction(StringObj *name, Chunk *chunk, int arity, VM *vm) {
#ifdef DEBUG_STRESS_GC
    collectGarbage(vm);
#endif

    auto *obj = new FunctionObj(name, chunk, arity);
    bytesAllocatedNewGen += calculateObjectSize(obj);
    logAllocation(obj);

    newObjects.push_back(obj);
    return obj;
}


Obj *Memory::allocateHeapString(std::string str, VM *vm) {
#ifdef DEBUG_STRESS_GC
    collectGarbage(vm);
#endif

    auto *obj = new StringObj(std::move(str));
    bytesAllocatedNewGen += calculateObjectSize(obj);
    logAllocation(obj);

#ifdef DEBUG_LOG_GC
    std::cout << "[DEBUG] Allocated string " <<  calculateObjectSize(obj) << " " << epochTime() << "\n";
#endif

    newObjects.push_back(obj);
    return obj;
}

Obj *Memory::allocateHeapClass(StringObj *name, VM *vm) {
#ifdef DEBUG_STRESS_GC
    collectGarbage(vm);
#endif

    auto *obj = new ClassObj(name);
    bytesAllocatedNewGen += calculateObjectSize(obj);
    logAllocation(obj);

#ifdef DEBUG_LOG_GC
    std::cout << "[DEBUG] Allocated class " <<  calculateObjectSize(obj)  << " " << epochTime() << "\n";
#endif

    newObjects.push_back(obj);
    return obj;
}

Obj *Memory::allocateHeapInstance(ClassObj *klass, VM *vm) {
#ifdef DEBUG_STRESS_GC
    collectGarbage(vm);
#endif

    auto *obj = new InstanceObj(klass);
    bytesAllocatedNewGen += calculateObjectSize(obj);
    logAllocation(obj);

#ifdef DEBUG_LOG_GC
    std::cout << "[DEBUG] Allocated instance " <<  calculateObjectSize(obj) << " " << epochTime() << "\n";
#endif

    newObjects.push_back(obj);
    return obj;
}

Obj *Memory::allocateAllocationObject(size_t kilobytes) {
#ifdef DEBUG_STRESS_GC
    collectGarbage(vm);
#endif

    char* memoryBlock = new char[kilobytes * 1024];
    auto *obj = new AllocationObj(kilobytes, memoryBlock);
    bytesAllocatedNewGen += calculateObjectSize(obj);
    logAllocation(obj);

#ifdef DEBUG_LOG_GC
    std::cout << "[DEBUG] Allocated allocation " <<  calculateObjectSize(obj)  << " " << epochTime() << "\n";
#endif

    newObjects.push_back(obj);
    return obj;
}

void Memory::freeAllHeapObjects() {
    for (Obj *obj : newObjects){
        bytesAllocatedNewGen -= calculateObjectSize(obj);
        logDeallocation(obj, true);
        delete obj;
    }

    for (Obj *obj : oldObjects){
        bytesAllocatedOldGen -= calculateObjectSize(obj);
        logDeallocation(obj, true);
        delete obj;
    }
}

/* A collection of old objects will always include a collection of new objects, but a collection of new objects
 * will not always come with a collection of old objects
 * */
void Memory::collectGarbage(VM *vm, bool force) {
    bool collectOldGen = bytesAllocatedOldGen > oldGenGCByteThreshold;
    bool collectNewGen = bytesAllocatedNewGen > newGenGCByteThreshold;

    if (!collectNewGen && !collectOldGen){
        return;
    }

    if (vm == nullptr){
        return;
    }

#ifdef DEBUG_LOG_GC
    std::cout << "[DEBUG] GC begin\n";
#endif

    markRoots(vm, collectOldGen);
    traceReferences();
    sweep(collectOldGen);
//    nextGCByteThreshold = bytesAllocated * heapGrowFactor;

#ifdef UNMARK_OBJECTS
    for (Obj *obj : heapObjects){
        obj->marked = false;
    }
#endif

#ifdef DEBUG_LOG_GC
    std::cout << "[DEBUG] GC end\n";
#endif

}

void Memory::markRoots(VM *vm, bool collectOldGen) { //TODO: also iterate through old gen
    for (CLoxLiteral &obj : vm->stack){
        markObject(obj, collectOldGen);
    }

    for (auto it = vm->globals.begin(); it != vm->globals.end(); it++){
        markObject(it->second, collectOldGen);
    }
}

void Memory::traceReferences() {
    while (!grayObjects.empty()){
        Obj *obj = grayObjects.top();
        grayObjects.pop();

        blackenObject(obj);
    }
}

void Memory::markObject(CLoxLiteral &literal, bool collectOldGen) {
    if (!literal.isObj()){ //value in the stack not an object. Could be an int or boolean for example
        return;
    }

    Obj* obj = literal.getObj();
    if (obj == nullptr){ //literal will not always contain an object
        return;
    }

    markObject(obj, collectOldGen);
}

void Memory::markObject(Obj *obj, bool collectOldGen) {
    if (obj->marked){
        return; //Avoid cycles
    }

    if (!collectOldGen && obj->age >= 2){
        return;
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

void Memory::sweep(bool collectOldGen) {
    auto it = newObjects.begin();
    while (it != newObjects.end()){
        Obj *obj = *it;
        if (obj->marked) {
            obj->marked = false;
            obj->age += 1;

            if (obj->age >= 2){
                bytesAllocatedNewGen -= calculateObjectSize(obj);
                it = newObjects.erase(it);
                oldObjects.push_back(obj);
                bytesAllocatedOldGen += calculateObjectSize(obj);
            } else {
                ++it;
            }
        } else {
#ifdef DEBUG_LOG_GC
            std::cout << "[DEBUG] Sweeped " << CLoxLiteral(obj) << " address " << obj << "\n";
#endif
            bytesAllocatedNewGen -= calculateObjectSize(obj);
            logDeallocation(obj);
            it = newObjects.erase(it);
            delete obj;
        }
    }

    if (collectOldGen){
        auto it = oldObjects.begin();
        while (it != oldObjects.end()){
            Obj *obj = *it;
            if (obj->marked) {
                obj->marked = false;
                ++it;
            } else {
#ifdef DEBUG_LOG_GC
                std::cout << "[DEBUG] Sweeped " << CLoxLiteral(obj) << " address " << obj << "\n";
#endif
                bytesAllocatedOldGen -= calculateObjectSize(obj);
                logDeallocation(obj);
                it = oldObjects.erase(it);
                delete obj;
            }
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
        std::clog << "Allocated string " << str->str << " " <<  calculateObjectSize(str)  << " " << bytesAllocatedNewGen + bytesAllocatedOldGen << " " << epochTime() << "\n";
    } else if (const auto* klass = dynamic_cast<const ClassObj*>(obj)) {
        std::clog << "Allocated class " << klass->name->str << " " <<   calculateObjectSize(klass)  << " " << bytesAllocatedNewGen + bytesAllocatedOldGen << " " << epochTime() << "\n";
    } else if (const auto* instance = dynamic_cast<const InstanceObj*>(obj)) {
        std::clog << "Allocated instance " << instance->klass->name->str << " " <<  calculateObjectSize(instance) << " " << bytesAllocatedNewGen + bytesAllocatedOldGen << " " << epochTime() << "\n";
    } else if (const auto* function = dynamic_cast<const FunctionObj*>(obj)) {
        std::clog << "Allocated function " << function->name->str << " " << calculateObjectSize(function)  << " " << bytesAllocatedNewGen + bytesAllocatedOldGen << " " << epochTime() << "\n";
    } else if (const auto* allocation = dynamic_cast<const AllocationObj*>(obj)) {
        std::clog << "Allocated allocation [noname] " << calculateObjectSize(allocation)  << " " << bytesAllocatedNewGen + bytesAllocatedOldGen << " "  <<  epochTime() << "\n";
    }
}

void Memory::logDeallocation(const Obj *obj, bool finalCleanup) {
    //sweeped: GC collected. Deallocated: freed when program terminates (not by gc)
    std::string message = finalCleanup ? "Deallocated" : "Sweeped";

    if (const auto* str = dynamic_cast<const StringObj*>(obj)){
        std::clog << message << "  string " << str->str << " " <<  calculateObjectSize(str) << " " << bytesAllocatedNewGen + bytesAllocatedOldGen << " " << epochTime() << "\n";
    } else if (const auto* klass = dynamic_cast<const ClassObj*>(obj)) {
        std::clog << message << " class " << klass->name->str << " " << calculateObjectSize(klass) << " " << bytesAllocatedNewGen + bytesAllocatedOldGen << " " << epochTime() << "\n";
    } else if (const auto* instance = dynamic_cast<const InstanceObj*>(obj)) {
        std::clog << message << " instance " << instance->klass->name->str << " " << calculateObjectSize(instance) << " " << bytesAllocatedNewGen + bytesAllocatedOldGen << " " << epochTime() << "\n";
    } else if (const auto* function = dynamic_cast<const FunctionObj*>(obj)) {
        std::clog << message << " function [noname] " << calculateObjectSize(function)  << " " << bytesAllocatedNewGen + bytesAllocatedOldGen << " " <<  epochTime() << "\n";
    } else if (const auto* allocation = dynamic_cast<const AllocationObj*>(obj)) {
        std::clog << message << " allocation [noname] " << calculateObjectSize(allocation)  << " " << bytesAllocatedNewGen + bytesAllocatedOldGen << " " <<  epochTime() << "\n";
    }
}


