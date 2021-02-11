#include <iostream>
#include <bitset>
#include "DebugUtils.h"

void DebugUtils::printChunk(const Chunk *chunk, const std::string &name) {
    std::cout << "Chunk: " << name << "\n";
    int offset = 0;
    while (offset < chunk->byteCount()){
        offset = printInstruction(offset, chunk);
    }
}

void DebugUtils::constantInstruction(const std::string& instructionName, int offset, const Chunk *chunk) {
    std::cout << instructionName << " ";
    int constantOffset = (int) chunk->readByte(offset + 1);
    std::cout << chunk->readConstant(constantOffset) << "\n";
}


//Prints instruction and returns the offset of next instruction. Instructions are not always one byte
//(constants are 2 bytes for example) so this function takes care of that.
int DebugUtils::printInstruction(int offset, const Chunk *chunk) {
    std::byte byteInstruction = chunk->readByte(offset);
    int lineNumber = chunk->readLine(offset);
    std::cout << lineNumber << " ";
    auto opcode = static_cast<OpCode>(byteInstruction);
    switch (opcode) {
        case OpCode::OP_CONSTANT:
            constantInstruction("OP_CONSTANT", offset, chunk);
            return offset + 2;
        case OpCode::OP_RETURN:
            std::cout << "OP_RETURN\n";
            return offset + 1;
        case OpCode::OP_PRINT:
            std::cout << "OP_PRINT\n";
            return offset + 1;
        case OpCode::OP_NEGATE:
            std::cout << "OP_NEGATE\n";
            return offset + 1;
        case OpCode::OP_ADD:
            std::cout << "OP_ADD\n";
            return offset + 1;
        case OpCode::OP_SUBTRACT:
            std::cout << "OP_SUBTRACT\n";
            return offset + 1;
        case OpCode::OP_MULTIPLY:
            std::cout << "OP_MULTIPLY\n";
            return offset + 1;
        case OpCode::OP_DIVIDE:
            std::cout << "OP_DIVIDE\n";
            return offset + 1;
        case OpCode::OP_TRUE:
            std::cout << "OP_TRUE\n";
            return offset + 1;
        case OpCode::OP_FALSE:
            std::cout << "OP_FALSE\n";
            return offset + 1;
        case OpCode::OP_NIL:
            std::cout << "OP_NIL\n";
            return offset + 1;
        case OpCode::OP_NOT:
            std::cout << "OP_NOT\n";
            return offset + 1;
        case OpCode::OP_EQUAL:
            std::cout << "OP_EQUAL\n";
            return offset + 1;
        case OpCode::OP_GREATER:
            std::cout << "OP_GREATER\n";
            return offset + 1;
        case OpCode::OP_LESS:
            std::cout << "OP_LESS\n";
            return offset + 1;
        case OpCode::OP_POP:
            std::cout << "OP_POP\n";
            return offset + 1;
        case OpCode::OP_DEFINE_GLOBAL:
            constantInstruction("OP_DEFINE_GLOBAL", offset, chunk);
            return offset + 2;
        default:
            std::cout << "UNKNOWN\n";
            return offset + 1;
    }

}