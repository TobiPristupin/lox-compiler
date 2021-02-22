#include <iostream>
#include <cassert>
#include "VM.h"
#include "DebugUtils.h"
#include "LoxError.h"
#include "Memory.h"

//when this macro is enabled, the VM will print every instruction before executing it
//#define DEBUG_VM


ExecutionResult VM::execute(std::shared_ptr<Chunk> chunk) {
    programCounter = 0;
    this->chunk = chunk;

    while (true){
        //keep track of the current offset before we modify it in case the DEBUG flag is on and we want to debug print info about
        //the last executed instruction.
        int currentOffset = programCounter;
        std::byte instruction = chunk->readByte(programCounter);
        programCounter++;

        switch (static_cast<OpCode>(instruction)) {
            case OpCode::OP_RETURN:
                freeHeapObjects();
                return ExecutionResult::OK;
            case OpCode::OP_PRINT:
                std::cout << popStack() << "\n";
                break;
            case OpCode::OP_CONSTANT:
                pushStack(readConstant());
                break;
            case OpCode::OP_NEGATE:
                negate();
                break;
            case OpCode::OP_ADD:
                add();
                break;
            case OpCode::OP_SUBTRACT:
                subtract();
                break;
            case OpCode::OP_MULTIPLY:
                multiply();
                break;
            case OpCode::OP_DIVIDE:
                divide();
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
                pushStack(CLoxLiteral(!isTruthy(popStack())));
                break;
            case OpCode::OP_EQUAL:
                equal();
                break;
            case OpCode::OP_GREATER:
                greater();
                break;
            case OpCode::OP_LESS:
                less();
                break;
            case OpCode::OP_POP:
                popStack();
                break;
            case OpCode::OP_DEFINE_GLOBAL:
            {
                CLoxLiteral constant = readConstant();
                assert(constant.isObj() && constant.getObj()->isString());
                auto name = dynamic_cast<StringObj*>(constant.getObj());
                if (globals.find(name->str) != globals.end()){
                    throw LoxRuntimeError("Cannot redefine global variable '" + name->str + "' ", chunk->readLine(programCounter));
                }
                globals[name->str] = popStack();
                popStack(); //pop variable identifier from stack
                break;
            }
            case OpCode::OP_GET_GLOBAL:
            {
                CLoxLiteral constant = readConstant();
                assert(constant.isObj() && constant.getObj()->isString());
                auto name = dynamic_cast<StringObj*>(constant.getObj());
                if (globals.find(name->str) == globals.end()){
                    throw LoxRuntimeError("Undefined variable '" + name->str + "'", chunk->readLine(programCounter));
                }
                popStack(); //pop variable identifier from stack
                pushStack(globals.at(name->str));
                break;
            }
            case OpCode::OP_SET_GLOBAL:
            {
                CLoxLiteral constant = readConstant();
                assert(constant.isObj() && constant.getObj()->isString());
                auto name = dynamic_cast<StringObj*>(constant.getObj());
                if (globals.find(name->str) == globals.end()){
                    throw LoxRuntimeError("Undefined variable '" + name->str + "'", chunk->readLine(programCounter));
                }
                globals[name->str] = popStack();
                break;
            }
            case OpCode::OP_GET_LOCAL:
            {
                int localIndex = (int) chunk->readByte(programCounter);
                programCounter++;
                pushStack(stack.at(localIndex));
                break;
            }
            case OpCode::OP_SET_LOCAL: {
                int localIndex = (int) chunk->readByte(programCounter);
                programCounter++;
                stack.at(localIndex) = stack.back();
                break;
            }
            case OpCode::OP_JUMP_IF_FALSE: {
                uint16_t offset = ((uint16_t) chunk->readByte(programCounter) >> 8u) | (uint16_t) chunk->readByte(programCounter + 1);
                programCounter += 2;
                if (!isTruthy(stack.back())){
                    programCounter += offset;
                }

                break;
            }
            case OpCode::OP_JUMP: {
                uint16_t offset = ((uint16_t) chunk->readByte(programCounter) >> 8u) | (uint16_t) chunk->readByte(programCounter + 1);
                programCounter += 2 + offset;
                break;
            }
            case OpCode::OP_LOOP: {
                uint16_t offset = ((uint16_t) chunk->readByte(programCounter) >> 8u) | (uint16_t) chunk->readByte(programCounter + 1);
                programCounter += 2;
                programCounter -= offset + 1;
                break;
            }

        }


#ifdef DEBUG_VM
        printDebugInfo(currentOffset);
#endif
    }
    return ExecutionResult::OK;
}

void VM::add() {
    CLoxLiteral b = popStack();
    CLoxLiteral a = popStack();
    if (a.isNumber() && b.isNumber()){
        pushStack(CLoxLiteral(a.getNumber() + b.getNumber()));
    } else if (a.isObj() && b.isObj() && a.getObj()->isString() && b.getObj()->isString()){
        auto *aObj = dynamic_cast<StringObj*>(a.getObj());
        auto *bObj = dynamic_cast<StringObj*>(b.getObj());
        Obj* cObj = allocateObject(aObj->str + bObj->str);
        pushStack(CLoxLiteral(cObj));
    } else {
        throw LoxRuntimeError("Cannot apply operand '+' to objects of type " + literalTypeToString(a.type) + " and " + literalTypeToString(b.type), chunk->readLine(programCounter));
    }
}

void VM::subtract() {
    CLoxLiteral b = popStack();
    CLoxLiteral a = popStack();
    if (a.isNumber() && b.isNumber()){
        pushStack(CLoxLiteral(a.getNumber() - b.getNumber()));
    } else {
        throw LoxRuntimeError("Cannot apply operand '-' to objects of type " + literalTypeToString(a.type) + " and " + literalTypeToString(b.type), chunk->readLine(programCounter));
    }
}

void VM::multiply() {
    CLoxLiteral b = popStack();
    CLoxLiteral a = popStack();
    if (a.isNumber() && b.isNumber()){
        pushStack(CLoxLiteral(a.getNumber() * b.getNumber()));
    } else {
        throw LoxRuntimeError("Cannot apply operand '*' to objects of type " + literalTypeToString(a.type) + " and " + literalTypeToString(b.type), chunk->readLine(programCounter));
    }
}

void VM::divide() {
    CLoxLiteral b = popStack();
    CLoxLiteral a = popStack();
    if (a.isNumber() && b.isNumber()){
        if (b.getNumber() == 0.0){
            throw LoxRuntimeError("Cannot divide by 0", chunk->readLine(programCounter));
        }
        pushStack(CLoxLiteral(a.getNumber() / b.getNumber()));
    } else {
        throw LoxRuntimeError("Cannot apply operand '*' to objects of type " + literalTypeToString(a.type) + " and " + literalTypeToString(b.type), chunk->readLine(programCounter));
    }
}

void VM::equal() {
    CLoxLiteral b = popStack();
    CLoxLiteral a = popStack();

    if (a.type != b.type) pushStack(CLoxLiteral(false));

    if (a.isNumber() && b.isNumber()){
         pushStack(CLoxLiteral(a.getNumber() == b.getNumber()));
    } else if (a.isObj() && b.isObj()){
        if (a.getObj()->isString() && b.getObj()->isString()){
            pushStack(CLoxLiteral(dynamic_cast<StringObj*>(a.getObj())->str == dynamic_cast<StringObj*>(b.getObj())->str));
        }
    } else if (a.isBoolean() && b.isBoolean()){
        pushStack(CLoxLiteral(a.getBoolean() == b.getBoolean()));
    } else if (a.isNil() && b.isNil()){
         pushStack(CLoxLiteral(true));
    }

    throw std::runtime_error("This should be unreachable. Missing case");
}

void VM::greater() {
    CLoxLiteral b = popStack();
    CLoxLiteral a = popStack();

    if (a.isNumber() && b.isNumber()){
         pushStack(CLoxLiteral(a.getNumber() > b.getNumber()));
    } else if (a.isObj() && b.isObj()){
        if (a.getObj()->isString() && b.getObj()->isString()){
             pushStack(CLoxLiteral(dynamic_cast<StringObj*>(a.getObj()) > dynamic_cast<StringObj*>(b.getObj())));
        }
    } else {
        throw LoxRuntimeError("Cannot apply operator '>' to operands of type " + literalTypeToString(a.type) + " and " + literalTypeToString(b.type), chunk->readLine(programCounter));
    }
}

void VM::less() {
    CLoxLiteral b = popStack();
    CLoxLiteral a = popStack();

    if (a.isNumber() && b.isNumber()){
        pushStack(CLoxLiteral(a.getNumber() < b.getNumber()));
    } else if (a.isObj() && b.isObj()){
        if (a.getObj()->isString() && b.getObj()->isString()){
            pushStack(CLoxLiteral(dynamic_cast<StringObj*>(a.getObj()) < dynamic_cast<StringObj*>(b.getObj())));
        }
    } else {
        throw LoxRuntimeError("Cannot apply operator '>' to operands of type " + literalTypeToString(a.type) + " and " + literalTypeToString(b.type), chunk->readLine(programCounter));
    }
}

void VM::negate() {
    CLoxLiteral a = popStack();
    if (a.isNumber()){
        pushStack(CLoxLiteral(-a.getNumber()));
        return;
    }

    throw std::runtime_error("Cannot apply unary operator '-' to operand of type " + literalTypeToString(a.type));
}

bool VM::isTruthy(const CLoxLiteral &a) {
    if (a.isBoolean()){
        return a.getBoolean();
    } else if (a.isNumber()){
        return a.getNumber() != 0;
    } else if (a.isNil()){
        return false;
    }

    return true;
}

CLoxLiteral VM::readConstant() {
    int constantOffset = (int) chunk->readByte(programCounter);
    programCounter++;
    return chunk->readConstant(constantOffset);
}

void VM::pushStack(const CLoxLiteral& val) {
    stack.push_back(val);
}

CLoxLiteral VM::popStack() {
    CLoxLiteral val = stack.back();
    stack.pop_back();
    return val;
}

Obj* VM::allocateObject(const std::string &str) {
    auto *obj = new StringObj(str);
    Memory::heapObjects.push_back(obj);
    return obj;
}

void VM::freeHeapObjects() {
    for (Obj* obj : Memory::heapObjects){
        delete obj;
    }
}

void VM::printDebugInfo(int offset) {
    std::cout << "[DEBUG]";
    std::cout << "\tInstruction: ";
    DebugUtils::printInstruction(offset, chunk.get());
    std::cout << "\tStack: [";
    for (auto reverse_it = stack.rbegin(); reverse_it != stack.rend(); reverse_it++){
        std::cout << *reverse_it << ", ";
    }
    std::cout << "]\n";
}




