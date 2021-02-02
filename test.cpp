#include <cstddef>
#include <cstdio>
#include <iostream>
#include <bitset>

enum class OpCode  : uint8_t { //opcodes are internally represented as unsigned 8-bit integers
    OP_RETURN,
    OP_PRINT,
    OP_CONSTANT,
    OP_NEGATE,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE
};


int main(){
    std::byte b = static_cast<std::byte>(OpCode::OP_MULTIPLY);



}
