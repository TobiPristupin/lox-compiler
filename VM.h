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
    std::vector<CLoxLiteral> stack;
    std::shared_ptr<Chunk> chunk;
    std::unordered_map<std::string, CLoxLiteral> globals;
    int programCounter = 0; //holds the index of the next instruction to be executed

    CLoxLiteral readConstant();
    void pushStack(const CLoxLiteral& val);
    CLoxLiteral popStack();

    Obj* allocateObject(const std::string &str);
    void freeHeapObjects();

    void add();
    void subtract();
    void multiply();
    void divide();
    void equal();
    void greater();
    void less();
    void negate();
    bool isTruthy();



    void printDebugInfo(int offset);
};


#endif //CLOX_VM_H
