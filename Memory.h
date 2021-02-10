#ifndef CLOX_MEMORY_H
#define CLOX_MEMORY_H

#include <vector>
#include "CLoxLiteral.h"

class Memory {
public:
    static std::vector<Obj*> heapObjects;

};


#endif //CLOX_MEMORY_H
