#include <iostream>
#include "Compiler.h"
#include "LoxError.h"

std::shared_ptr<Chunk> Compiler::compile(const std::vector<Token> &tokens, bool &successFlag) {
    successFlag = true;
    this->tokens = tokens;
    chunk = std::make_shared<Chunk>();
    while (!isAtEnd()){
        Token token = advance();
        if (token.type == TokenType::NUMBER){
            number();
        }
    }

    if (hadError){
        successFlag = false;
    }

    currentChunk()->writeInstruction(OpCode::OP_RETURN, -1); //fiz this
//    emitByte(static_cast<std::byte>(OpCode::OP_RETURN));

    return chunk;
}

void Compiler::number() {
    double value = stoi(previous().lexeme);
    size_t constantOffset = currentChunk()->writeConstant(value);
    currentChunk()->writeInstruction(OpCode::OP_CONSTANT, previous().line);
    currentChunk()->write(static_cast<std::byte>(constantOffset), previous().line);
}

void Compiler::emitByte(std::byte byte) {
    currentChunk()->write(byte, peek().line);
}

void Compiler::emitByte(std::byte first, std::byte second) {
    emitByte(first);
    emitByte(second);
}

std::shared_ptr<Chunk> Compiler::currentChunk() {
    return chunk;
}


Token Compiler::peek() {
    return tokens.at(current);
}

Token Compiler::advance() {
    if (!isAtEnd()) current++;
    return previous();
}

Token Compiler::previous() {
    return tokens.at(current - 1);
}

bool Compiler::isAtEnd() {
    return current >= tokens.size();
}

Token Compiler::expect(const TokenType &type, const std::string &message) {
    if (peek().type == type) return advance();
    else throw LoxCompileError(message, peek().line);
}

void Compiler::synchronize() {
    //TODO: Implement synchronization
    return;
}


