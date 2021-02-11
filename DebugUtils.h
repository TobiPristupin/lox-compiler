#ifndef CLOX_DEBUGUTILS_H
#define CLOX_DEBUGUTILS_H


#include "Chunk.h"

namespace DebugUtils {

    void printChunk(const Chunk *chunk, const std::string &name);
    //prints instruction and returns offset of next instruction
    int printInstruction(int offset, const Chunk *chunk);
    void constantInstruction(const std::string& instructionName, int offset, const Chunk *chunk);

}

#endif //CLOX_DEBUGUTILS_H
