#ifndef CLOX_CHUNK_H
#define CLOX_CHUNK_H

#include <vector>
#include <cstdint>
#include <string>

class CLoxLiteral;


enum class OpCode  : uint8_t { //opcodes are internally represented as unsigned 8-bit integers
    OP_RETURN,
    OP_PRINT,
    OP_CONSTANT,
    OP_NEGATE,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_TRUE,
    OP_FALSE,
    OP_NIL,
    OP_NOT,
    OP_EQUAL,
    OP_GREATER,
    OP_LESS,
    OP_POP,
    OP_DEFINE_GLOBAL,
    OP_GET_GLOBAL,
    OP_SET_GLOBAL,
    OP_GET_LOCAL,
    OP_SET_LOCAL,
    OP_JUMP_IF_FALSE,
    OP_JUMP,
    OP_LOOP,
    OP_CLASS,
    OP_CALL,
    OP_GET_PROPERTY,
    OP_SET_PROPERTY,
    OP_ALLOCATE,
    OP_COLLECT
};

class Chunk {
public:
    //writes an instruction to the chunk and returns the offset it was added at
    size_t write(std::byte code, int line);
    size_t writeInstruction(OpCode code, int line);

    std::byte readByte(int offset) const;
    size_t byteCount() const;

    //writes a constant to the chunk and returns the offset it was added at
    size_t writeConstant(const CLoxLiteral &value);

    CLoxLiteral readConstant(int offset) const;
    size_t constantCount() const;

    //writes a line to the chunk
    void writeLine(int line);

    int readLine(int offset) const;
    size_t lineCount() const;

    std::vector<std::byte> bytecode;
    std::vector<CLoxLiteral> constants;
    std::vector<int> lines;
};


#endif //CLOX_CHUNK_H
