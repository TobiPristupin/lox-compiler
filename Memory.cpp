#include <iostream>
#include <chrono>
#include "Memory.h"

//#define DEBUG_STRESS_GC //Run the GC after every allocation
#define DEBUG_LOG_GC

std::vector<Obj*> Memory::heapObjects = std::vector<Obj*>();
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

void Memory::collectGarbage(Obj *obj) {
#ifdef DEBUG_LOG_GC
    std::cout << "[DEBUG] Deleted " << CLoxLiteral(obj) << "\n";
#endif

    bytesAllocated -= calculateObjectSize(obj);
    heapObjects.erase(std::remove(heapObjects.begin(), heapObjects.end(), obj), heapObjects.end());
    decreaseRefCountOfNeighbors(obj);
    logDeallocation(obj);
    delete obj;
}

void Memory::decreaseRefCountOfNeighbors(Obj *obj) {
    const auto* instance = dynamic_cast<const InstanceObj*>(obj);
    if (!instance){
        return; //only instance objects can point to other objects in the heap
    }

    for (const auto &field : instance->fields){
        decrementRefCount(field.second);
    }

}

void Memory::incrementRefCount(const CLoxLiteral &value) {
    if (!value.isObj()){
        return;
    }

    Obj* o  = value.getObj();
    o->refs += 1;

#ifdef DEBUG_LOG_GC
    std::cout << "[DEBUG] Incremented refcount of " << value << " from " << o->refs-1 << " to " << o->refs << "\n";
#endif
}

void Memory::decrementRefCount(const CLoxLiteral &value) {
    if (!value.isObj()){
        return;
    }

    Obj* o  = value.getObj();
    o->refs -= 1;

#ifdef DEBUG_LOG_GC
    std::cout << "[DEBUG] Decremented refcount of " << value << " from " << o->refs+1 << " to " << o->refs << "\n";
#endif

    if (o->refs == 0){
        collectGarbage(o);
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

