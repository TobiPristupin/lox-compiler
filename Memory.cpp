#include <iostream>
#include <chrono>
#include "Memory.h"

std::vector<Obj*> Memory::heapObjects = std::vector<Obj*>();

auto Memory::epochTime() {
    return std::chrono::system_clock::now().time_since_epoch().count();
}

Obj *Memory::allocateHeapString(std::string str) {
    auto *obj = new StringObj(std::move(str));
    std::clog << "Allocated string " << sizeof(*obj) + obj->str.size() * sizeof(std::string::value_type) << " " << epochTime() << "\n";
    heapObjects.push_back(obj);
    return obj;
}

Obj *Memory::allocateHeapFunction(std::string name, Chunk chunk, int arity) {
    auto *obj = new FunctionObj(std::make_unique<std::string>(std::move(name)), std::make_unique<Chunk>(std::move(chunk)), arity);
    heapObjects.push_back(obj);
    return obj;
}

Obj *Memory::allocateHeapClass(StringObj *name) {
    auto *obj = new ClassObj(name);
    std::clog << "Allocated class " << sizeof(*obj)  << " " << epochTime() << "\n";
    heapObjects.push_back(obj);
    return obj;
}

Obj *Memory::allocateHeapInstance(ClassObj *klass) {
    auto *obj = new InstanceObj(klass);
    std::clog << "Allocated instance " << sizeof(*obj) << " " << epochTime() << "\n";
    heapObjects.push_back(obj);
    return obj;
}

void Memory::freeAllHeapObjects() {
    for (Obj *obj : heapObjects){
        if (auto* str = dynamic_cast<StringObj*>(obj)){
            std::clog << "Deallocated string " << sizeof(*str) + str->str.size() * sizeof(std::string::value_type) << " " << epochTime() <<  "\n";
        }
        delete obj;
    }
}
