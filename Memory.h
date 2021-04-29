#ifndef CLOX_MEMORY_H
#define CLOX_MEMORY_H

#include <vector>
#include <list>
#include <unordered_map>
#include "CLoxLiteral.h"
#include "VM.h"

class Memory {
public:
    static std::vector<Obj*> newObjects;
    static std::vector<Obj*> oldObjects;
    static std::stack<Obj*> grayObjects;
    static size_t bytesAllocatedNewGen;
    static size_t bytesAllocatedOldGen;
    static size_t newGenGCByteThreshold;
    static size_t oldGenGCByteThreshold;
    static size_t newGenGrowFactor;
    static size_t oldGenGrowFactor;

    static Obj* allocateHeapString(std::string str, VM *vm = nullptr);
    static Obj* allocateHeapClass(StringObj *name, VM *vm = nullptr);
    static Obj* allocateHeapInstance(ClassObj *klass, VM *vm = nullptr);
    static Obj* allocateHeapFunction(StringObj *name, Chunk *chunk, int arity, VM *vm = nullptr);
    static Obj* allocateAllocationObject(size_t kilobytes);
    static void freeAllHeapObjects();
    static void collectGarbage(VM *vm = nullptr, bool force=false);

    static void markRoots(VM *vm = nullptr, bool collectOldGen=false);
    static void markObject(CLoxLiteral &obj, bool collectOldGen=false);
    static void markObject(Obj *obj, bool collectOldGen=false);
    static void traceReferences();
    static void blackenObject(Obj *obj);
    static void sweep(bool collectOldGen);

    static size_t calculateObjectSize(const Obj *obj);

private:
    static auto epochTime();
    //finalCleanup = true if this is the final object freeing when the program terminates, false if this is a regular gc cycle
    static void logDeallocation(const Obj *obj, bool finalCleanup = false);
    static void logAllocation(const Obj *obj);

};


#endif //CLOX_MEMORY_H
