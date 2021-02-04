#include <iostream>
#include <bitset>
#include <cstddef>
#include <cassert>
#include "Compiler.h"
#include "LoxError.h"
#include "DebugUtils.h"

//if this directive is enabled the compiler prints out every opcode after emitting them to the current chunk
#define DEBUG_COMPILER

ParseRule::ParseRule(ParseFunction parseAsPrefix, ParseFunction parseAsInfix, PrecedenceLevel precedenceLevel)
    : parseAsPrefix(parseAsPrefix), parseAsInfix(parseAsInfix), precedenceLevel(precedenceLevel) {}

Compiler::Compiler() {
    registerParsingRules();
}

void Compiler::registerParsingRules() {
    parsingRules = {
            {TokenType::LEFT_PAREN, ParseRule([this] {grouping();}, std::nullopt, PrecedenceLevel::NONE)},
            {TokenType::RIGHT_PAREN, ParseRule(std::nullopt, std::nullopt, PrecedenceLevel::NONE)},
            {TokenType::LEFT_BRACE, ParseRule(std::nullopt, std::nullopt, PrecedenceLevel::NONE)},
            {TokenType::RIGHT_BRACE, ParseRule(std::nullopt, std::nullopt, PrecedenceLevel::NONE)},
            {TokenType::LEFT_BRACKET, ParseRule(std::nullopt, std::nullopt, PrecedenceLevel::NONE)},
            {TokenType::RIGHT_BRACKET, ParseRule(std::nullopt, std::nullopt, PrecedenceLevel::NONE)},
            {TokenType::COMMA, ParseRule(std::nullopt, std::nullopt, PrecedenceLevel::NONE)},
            {TokenType::DOT, ParseRule(std::nullopt, std::nullopt, PrecedenceLevel::NONE)},
            {TokenType::MINUS, ParseRule([this] {unary();}, [this] {binary();}, PrecedenceLevel::TERM)},
            {TokenType::PLUS, ParseRule(std::nullopt, [this] {binary();}, PrecedenceLevel::TERM)},
            {TokenType::SEMICOLON, ParseRule(std::nullopt, std::nullopt, PrecedenceLevel::NONE)},
            {TokenType::SLASH, ParseRule(std::nullopt, [this] {binary();}, PrecedenceLevel::FACTOR)},
            {TokenType::STAR, ParseRule(std::nullopt, [this] {binary();}, PrecedenceLevel::FACTOR)},
            {TokenType::COLON, ParseRule(std::nullopt, std::nullopt, PrecedenceLevel::NONE)},
            {TokenType::BANG, ParseRule([this] {unary();}, std::nullopt, PrecedenceLevel::NONE)},
            {TokenType::BANG_EQUAL, ParseRule(std::nullopt, [this] {binary();}, PrecedenceLevel::EQUALITY)},
            {TokenType::EQUAL, ParseRule(std::nullopt, std::nullopt, PrecedenceLevel::NONE)},
            {TokenType::EQUAL_EQUAL, ParseRule(std::nullopt, [this] {binary();}, PrecedenceLevel::EQUALITY)},
            {TokenType::GREATER, ParseRule(std::nullopt, [this] {binary();}, PrecedenceLevel::COMPARISON)},
            {TokenType::GREATER_EQUAL, ParseRule(std::nullopt, [this] {binary();}, PrecedenceLevel::COMPARISON)},
            {TokenType::LESS, ParseRule(std::nullopt, [this] {binary();}, PrecedenceLevel::COMPARISON)},
            {TokenType::LESS_EQUAL, ParseRule(std::nullopt, [this] {binary();}, PrecedenceLevel::COMPARISON)},
            {TokenType::PLUS_PLUS, ParseRule(std::nullopt, std::nullopt, PrecedenceLevel::NONE)},
            {TokenType::MINUS_MINUS, ParseRule(std::nullopt, std::nullopt, PrecedenceLevel::NONE)},
            {TokenType::IDENTIFIER, ParseRule(std::nullopt, std::nullopt, PrecedenceLevel::NONE)},
            {TokenType::STRING, ParseRule(std::nullopt, std::nullopt, PrecedenceLevel::NONE)},
            {TokenType::NUMBER, ParseRule([this] {number();}, std::nullopt, PrecedenceLevel::FACTOR)},
            {TokenType::AND, ParseRule(std::nullopt, std::nullopt, PrecedenceLevel::NONE)},
            {TokenType::CLASS, ParseRule(std::nullopt, std::nullopt, PrecedenceLevel::NONE)},
            {TokenType::ELSE, ParseRule(std::nullopt, std::nullopt, PrecedenceLevel::NONE)},
            {TokenType::ELIF, ParseRule(std::nullopt, std::nullopt, PrecedenceLevel::NONE)},
            {TokenType::FALSE, ParseRule([this] {literal();}, std::nullopt, PrecedenceLevel::NONE)},
            {TokenType::FUN, ParseRule(std::nullopt, std::nullopt, PrecedenceLevel::NONE)},
            {TokenType::FOR, ParseRule(std::nullopt, std::nullopt, PrecedenceLevel::NONE)},
            {TokenType::IF, ParseRule(std::nullopt, std::nullopt, PrecedenceLevel::NONE)},
            {TokenType::NIL, ParseRule([this] {literal();}, std::nullopt, PrecedenceLevel::NONE)},
            {TokenType::OR, ParseRule(std::nullopt, std::nullopt, PrecedenceLevel::NONE)},
            {TokenType::PRINT, ParseRule(std::nullopt, std::nullopt, PrecedenceLevel::NONE)},
            {TokenType::RETURN, ParseRule(std::nullopt, std::nullopt, PrecedenceLevel::NONE)},
            {TokenType::SUPER, ParseRule(std::nullopt, std::nullopt, PrecedenceLevel::NONE)},
            {TokenType::THIS, ParseRule(std::nullopt, std::nullopt, PrecedenceLevel::NONE)},
            {TokenType::TRUE, ParseRule([this] {literal();}, std::nullopt, PrecedenceLevel::NONE)},
            {TokenType::VAR, ParseRule(std::nullopt, std::nullopt, PrecedenceLevel::NONE)},
            {TokenType::WHILE, ParseRule(std::nullopt, std::nullopt, PrecedenceLevel::NONE)},
            {TokenType::BREAK, ParseRule(std::nullopt, std::nullopt, PrecedenceLevel::NONE)},
            {TokenType::CONTINUE, ParseRule(std::nullopt, std::nullopt, PrecedenceLevel::NONE)},
            {TokenType::LAMBDA, ParseRule(std::nullopt, std::nullopt, PrecedenceLevel::NONE)},
            {TokenType::END_OF_FILE, ParseRule(std::nullopt, std::nullopt, PrecedenceLevel::NONE)},
    };
}


std::shared_ptr<Chunk> Compiler::compile(const std::vector<Token> &tokens, bool &successFlag) {
    successFlag = true;
    this->tokens = tokens;
    chunk = std::make_shared<Chunk>();
    while (peek().type != TokenType::END_OF_FILE){
        try {
            expression();
        } catch (const LoxCompileError &error) {
            std::cout << error.what() << "\n";
            hadError = true;
            //TODO: synchronize!
            break;
        }
    }

    if (!hadError){
        assert(match(TokenType::END_OF_FILE)); //Scanner should have included a END_OF_FILE token
    }

    emitByte(OpCode::OP_RETURN);
    successFlag = !hadError;
    return chunk;
}

void Compiler::expression() {
    parsePrecedence(PrecedenceLevel::ASSIGNMENT);
}

//Parses all tokens that have a precedence >= to the precedence passed
void Compiler::parsePrecedence(PrecedenceLevel precedence) {
    advance();

    //Get the function to parse the previous token as a prefix expression
    auto prefixRule = parsingRules.find(previous().type);
    ParseFunction parseAsPrefix = prefixRule->second.parseAsPrefix;

    /*Every expression by definition must start with a prefix token. If the current token does not have a function
    to parse it as a prefix token, then that means we started our expression with a non-prefix token, which is invalid*/
    if (!parseAsPrefix.has_value()){
        throw LoxCompileError("Expected expression", previous().line);
    }

    //call the function to parse the token as prefix
    parseAsPrefix.value()();

    //keep parsing tokens while the precedence level of the following token is greater than the precedence passes as a param
    while (precedence <= parsingRules.at(peek().type).precedenceLevel){
        advance();
        //get rule to parse as infix and parse
        ParseFunction parseAsInfix = parsingRules.at(previous().type).parseAsInfix;
        parseAsInfix.value()();
    }
}

void Compiler::number() {
    CLoxLiteral value(previous());
    emitConstant(value);
}

void Compiler::unary() {
    TokenType type = previous().type;

    //compile operand, only parse tokens with precedence of unary or higher
    parsePrecedence(PrecedenceLevel::UNARY);

    switch (type) {
        case TokenType::MINUS:
            emitByte(static_cast<std::byte>(OpCode::OP_NEGATE));
        case TokenType::BANG:
            emitByte(static_cast<std::byte>(OpCode::OP_NOT));
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
            emitByte(OpCode::OP_ADD); break;
        case TokenType::MINUS:
            emitByte(OpCode::OP_SUBTRACT); break;
        case TokenType::STAR:
            emitByte(OpCode::OP_MULTIPLY); break;
        case TokenType::SLASH:
            emitByte(OpCode::OP_DIVIDE); break;
        case TokenType::EQUAL_EQUAL:
            emitByte(OpCode::OP_EQUAL); break;
        case TokenType::BANG_EQUAL:
            emitByte(OpCode::OP_EQUAL, OpCode::OP_NOT); break;
        case TokenType::GREATER:
            emitByte(OpCode::OP_GREATER); break;
        case TokenType::LESS:
            emitByte(OpCode::OP_LESS); break;
        case TokenType::GREATER_EQUAL:
            emitByte(OpCode::OP_LESS, OpCode::OP_NOT); break;
        case TokenType::LESS_EQUAL:
            emitByte(OpCode::OP_GREATER, OpCode::OP_NOT); break;
        default:
            throw std::runtime_error("Unreachable");
    }
}

void Compiler::grouping() {
    expression();
    expect(TokenType::RIGHT_PAREN, "Expected ')' after expression");
}

void Compiler::literal() {
    switch (previous().type) {
        case TokenType::TRUE:
            emitByte(OpCode::OP_TRUE); break;
        case TokenType::FALSE:
            emitByte(OpCode::OP_FALSE); break;
        case TokenType::NIL:
            emitByte(OpCode::OP_NIL); break;
        default:
            throw std::runtime_error("unreachable");
    }
}

void Compiler::emitByte(std::byte byte) {
    currentChunk()->write(byte, previous().line);
#ifdef DEBUG_COMPILER
    std::cout << "[DEBUG] Compiler Emitted: " << std::to_integer<int>(byte) << "\n";
#endif
}

void Compiler::emitByte(std::byte first, std::byte second) {
    emitByte(first);
    emitByte(second);
}

void Compiler::emitByte(OpCode opCode) {
    emitByte(static_cast<std::byte>(opCode));
}

void Compiler::emitByte(OpCode opCode1, std::byte byte) {
    emitByte(opCode1);
    emitByte(byte);
}

void Compiler::emitByte(OpCode opCode1, OpCode opcode2) {
    emitByte(opCode1);
    emitByte(opcode2);
}

void Compiler::emitConstant(const CLoxLiteral &constant) {
    size_t constantOffset = currentChunk()->writeConstant(constant);

    ////A chunk can only hold 256 constants because 8 bits are used to represent the index of the constant in the constant pool
    //TODO: Add a special OP_CONSTANT_16 special instruction that uses 16 bits?
    if (constantOffset >= 256) {
        throw LoxCompileError("Cannot have more than 256 constants", previous().line);
    }

    emitByte(OpCode::OP_CONSTANT);
    emitByte(static_cast<std::byte>(constantOffset));
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




