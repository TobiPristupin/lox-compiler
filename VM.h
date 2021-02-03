#ifndef CLOX_VM_H
#define CLOX_VM_H


#include <memory>
#include <stack>
#include <functional>
#include "Chunk.h"

enum class ExecutionResult {
    OK,
    COMPILE_ERROR,
    RUNTIME_ERROR
};

class VM {
public:
    ExecutionResult execute(std::shared_ptr<Chunk> chunk);


private:
    std::stack<CLoxLiteral> stack;
    std::shared_ptr<Chunk> chunk;
    int programCounter = 0; //holds the index of the next instruction to be exeuted

    CLoxLiteral readConstant();
    void pushStack(CLoxLiteral val);
    CLoxLiteral popStack();





    void printDebugInfo(int offset);
};


#endif //CLOX_VM_H
