#include <iostream>
#include "Chunk.h"
#include "DebugUtils.h"

int main() {
    Chunk mainChunk;
    mainChunk.writeInstruction(OpCode::OP_CONSTANT, 1);
    int offset = mainChunk.writeConstant(1.223);
    mainChunk.write(std::byte(offset), 1);
    mainChunk.writeInstruction(OpCode::OP_RETURN, 1);
    mainChunk.writeInstruction(OpCode::OP_CONSTANT, 2);
    offset = mainChunk.writeConstant(2.7);
    mainChunk.write(std::byte(offset), 2);
    mainChunk.writeInstruction(OpCode::OP_RETURN, 3);
    mainChunk.writeInstruction(OpCode::OP_RETURN, 3);
    mainChunk.writeInstruction(OpCode::OP_RETURN, 5);
    mainChunk.writeInstruction(OpCode::OP_PRINT, 8);

    DebugUtils::printChunk(mainChunk, "mainChunk");
}
