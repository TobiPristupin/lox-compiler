#include <iostream>
#include "VM.h"
#include "DebugUtils.h"

//when this macro is enabled, the VM will print every instruction before executing it
#define DEBUG_VM


ExecutionResult VM::execute(std::shared_ptr<Chunk> chunk) {
    programCounter = 0;
    this->chunk = chunk;

    while (true){
        //keep track of the current offset before we modify it in case the DEBUG flag is on and we want to debug print info about
        //the last executed instruction.
        [[maybe_unused]] int currentOffset = programCounter;
        std::byte instruction = chunk->readByte(programCounter);
        programCounter++;

        switch (static_cast<OpCode>(instruction)) {
            case OpCode::OP_RETURN:
                return ExecutionResult::OK;
            case OpCode::OP_PRINT:
                std::cout << stack.top() << "\n";
                break;
            case OpCode::OP_CONSTANT:
                pushStack(readConstant());
                break;
            case OpCode::OP_NEGATE:
                pushStack(-popStack());
                break;
            case OpCode::OP_ADD:
                pushStack(popStack() + popStack());
                break;
            case OpCode::OP_SUBTRACT:
            {
                Value b = popStack();
                Value a = popStack();
                pushStack(a - b);
            }
                break;
            case OpCode::OP_MULTIPLY:
                pushStack(popStack() * popStack());
                break;
            case OpCode::OP_DIVIDE:
            {
                Value b = popStack();
                Value a = popStack();
                pushStack(a / b);
            }
            break;
        }

#ifdef DEBUG_VM
        printDebugInfo(currentOffset);
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
    std::cout << "[DEBUG]";
    std::cout << "\tInstruction: ";
    DebugUtils::printInstruction(offset, chunk.get());
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




