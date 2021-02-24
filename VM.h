#ifndef CLOX_VM_H
#define CLOX_VM_H


#include <memory>
#include <stack>
#include <functional>
#include "Chunk.h"
#include "CLoxLiteral.h"

enum class ExecutionResult {
    OK,
    COMPILE_ERROR,
    RUNTIME_ERROR
};

class VM {
public:
    ExecutionResult execute(Chunk *chunk);


private:
    std::vector<CLoxLiteral> stack;
    Chunk *chunk;
    std::unordered_map<std::string, CLoxLiteral> globals;
    int programCounter = 0; //holds the index of the next instruction to be executed

    CLoxLiteral readConstant();
    StringObj* readConstantAsStringObj();
    void pushStack(const CLoxLiteral& val);
    CLoxLiteral popStack();

    void add();
    void subtract();
    void multiply();
    void divide();
    void equal();
    void greater();
    void less();
    void negate();
    bool isTruthy(const CLoxLiteral &literal);

    void defineGlobal();
    void getGlobal();
    void setGlobal();
    void setLocal();
    void getLocal();

    uint16_t readTwoByteOffset();



    void printDebugInfo(int offset);


};


#endif //CLOX_VM_H
