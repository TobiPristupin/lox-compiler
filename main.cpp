#include <iostream>
#include "Chunk.h"
#include "DebugUtils.h"

int main() {
    Chunk mainChunk;
    mainChunk.writeInstruction(OpCode::OP_CONSTANT, 1);
    int offset = mainChunk.writeConstant(1.223);
    mainChunk.write(std::byte(offset), 1);
    mainChunk.writeInstruction(OpCode::OP_RETURN, 2);
    mainChunk.writeInstruction(OpCode::OP_CONSTANT, 3);
    offset = mainChunk.writeConstant(2.7);
    mainChunk.write(std::byte(offset), 3);

    DebugUtils::printChunk(mainChunk, "mainChunk");
}
