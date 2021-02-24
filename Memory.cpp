//
// Created by pristu on 2/9/21.
//

#include "Memory.h"

std::vector<Obj*> Memory::heapObjects = std::vector<Obj*>();

Obj *Memory::allocateHeapString(std::string str) {
    auto *obj = new StringObj(std::make_unique<std::string>(std::move(str)));
    heapObjects.push_back(obj);
    return obj;
}

Obj *Memory::allocateHeapFunction(std::string name, Chunk chunk, int arity) {
    auto *obj = new FunctionObj(std::make_unique<std::string>(std::move(name)), std::make_unique<Chunk>(std::move(chunk)), arity);
    heapObjects.push_back(obj);
    return obj;
}

void Memory::freeAllHeapObjects() {
    for (Obj *obj : heapObjects){
        delete obj;
    }
}
