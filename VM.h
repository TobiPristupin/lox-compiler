#ifndef CLOX_VM_H
#define CLOX_VM_H


#include <memory>
#include <stack>
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
    std::stack<Value> stack;
    std::shared_ptr<Chunk> chunk;
    int programCounter = 0; //holds the index of the next instruction to be exeuted

    Value readConstant();
    void pushStack(Value val);
    Value popStack();


    void printDebugInfo(int offset);
};


#endif //CLOX_VM_H
