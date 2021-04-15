#ifndef CLOX_MEMORY_H
#define CLOX_MEMORY_H

#include <vector>
#include <list>
#include <unordered_map>
#include "CLoxLiteral.h"
#include "VM.h"

class Memory {
public:
    static std::vector<Obj*> heapObjects;
    static size_t bytesAllocated;

    static Obj* allocateHeapString(std::string str, VM *vm = nullptr);
    static Obj* allocateHeapClass(StringObj *name, VM *vm = nullptr);
    static Obj* allocateHeapInstance(ClassObj *klass, VM *vm = nullptr);
    static Obj* allocateHeapFunction(StringObj *name, Chunk *chunk, int arity, VM *vm = nullptr);
    static Obj* allocateAllocationObject(size_t kilobytes);
    static void freeAllHeapObjects();
    static void collectGarbage(Obj* obj);

    /*not necessary for CloxLiteral value to be an Obj literal allocated in the heap, they may be a CloxLiteral holding
     * an int or bool for example. These method will handle those cases gracefully by not updating their refcount
     * (because they have no refcount)
     */
    static void incrementRefCount(const CLoxLiteral &value);
    static void decrementRefCount(const CLoxLiteral &value);

    static size_t calculateObjectSize(const Obj *obj);

private:
    static void decreaseRefCountOfNeighbors(Obj *obj);

    static auto epochTime();
    static void logDeallocation(const Obj *obj, bool finalCleanup = false);
    static void logAllocation(const Obj *obj);

};


#endif //CLOX_MEMORY_H
