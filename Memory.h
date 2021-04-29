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
    static std::stack<Obj*> grayObjects;
    static size_t bytesAllocated;
    static size_t nextGCByteThreshold;
    static size_t heapGrowFactor;

    static Obj* allocateHeapString(std::string str, VM *vm = nullptr);
    static Obj* allocateHeapClass(StringObj *name, VM *vm = nullptr);
    static Obj* allocateHeapInstance(ClassObj *klass, VM *vm = nullptr);
    static Obj* allocateHeapFunction(StringObj *name, Chunk *chunk, int arity, VM *vm = nullptr);
    static Obj* allocateAllocationObject(size_t kilobytes);
    static void freeAllHeapObjects();
    static void collectGarbage(VM *vm = nullptr, bool force=false);
    static void markRoots(VM *vm = nullptr);
    static void markObject(CLoxLiteral &obj);
    static void markObject(Obj *obj);
    static void traceReferences();
    static void blackenObject(Obj *obj);
    static void sweep();

    static size_t calculateObjectSize(const Obj *obj);

private:
    static auto epochTime();
    //finalCleanup = true if this is the final object freeing when the program terminates, false if this is a regular gc cycle
    static void logDeallocation(const Obj *obj, bool finalCleanup = false);
    static void logAllocation(const Obj *obj);

};


#endif //CLOX_MEMORY_H
