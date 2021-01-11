#include <iostream>
#include "VM.h"
#include "DebugUtils.h"

//when this macro is enabled, the VM will print every instruction before executing it
#define DEBUG


ExecutionResult VM::execute(std::shared_ptr<Chunk> chunk) {
    programCounter = 0;
    this->chunk = chunk;

    while (true){
        std::byte instruction = chunk->readByte(programCounter);
        programCounter++;

        switch (static_cast<OpCode>(instruction)) {
            case OpCode::OP_RETURN:
                return ExecutionResult::OK;
            case OpCode::OP_PRINT:
                std::cout << popStack() << "\n";
                break;
            case OpCode::OP_CONSTANT:
                Value constant = readConstant();
                pushStack(constant);
                break;
        }

#ifdef DEBUG
        printDebugInfo(programCounter - 1); //programCounter - 1 because we want to print debug info for the last executed instruction
#endif
    }
    return ExecutionResult::OK;
}

Value VM::readConstant() {
    int constantOffset = (int) chunk->readByte(programCounter);
    programCounter++;
    return chunk->readConstant(constantOffset);
}

void VM::pushStack(Value val) {
    stack.push(val);
}

Value VM::popStack() {
    Value val = stack.top();
    stack.pop();
    return val;
}

void VM::printDebugInfo(int offset) {
    std::cout << "[DEBUG] \n";
    std::cout << "\tInstruction: ";
    DebugUtils::printInstruction(offset - 1, chunk.get());
    std::cout << "\tStack: [";
    std::stack<Value> copy(stack);

    while (!copy.empty()){
        std::cout << copy.top();
        copy.pop();

        if (copy.size() != 0){
            std::cout << ", ";
        }
    }

    std::cout << "]\n";
}




