#ifndef CLOX_MEMORY_H
#define CLOX_MEMORY_H

#include <vector>
#include "CLoxLiteral.h"

class Memory {
public:
    static std::vector<Obj*> heapObjects;
    static Obj* allocateHeapString(std::string str);
    static Obj* allocateHeapClass(StringObj *name);
    static Obj* allocateHeapInstance(ClassObj *klass);
    static Obj* allocateHeapFunction(std::string name, Chunk chunk, int arity);
    static void freeAllHeapObjects();

private:
    static auto epochTime();

};


#endif //CLOX_MEMORY_H
