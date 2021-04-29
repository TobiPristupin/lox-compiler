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

class CallFrame {
public:
    CallFrame() = default;
    CallFrame(FunctionObj *function, int programCounter, int stackIndex);

    FunctionObj *function = nullptr;
    int programCounter = 0;
    int stackIndex = 0;
};

class VM {
public:

    ExecutionResult execute(FunctionObj *function);


private:
    std::vector<CLoxLiteral> stack;
    std::unordered_map<std::string, CLoxLiteral> globals;
    std::vector<CallFrame> callFrames;
    CallFrame currentFrame;

    Chunk *currentChunk();

    CLoxLiteral readConstant();
    StringObj* readConstantAsStringObj();
    ClassObj* readConstantAsClassObj();
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
    uint8_t readOneByteOffset();
    int readChunkLine(int offset);

    void runGC();

    void printDebugInfo(int offset);

    friend class Memory; //Memory.h defined in this project, not the standard <memory> module
};


#endif //CLOX_VM_H
