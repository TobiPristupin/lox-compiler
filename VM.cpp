#include <iostream>
#include "VM.h"
#include "DebugUtils.h"
#include "LoxError.h"

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

        try {
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
                    CLoxLiteral b = popStack();
                    CLoxLiteral a = popStack();
                    pushStack(a - b);
                }
                    break;
                case OpCode::OP_MULTIPLY:
                    pushStack(popStack() * popStack());
                    break;
                case OpCode::OP_DIVIDE:
                {
                    CLoxLiteral b = popStack();
                    CLoxLiteral a = popStack();
                    pushStack(a / b);
                }
                    break;
                case OpCode::OP_TRUE:
                    pushStack(CLoxLiteral(true));
                    break;
                case OpCode::OP_FALSE:
                    pushStack(CLoxLiteral(false));
                    break;
                case OpCode::OP_NIL:
                    pushStack(CLoxLiteral::Nil());
                    break;
                case OpCode::OP_NOT:
                    pushStack(CLoxLiteral(!popStack().truthy()));
                    break;
                case OpCode::OP_EQUAL:
                    pushStack(popStack() == popStack());
                    break;
                case OpCode::OP_GREATER:
                {
                    CLoxLiteral b = popStack();
                    CLoxLiteral a = popStack();
                    pushStack(CLoxLiteral(a > b));
                }
                    break;
                case OpCode::OP_LESS:
                {
                    CLoxLiteral b = popStack();
                    CLoxLiteral a = popStack();
                    pushStack(CLoxLiteral(a < b));
                }
                    break;
            }
        } catch (const std::runtime_error &error) {
            //Overloaded operators in CLoxLiteral might throw exceptions, but CLoxLiteral has no knowledge of the current line,
            //so we catch the exception here, create a new one with the same message and with the current line, and throw it again.
            throw LoxRuntimeError(error.what(), chunk->readLine(currentOffset));
        }


#ifdef DEBUG_VM
        printDebugInfo(currentOffset);
#endif
    }
    return ExecutionResult::OK;
}

CLoxLiteral VM::readConstant() {
    int constantOffset = (int) chunk->readByte(programCounter);
    programCounter++;
    return chunk->readConstant(constantOffset);
}

void VM::pushStack(CLoxLiteral val) {
    stack.push(val);
}

CLoxLiteral VM::popStack() {
    CLoxLiteral val = stack.top();
    stack.pop();
    return val;
}

void VM::printDebugInfo(int offset) {
    std::cout << "[DEBUG]";
    std::cout << "\tInstruction: ";
    DebugUtils::printInstruction(offset, chunk.get());
    std::cout << "\tStack: [";
    std::stack<CLoxLiteral> copy(stack);

    while (!copy.empty()){
        std::cout << copy.top();
        copy.pop();

        if (!copy.empty()){
            std::cout << ", ";
        }
    }

    std::cout << "]\n";
}




