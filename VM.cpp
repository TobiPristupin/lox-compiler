#include <iostream>
#include <cassert>
#include "VM.h"
#include "DebugUtils.h"
#include "LoxError.h"
#include "Memory.h"

CallFrame::CallFrame(FunctionObj *function, int programCounter, int stackIndex) : function(function), programCounter(programCounter), stackIndex(stackIndex) {};

//when this macro is enabled, the VM will print every instruction before executing it
//#define DEBUG_VM

ExecutionResult VM::execute(FunctionObj *function) {
    CLoxLiteral functionLiteral(function);
    pushStack(functionLiteral);
    callFrames.emplace_back(CallFrame(function, 0, 0));
    currentFrame = callFrames.back();

    while (true){
        //keep track of the current offset before we modify it in case the DEBUG flag is on and we want to debug print info about
        //the last executed instruction.
        int currentOffset = currentFrame.programCounter;
        std::byte instruction = currentChunk()->readByte(currentOffset);
        currentFrame.programCounter++;

        switch (static_cast<OpCode>(instruction)) {
            case OpCode::OP_RETURN:
                Memory::freeAllHeapObjects();
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
                defineGlobal();
                break;
            case OpCode::OP_GET_GLOBAL:
                getGlobal();
                break;
            case OpCode::OP_SET_GLOBAL:
                setGlobal();
                break;
            case OpCode::OP_GET_LOCAL:
                getLocal();
                break;
            case OpCode::OP_SET_LOCAL:
                setLocal();
                break;
            case OpCode::OP_JUMP_IF_FALSE: {
                uint16_t offset = readTwoByteOffset();
                if (!isTruthy(stack.back())){
                    currentFrame.programCounter += offset;
                }

                break;
            }
            case OpCode::OP_JUMP: {
                uint16_t offset = readTwoByteOffset();
                currentFrame.programCounter += offset;
                break;
            }
            case OpCode::OP_LOOP: {
                uint16_t offset = readTwoByteOffset();
                currentFrame.programCounter -= offset + 1;
                break;
            }

            case OpCode::OP_CLASS: {
                pushStack(CLoxLiteral(Memory::allocateHeapClass(readConstantAsStringObj())));
                break;
            }
            case OpCode::OP_CALL: {

                CLoxLiteral obj = popStack();
                assert(obj.isObj() && obj.getObj()->isClass());
                auto *classObj = dynamic_cast<ClassObj*>(obj.getObj());
                pushStack(CLoxLiteral(Memory::allocateHeapInstance(classObj)));
                break;
            }
            case OpCode::OP_SET_PROPERTY: {
                CLoxLiteral value = popStack();
                StringObj *strObj = readConstantAsStringObj();
                popStack();

                CLoxLiteral literal = popStack();

                if (!literal.isObj() || !literal.getObj()->isInstance()){
                    throw LoxRuntimeError("Cannot access property. Only instances have fields.");
                }

                auto *instanceObj = dynamic_cast<InstanceObj*>(literal.getObj());
                instanceObj->fields[strObj->str] = value;
                pushStack(value);
                break;
            }
            case OpCode::OP_GET_PROPERTY: {
                StringObj *strObj = readConstantAsStringObj();
                popStack();
                CLoxLiteral literal = popStack();

                if (!literal.isObj() || !literal.getObj()->isInstance()){
                    throw LoxRuntimeError("Cannot access property. Only instances have fields.");
                }

                auto *instanceObj = dynamic_cast<InstanceObj*>(literal.getObj());

                if (instanceObj->fields.find(strObj->str) != instanceObj->fields.end()){
                    pushStack(instanceObj->fields.at(strObj->str));
                } else {
                    throw LoxRuntimeError("Undefined property " + strObj->str, readChunkLine(currentFrame.programCounter));
                }

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
        Obj* cObj = Memory::allocateHeapString(aObj->str + bObj->str);
        pushStack(CLoxLiteral(cObj));
    } else {
        throw LoxRuntimeError("Cannot apply operand '+' to objects of type " + literalTypeToString(a.type) + " and " + literalTypeToString(b.type), readChunkLine(currentFrame.programCounter));
    }
}

void VM::subtract() {
    CLoxLiteral b = popStack();
    CLoxLiteral a = popStack();
    if (a.isNumber() && b.isNumber()){
        pushStack(CLoxLiteral(a.getNumber() - b.getNumber()));
    } else {
        throw LoxRuntimeError("Cannot apply operand '-' to objects of type " + literalTypeToString(a.type) + " and " + literalTypeToString(b.type), readChunkLine(currentFrame.programCounter));
    }
}

void VM::multiply() {
    CLoxLiteral b = popStack();
    CLoxLiteral a = popStack();
    if (a.isNumber() && b.isNumber()){
        pushStack(CLoxLiteral(a.getNumber() * b.getNumber()));
    } else {
        throw LoxRuntimeError("Cannot apply operand '*' to objects of type " + literalTypeToString(a.type) + " and " + literalTypeToString(b.type), readChunkLine(currentFrame.programCounter));
    }
}

void VM::divide() {
    CLoxLiteral b = popStack();
    CLoxLiteral a = popStack();
    if (a.isNumber() && b.isNumber()){
        if (b.getNumber() == 0.0){
            throw LoxRuntimeError("Cannot divide by 0", readChunkLine(currentFrame.programCounter));
        }
        pushStack(CLoxLiteral(a.getNumber() / b.getNumber()));
    } else {
        throw LoxRuntimeError("Cannot apply operand '*' to objects of type " + literalTypeToString(a.type) + " and " + literalTypeToString(b.type), readChunkLine(currentFrame.programCounter));
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
    } else {
        throw std::runtime_error("This should be unreachable. Missing case");
    }
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
        throw LoxRuntimeError("Cannot apply operator '>' to operands of type " + literalTypeToString(a.type) + " and " + literalTypeToString(b.type), readChunkLine(currentFrame.programCounter));
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
        throw LoxRuntimeError("Cannot apply operator '>' to operands of type " + literalTypeToString(a.type) + " and " + literalTypeToString(b.type), readChunkLine(currentFrame.programCounter));
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

void VM::defineGlobal() {
    std::string name = readConstantAsStringObj()->str;
    if (globals.find(name) != globals.end()){
        throw LoxRuntimeError("Cannot redefine global variable '" + name + "' ", currentFrame.function->chunk->readLine(currentFrame.programCounter));
    }
    globals[name] = popStack();
    popStack(); //pop variable identifier from stack
}

void VM::getGlobal() {
    std::string name = readConstantAsStringObj()->str;
    if (globals.find(name) == globals.end()){
        throw LoxRuntimeError("Undefined variable '" + name + "'", readChunkLine(currentFrame.programCounter));
    }
    popStack(); //pop variable identifier from stack
    pushStack(globals.at(name));
}

void VM::setGlobal() {
    std::string name = readConstantAsStringObj()->str;
    if (globals.find(name) == globals.end()){
        throw LoxRuntimeError("Undefined variable '" + name + "'", readChunkLine(currentFrame.programCounter));
    }
    globals[name] = popStack();
}

void VM::getLocal() {
    int localIndex = readOneByteOffset();
    pushStack(stack.at(localIndex));
}

void VM::setLocal() {
    uint8_t localIndex = readOneByteOffset();
    stack.at(localIndex) = stack.back();
}

CLoxLiteral VM::readConstant() {
    uint8_t constantOffset = readOneByteOffset();
    return currentChunk()->readConstant(constantOffset);
}

StringObj *VM::readConstantAsStringObj() {
    CLoxLiteral constant = readConstant();
    assert(constant.isObj() && constant.getObj()->isString());
    return dynamic_cast<StringObj*>(constant.getObj());
}

ClassObj *VM::readConstantAsClassObj() {
    CLoxLiteral constant = readConstant();
    assert(constant.isObj() && constant.getObj()->isClass());
    return dynamic_cast<ClassObj*>(constant.getObj());
}

uint16_t VM::readTwoByteOffset() {
    uint16_t offset = ((uint16_t) currentChunk()->readByte(currentFrame.programCounter) >> 8u) | (uint16_t) currentChunk()->readByte(currentFrame.programCounter + 1);
    currentFrame.programCounter += 2;
    return offset;
}

uint8_t VM::readOneByteOffset() {
    uint8_t offset = (uint8_t) currentChunk()->readByte(currentFrame.programCounter);
    currentFrame.programCounter++;
    return offset;
}

void VM::pushStack(const CLoxLiteral& val) {
    stack.push_back(val);
}

CLoxLiteral VM::popStack() {
    CLoxLiteral val = stack.back();
    stack.pop_back();
    return val;
}

Chunk *VM::currentChunk() {
    return currentFrame.function->chunk.get();
}

int VM::readChunkLine(int offset) {
    return currentChunk()->readLine(offset);
}

void VM::printDebugInfo(int offset) {
    std::cout << "[DEBUG]";
    std::cout << "\tInstruction: ";
    DebugUtils::printInstruction(offset, currentChunk());
    std::cout << "\tStack: [";
    for (auto reverse_it = stack.rbegin(); reverse_it != stack.rend(); reverse_it++){
        std::cout << *reverse_it << ", ";
    }
    std::cout << "]\n";
}




