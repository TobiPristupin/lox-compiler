


#include "Chunk.h"
#include "DebugUtils.h"
#include "VM.h"


int main() {
    VM vm;

    std::shared_ptr<Chunk> mainChunk = std::make_shared<Chunk>();
    mainChunk->writeInstruction(OpCode::OP_CONSTANT, 1);
    int offset = mainChunk->writeConstant(1.223);
    mainChunk->write(std::byte(offset), 1);
    mainChunk->writeInstruction(OpCode::OP_CONSTANT, 2);
    offset = mainChunk->writeConstant(2.7);
    mainChunk->write(std::byte(offset), 2);
    mainChunk->writeInstruction(OpCode::OP_PRINT, 3);
    mainChunk->writeInstruction(OpCode::OP_RETURN, 3);
    mainChunk->writeInstruction(OpCode::OP_PRINT, 4);


//    DebugUtils::printChunk(mainChunk.get(), "mainChunk");
    vm.execute(mainChunk);


}
