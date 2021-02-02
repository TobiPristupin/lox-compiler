#include <iostream>
#include "Compiler.h"
#include "LoxError.h"

ParseRule::ParseRule(ParseFunction parseAsPrefix, ParseFunction parseAsInfix, PrecedenceLevel precedenceLevel)
    : parseAsPrefix(parseAsPrefix), parseAsInfix(parseAsInfix), precedenceLevel(precedenceLevel) {}

Compiler::Compiler() {
    registerParselets();
}

void Compiler::registerParselets() {
    parsingRules.emplace(TokenType::END_OF_FILE, ParseRule(std::nullopt, std::nullopt, PrecedenceLevel::NONE));
    parsingRules.emplace(TokenType::LEFT_PAREN, ParseRule([this] {grouping();}, std::nullopt, PrecedenceLevel::NONE));
    parsingRules.emplace(TokenType::RIGHT_PAREN, ParseRule(std::nullopt, std::nullopt, PrecedenceLevel::NONE));
    parsingRules.emplace(TokenType::MINUS, ParseRule([this] {unary();}, [this] {binary();}, PrecedenceLevel::TERM));
    parsingRules.emplace(TokenType::PLUS, ParseRule(std::nullopt, [this] {binary();}, PrecedenceLevel::TERM));
    parsingRules.emplace(TokenType::SLASH, ParseRule(std::nullopt, [this] {binary();}, PrecedenceLevel::FACTOR));
    parsingRules.emplace(TokenType::STAR, ParseRule(std::nullopt, [this] {binary();}, PrecedenceLevel::FACTOR));
    parsingRules.emplace(TokenType::NUMBER, ParseRule([this] {number();}, std::nullopt, PrecedenceLevel::FACTOR));
}




std::shared_ptr<Chunk> Compiler::compile(const std::vector<Token> &tokens, bool &successFlag) {
    successFlag = true;
    this->tokens = tokens;
    chunk = std::make_shared<Chunk>();
    while (!isAtEnd()){
        try {
            expression();
        } catch (const LoxCompileError &error) {
            std::cout << error.what() << "\n";
            hadError = true;
            //TODO: synchronize!
            break;
        }
    }

    if (hadError){
        successFlag = false;
    }

    return chunk;
}

void Compiler::expression() {
    parsePrecedence(PrecedenceLevel::ASSIGNMENT);
}

void Compiler::parsePrecedence(PrecedenceLevel precedence) {
    advance();

    if (previous().type == TokenType::END_OF_FILE){
        currentChunk()->writeInstruction(OpCode::OP_RETURN, previous().line);
        return;
    }


    auto prefixRule = parsingRules.find(previous().type);
    if (prefixRule == parsingRules.end()){
        throw LoxCompileError("Expected expression", previous().line);
    }

    ParseFunction parseAsPrefix = prefixRule->second.parseAsPrefix;
    if (!parseAsPrefix.has_value()){
        throw LoxCompileError("Expected expression", previous().line);
    }

    //call the function
    parseAsPrefix.value()();

    while (peek().type != TokenType::END_OF_FILE && precedence <= parsingRules.at(peek().type).precedenceLevel){
        advance();
        ParseFunction parseAsInfix = parsingRules.at(previous().type).parseAsInfix;
        parseAsInfix.value()();
    }
}

void Compiler::number() {
    double value = stod(previous().lexeme);
    size_t constantOffset = currentChunk()->writeConstant(value);

    ////A chunk can only hold 256 constants because 8 bits are used to represent the index of the constant in the constant pool
    //TODO: Add a special OP_CONSTANT_16 special instruction that uses 16 bits?
    if (constantOffset >= 256) {
        throw LoxCompileError("Cannot have more than 256 constants", previous().line);
    }

    currentChunk()->writeInstruction(OpCode::OP_CONSTANT, previous().line);
    currentChunk()->write(static_cast<std::byte>(constantOffset), previous().line);
}

void Compiler::unary() {
    TokenType type = previous().type;

    //compile operand, only parse tokens with precedence of unary or higher
    parsePrecedence(PrecedenceLevel::UNARY);

    switch (type) {
        case TokenType::MINUS:
            emitByte(static_cast<std::byte>(OpCode::OP_NEGATE));
        default:
            return; //unreachable
    }

}

void Compiler::binary() {
    TokenType type = previous().type;

    ParseRule rule = parsingRules.at(type);
    int greaterPrecedence = static_cast<int>(rule.precedenceLevel) + 1;
    parsePrecedence(static_cast<PrecedenceLevel>(greaterPrecedence));

    switch (type) {
        case TokenType::PLUS:
            currentChunk()->writeInstruction(OpCode::OP_ADD, previous().line);
            break;
        case TokenType::MINUS:
            currentChunk()->writeInstruction(OpCode::OP_SUBTRACT, previous().line);
            break;
        case TokenType::STAR:
            currentChunk()->writeInstruction(OpCode::OP_MULTIPLY, previous().line);
            break;
        case TokenType::SLASH:
            currentChunk()->writeInstruction(OpCode::OP_DIVIDE, previous().line);
            break;
        default:
            throw std::runtime_error("Unreachable");
    }
}

void Compiler::grouping() {
    expression();
    expect(TokenType::RIGHT_PAREN, "Expected ')' after expression");
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

bool Compiler::match(const TokenType &type) {
    if (peek().type == type){
        advance();
        return true;
    }

    return false;
}

void Compiler::synchronize() {
    //TODO: Implement synchronization
    return;
}




