#include <iostream>
#include <cstddef>
#include <cassert>
#include "Compiler.h"
#include "LoxError.h"
#include "DebugUtils.h"
#include "Memory.h"

//if this directive is enabled the compiler prints out every opcode after emitting them to the current chunk
//#define DEBUG_COMPILER

ParseRule::ParseRule(ParseFunction parseAsPrefix, ParseFunction parseAsInfix, PrecedenceLevel precedenceLevel)
    : parseAsPrefix(parseAsPrefix), parseAsInfix(parseAsInfix), precedenceLevel(precedenceLevel) {}


LocalVariables::Variable::Variable(const Token &name, int depth) : name(name), depth(depth) {}

Compiler::Compiler() {
    functionType = FunctionType::SCRIPT;
    function = dynamic_cast<FunctionObj*>(Memory::allocateHeapFunction("", Chunk(), 0));

    localVariables.locals.emplace_back(Token(TokenType::IDENTIFIER, "", 0), 0);

    registerParsingRules();
}

void Compiler::registerParsingRules() {
    parsingRules = {
            {TokenType::LEFT_PAREN, ParseRule([this] (bool canAssign) {grouping(canAssign);}, [this] (bool canAssign) {call(canAssign);}, PrecedenceLevel::CALL)},
            {TokenType::RIGHT_PAREN, ParseRule(std::nullopt, std::nullopt, PrecedenceLevel::NONE)},
            {TokenType::LEFT_BRACE, ParseRule(std::nullopt, std::nullopt, PrecedenceLevel::NONE)},
            {TokenType::RIGHT_BRACE, ParseRule(std::nullopt, std::nullopt, PrecedenceLevel::NONE)},
            {TokenType::LEFT_BRACKET, ParseRule(std::nullopt, std::nullopt, PrecedenceLevel::NONE)},
            {TokenType::RIGHT_BRACKET, ParseRule(std::nullopt, std::nullopt, PrecedenceLevel::NONE)},
            {TokenType::COMMA, ParseRule(std::nullopt, std::nullopt, PrecedenceLevel::NONE)},
            {TokenType::DOT, ParseRule(std::nullopt, [this] (bool canAssign) {dot(canAssign);}, PrecedenceLevel::CALL)},
            {TokenType::MINUS, ParseRule([this] (bool canAssign) {unary(canAssign);}, [this] (bool canAssign) {binary(canAssign);}, PrecedenceLevel::TERM)},
            {TokenType::PLUS, ParseRule(std::nullopt, [this] (bool canAssign) {binary(canAssign);}, PrecedenceLevel::TERM)},
            {TokenType::SEMICOLON, ParseRule(std::nullopt, std::nullopt, PrecedenceLevel::NONE)},
            {TokenType::SLASH, ParseRule(std::nullopt, [this] (bool canAssign) {binary(canAssign);}, PrecedenceLevel::FACTOR)},
            {TokenType::STAR, ParseRule(std::nullopt, [this] (bool canAssign) {binary(canAssign);}, PrecedenceLevel::FACTOR)},
            {TokenType::COLON, ParseRule(std::nullopt, std::nullopt, PrecedenceLevel::NONE)},
            {TokenType::BANG, ParseRule([this] (bool canAssign) {unary(canAssign);}, std::nullopt, PrecedenceLevel::NONE)},
            {TokenType::BANG_EQUAL, ParseRule(std::nullopt, [this] (bool canAssign) {binary(canAssign);}, PrecedenceLevel::EQUALITY)},
            {TokenType::EQUAL, ParseRule(std::nullopt, std::nullopt, PrecedenceLevel::NONE)},
            {TokenType::EQUAL_EQUAL, ParseRule(std::nullopt, [this] (bool canAssign) {binary(canAssign);}, PrecedenceLevel::EQUALITY)},
            {TokenType::GREATER, ParseRule(std::nullopt, [this] (bool canAssign) {binary(canAssign);}, PrecedenceLevel::COMPARISON)},
            {TokenType::GREATER_EQUAL, ParseRule(std::nullopt, [this] (bool canAssign) {binary(canAssign);}, PrecedenceLevel::COMPARISON)},
            {TokenType::LESS, ParseRule(std::nullopt, [this] (bool canAssign) {binary(canAssign);}, PrecedenceLevel::COMPARISON)},
            {TokenType::LESS_EQUAL, ParseRule(std::nullopt, [this] (bool canAssign) {binary(canAssign);}, PrecedenceLevel::COMPARISON)},
            {TokenType::PLUS_PLUS, ParseRule(std::nullopt, std::nullopt, PrecedenceLevel::NONE)},
            {TokenType::MINUS_MINUS, ParseRule(std::nullopt, std::nullopt, PrecedenceLevel::NONE)},
            {TokenType::IDENTIFIER, ParseRule([this] (bool canAssign) {variable(canAssign);}, std::nullopt, PrecedenceLevel::NONE)},
            {TokenType::IDENTIFIER, ParseRule([this] (bool canAssign) {variable(canAssign);}, std::nullopt, PrecedenceLevel::NONE)},
            {TokenType::STRING, ParseRule([this] (bool canAssign) {string(canAssign);}, std::nullopt, PrecedenceLevel::NONE)},
            {TokenType::NUMBER, ParseRule([this] (bool canAssign) {number(canAssign);}, std::nullopt, PrecedenceLevel::FACTOR)},
            {TokenType::AND, ParseRule(std::nullopt, [this] (bool canAssign) {parseAnd(canAssign);}, PrecedenceLevel::AND)},
            {TokenType::CLASS, ParseRule(std::nullopt, std::nullopt, PrecedenceLevel::NONE)},
            {TokenType::ELSE, ParseRule(std::nullopt, std::nullopt, PrecedenceLevel::NONE)},
            {TokenType::ELIF, ParseRule(std::nullopt, std::nullopt, PrecedenceLevel::NONE)},
            {TokenType::FALSE, ParseRule([this] (bool canAssign) {literal(canAssign);}, std::nullopt, PrecedenceLevel::NONE)},
            {TokenType::FUN, ParseRule(std::nullopt, std::nullopt, PrecedenceLevel::NONE)},
            {TokenType::FOR, ParseRule(std::nullopt, std::nullopt, PrecedenceLevel::NONE)},
            {TokenType::IF, ParseRule(std::nullopt, std::nullopt, PrecedenceLevel::NONE)},
            {TokenType::NIL, ParseRule([this] (bool canAssign) {literal(canAssign);}, std::nullopt, PrecedenceLevel::NONE)},
            {TokenType::OR, ParseRule(std::nullopt, [this] (bool canAssign) {parseOr(canAssign);}, PrecedenceLevel::OR)},
            {TokenType::PRINT, ParseRule(std::nullopt, std::nullopt, PrecedenceLevel::NONE)},
            {TokenType::RETURN, ParseRule(std::nullopt, std::nullopt, PrecedenceLevel::NONE)},
            {TokenType::SUPER, ParseRule(std::nullopt, std::nullopt, PrecedenceLevel::NONE)},
            {TokenType::THIS, ParseRule(std::nullopt, std::nullopt, PrecedenceLevel::NONE)},
            {TokenType::TRUE, ParseRule([this] (bool canAssign) {literal(canAssign);}, std::nullopt, PrecedenceLevel::NONE)},
            {TokenType::VAR, ParseRule(std::nullopt, std::nullopt, PrecedenceLevel::NONE)},
            {TokenType::WHILE, ParseRule(std::nullopt, std::nullopt, PrecedenceLevel::NONE)},
            {TokenType::BREAK, ParseRule(std::nullopt, std::nullopt, PrecedenceLevel::NONE)},
            {TokenType::CONTINUE, ParseRule(std::nullopt, std::nullopt, PrecedenceLevel::NONE)},
            {TokenType::LAMBDA, ParseRule(std::nullopt, std::nullopt, PrecedenceLevel::NONE)},
            {TokenType::END_OF_FILE, ParseRule(std::nullopt, std::nullopt, PrecedenceLevel::NONE)},
    };
}


FunctionObj* Compiler::compile(const std::vector<Token> &tokens, bool &successFlag) {
    successFlag = true;
    this->tokens = tokens;
    while (peek().type != TokenType::END_OF_FILE){
        declaration();
    }

    if (!hadError){
        assert(match(TokenType::END_OF_FILE)); //Scanner should have included a END_OF_FILE token
    }

    emitByte(OpCode::OP_RETURN);
    successFlag = !hadError;
    return function;
}

void Compiler::declaration() {
    try {
        if (match(TokenType::VAR)){
            varDeclaration();
            return;
        } else {
            statement();
        }
    } catch (const LoxCompileError &error) {
        std::cout << error.what() << "\n";
        hadError = true;
        synchronize();
    }
}

void Compiler::varDeclaration() {
    std::byte offset = parseVariableName();

    if (match(TokenType::EQUAL)){
        expression();
    } else {
        emitByte(OpCode::OP_NIL); //If the user did not provide an initializer, use nil
    }

    expect(TokenType::SEMICOLON, "Expected ';' after variable declaration");

    defineVariable(offset);
}

void Compiler::statement() {
    if (match(TokenType::PRINT)){
        printStatement();
    } else if (match(TokenType::LEFT_BRACE)){
        beginScope();
        block();
        endScope();
    } else if (match(TokenType::IF)){
        ifStatement();
    } else if (match(TokenType::WHILE)) {
        whileStatement();
    } else if (match(TokenType::FOR)){
        forStatement();
    } else if (match(TokenType::FUN)){
        functionDeclaration();
    } else if (match(TokenType::RETURN)){
        emitByte(OpCode::OP_RETURN); //temporary
        expect(TokenType::SEMICOLON, "Expected ';' after return");
    } else if (match(TokenType::CLASS)){
        classDeclaration();
    } else {
        expressionStatement();
    }

}

void Compiler::block() {
    while (peek().type != TokenType::RIGHT_BRACE && peek().type != TokenType::END_OF_FILE){
        declaration();
    }

    expect(TokenType::RIGHT_BRACE, "Expected '}' after block");
}

void Compiler::classDeclaration() {
    Token name = expect(TokenType::IDENTIFIER, "Expected identifier after 'class'");
    std::byte nameConstant = emitIdentifierConstant(name);
    declareVariable();

    emitByte(OpCode::OP_CLASS, nameConstant);
    defineVariable(nameConstant);

    expect(TokenType::LEFT_BRACE, "Expected '{' before class body");
    expect(TokenType::RIGHT_BRACE, "Expected '}' after class body");
}

void Compiler::functionDeclaration() {
    std::byte global = parseVariableName();
    markVariableInitialized();
    parseFunction(FunctionType::FUNCTION);
    defineVariable(global);
}

void Compiler::parseFunction(FunctionType type) {
    //TODO
}

void Compiler::ifStatement() {
    expect(TokenType::LEFT_PAREN, "Expected '(' after if statement");
    expression();
    expect(TokenType::RIGHT_PAREN, "Expected ')' after if condition");

    int thenJump = emitJump(OpCode::OP_JUMP_IF_FALSE);
    emitByte(OpCode::OP_POP);
    statement();

    int elseJump = emitJump(OpCode::OP_JUMP);

    patchJump(thenJump);

    emitByte(OpCode::OP_POP);
    if (match(TokenType::ELSE)){
        statement();
    }

    patchJump(elseJump);
}

void Compiler::whileStatement() {
    int loopStart = currentChunk()->byteCount();

    expect(TokenType::LEFT_PAREN, "Expected '(' after while");
    expression();
    expect(TokenType::RIGHT_PAREN, "Expected ')' after while condition");

    int exitJump = emitJump(OpCode::OP_JUMP_IF_FALSE);
    emitByte(OpCode::OP_POP);
    statement();

    emitLoop(loopStart);

    patchJump(exitJump);
    emitByte(OpCode::OP_POP);
}

void Compiler::forStatement() {
    beginScope();

    expect(TokenType::LEFT_PAREN, "Expected '(' after for");
    if (match(TokenType::SEMICOLON)){
        //no initializer
    } else if (match(TokenType::VAR)){
        varDeclaration();
    } else {
        expressionStatement();
    }


    int loopStart = currentChunk()->byteCount();

    int exitJump = -1;
    if (!match(TokenType::SEMICOLON)){
        expression();
        expect(TokenType::SEMICOLON, "Expected ';' after loop condition");
        exitJump = emitJump(OpCode::OP_JUMP_IF_FALSE);
        emitByte(OpCode::OP_POP); //pop condition from stack
    }

    if (!match(TokenType::RIGHT_PAREN)){
        int bodyJump = emitJump(OpCode::OP_JUMP);
        int incrementStart = currentChunk()->byteCount();
        expression();
        emitByte(OpCode::OP_POP);
        expect(TokenType::RIGHT_PAREN, "Expected ')' after for");

        emitLoop(loopStart);
        loopStart = incrementStart;
        patchJump(bodyJump);
    }


    statement();

    emitLoop(loopStart);

    if (exitJump != -1){
        patchJump(exitJump);
        emitByte(OpCode::OP_POP);
    }

    endScope();
}

void Compiler::expressionStatement() {
    expression();
    expect(TokenType::SEMICOLON, "Expected ';' after expression");
    emitByte(OpCode::OP_POP); //discard any value that was added to the stack
}

void Compiler::printStatement() {
    expression();
    expect(TokenType::SEMICOLON, "Expected ';' after print statement");
    emitByte(OpCode::OP_PRINT);
}

void Compiler::expression() {
    parsePrecedence(PrecedenceLevel::ASSIGNMENT);
}

void Compiler::call(bool canAssign) {
    emitByte(OpCode::OP_CALL);
    expect(TokenType::RIGHT_PAREN, "Expected ')' after call");
}

void Compiler::dot(bool canAssign) {
    Token name = expect(TokenType::IDENTIFIER, "Expected identifier after '.'");
    std::byte offset = emitIdentifierConstant(name);

    if (canAssign && match(TokenType::EQUAL)){
        expression();
        emitByte(OpCode::OP_SET_PROPERTY, offset);
    } else {
        emitByte(OpCode::OP_GET_PROPERTY, offset);
    }
}

void Compiler::variable(bool canAssign) {
    namedVariable(canAssign, previous());
}

void Compiler::beginScope() {
    localVariables.currentScopeDepth++;
}

void Compiler::endScope() {
    localVariables.currentScopeDepth--;

    auto reverse_it = localVariables.locals.rbegin();
    while (reverse_it != localVariables.locals.rend() && reverse_it->depth > localVariables.currentScopeDepth) {
        //https://stackoverflow.com/questions/1830158/how-to-call-erase-with-a-reverse-iterator/50282077#50282077
        reverse_it = decltype(reverse_it) (localVariables.locals.erase(std::next(reverse_it).base()));
        emitByte(OpCode::OP_POP);
    }
}

void Compiler::namedVariable(bool canAssign, const Token &name) {
    OpCode getOpCode, setOpCode;
    std::optional<std::byte> offset = resolveLocalVariable(name);

    if (offset.has_value()){
        getOpCode = OpCode::OP_GET_LOCAL;
        setOpCode = OpCode::OP_SET_LOCAL;
    } else {
        getOpCode = OpCode::OP_GET_GLOBAL;
        setOpCode = OpCode::OP_SET_GLOBAL;
        offset = emitIdentifierConstant(name);
    }

    if (canAssign && match(TokenType::EQUAL)){
        expression();
        emitByte(setOpCode, offset.value());
    } else {
        emitByte(getOpCode, offset.value());
    }
}

std::optional<std::byte> Compiler::resolveLocalVariable(const Token &name) {
    for (auto reverse_it = localVariables.locals.rbegin(); reverse_it != localVariables.locals.rend(); ++reverse_it){
        if (reverse_it->name.lexeme == name.lexeme){
            if (reverse_it->depth == -1){
                throw LoxCompileError("Can't read local variable in its own initializer", previous().line);
            }

            //https://stackoverflow.com/a/24998000  safe to narrowly cast to int because maximum amount of locals is 256
            int index = (int) std::distance(localVariables.locals.begin(), reverse_it.base()) - 1;
            return (std::byte) index;
        }
    }

    return std::nullopt;
}

std::byte Compiler::parseVariableName() {
    Token name = expect(TokenType::IDENTIFIER, "Expected variable identifier after 'var'");

    declareVariable();
    if (localVariables.currentScopeDepth > 0) return std::byte{0};

    return emitIdentifierConstant(name);
}

void Compiler::declareVariable() {
    if (localVariables.currentScopeDepth == 0) return;

    Token name = previous();

    for (auto reverse_it = localVariables.locals.rbegin(); reverse_it != localVariables.locals.rend(); ++reverse_it){
        if (reverse_it->depth != -1 && reverse_it->depth < localVariables.currentScopeDepth){
            break;
        }

        if (reverse_it->name.lexeme == name.lexeme){
            throw LoxCompileError("Cannot redefine variable '" + name.lexeme + "'", previous().line);
        }
    }

    addLocalVariable(name);
}

void Compiler::addLocalVariable(const Token &name) {
    if (localVariables.locals.size() == 256){
        throw LoxCompileError("Too many local variables in scope", previous().line);
    }

    int uninitializedSentinel = -1;
    LocalVariables::Variable v(name, uninitializedSentinel);
    localVariables.locals.push_back(v);
}

std::byte Compiler::emitIdentifierConstant(const Token &identifier) {
    Obj* obj = Memory::allocateHeapString(identifier.lexeme);
    return emitConstant(CLoxLiteral(obj));
}

void Compiler::defineVariable(std::byte identifierOffset) {
    if (localVariables.currentScopeDepth > 0){
        markVariableInitialized();
        return;
    }

    emitByte(OpCode::OP_DEFINE_GLOBAL, identifierOffset);
}

void Compiler::markVariableInitialized() {
    if (localVariables.currentScopeDepth == 0) return;
    localVariables.locals.back().depth = localVariables.currentScopeDepth;
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

    bool canAssign = precedence <= PrecedenceLevel::ASSIGNMENT;
    //call the function to parse the token as prefix
    parseAsPrefix.value()(canAssign);

    //keep parsing tokens while the precedence level of the following token is greater than the precedence passes as a param
    while (precedence <= parsingRules.at(peek().type).precedenceLevel){
        advance();
        //get rule to parse as infix and parse
        ParseFunction parseAsInfix = parsingRules.at(previous().type).parseAsInfix;
        parseAsInfix.value()(canAssign);
    }

    if (canAssign && match(TokenType::EQUAL)){
        throw LoxCompileError("Invalid assignment target", previous().line);
    }
}

void Compiler::number(bool canAssign) {
    CLoxLiteral value(stod(previous().lexeme));
    emitConstant(value);
}

void Compiler::unary(bool canAssign) {
    TokenType type = previous().type;

    //compile operand, only parse tokens with precedence of unary or higher
    parsePrecedence(PrecedenceLevel::UNARY);

    switch (type) {
        case TokenType::MINUS:
            emitByte(static_cast<std::byte>(OpCode::OP_NEGATE));
            break;
        case TokenType::BANG:
            emitByte(static_cast<std::byte>(OpCode::OP_NOT));
            break;
        default:
            return; //unreachable
    }

}

void Compiler::binary(bool canAssign) {
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

void Compiler::grouping(bool canAssign) {
    expression();
    expect(TokenType::RIGHT_PAREN, "Expected ')' after expression");
}

void Compiler::literal(bool canAssign) {
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

void Compiler::string(bool canAssign) {
    Obj* obj = Memory::allocateHeapString(previous().lexeme);
    CLoxLiteral str(obj);
    emitConstant(str);
}

void Compiler::parseAnd(bool canAssign) {
    int endJump = emitJump(OpCode::OP_JUMP_IF_FALSE);
    emitByte(OpCode::OP_POP);
    parsePrecedence(PrecedenceLevel::AND);
    patchJump(endJump);
}

void Compiler::parseOr(bool canAssign) {
    int elseJump = emitJump(OpCode::OP_JUMP_IF_FALSE);
    int endJump = emitJump(OpCode::OP_JUMP);

    patchJump(elseJump);
    emitByte(OpCode::OP_POP);

    parsePrecedence(PrecedenceLevel::OR);
    patchJump(endJump);
}

int Compiler::emitJump(OpCode instruction) {
    emitByte(instruction);
    emitByte(std::byte(0xff));
    emitByte(std::byte(0xff));
    return currentChunk()->byteCount() - 2;
}

void Compiler::patchJump(int offset) {
    // -2 to adjust for the bytecode for the jump offset itself.
    unsigned int jump = currentChunk()->byteCount() - offset - 2;

    if (jump > UINT16_MAX){
        throw LoxCompileError("Too much code to jump over", previous().line);
    }

    currentChunk()->bytecode[offset] = std::byte((jump >> 8u) & 0xff);
    currentChunk()->bytecode[offset+1] = std::byte(jump & 0xff);
 }

void Compiler::emitLoop(int loopStart) {
    int offset = currentChunk()->byteCount() - loopStart + 2;
    if (offset > UINT16_MAX){
        throw LoxCompileError("Loop body too large", previous().line);
    }

    emitByte(OpCode::OP_LOOP);
    emitByte(std::byte((offset >> 8u) & 0xff));
    emitByte(std::byte(offset & 0xff));
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

std::byte Compiler::emitConstant(const CLoxLiteral &constant) {
    size_t constantOffset = currentChunk()->writeConstant(constant);

    ////A chunk can only hold 256 constants because 8 bits are used to represent the index of the constant in the constant pool
    //TODO: Add a special OP_CONSTANT_16 special instruction that uses 16 bits?
    if (constantOffset >= 256) {
        throw LoxCompileError("Cannot have more than 256 constants", previous().line);
    }

    emitByte(OpCode::OP_CONSTANT);
    auto offsetAsByte = static_cast<std::byte>(constantOffset);
    emitByte(offsetAsByte);
    return offsetAsByte;
}

Chunk* Compiler::currentChunk() {
    return function->chunk.get();
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
    else throw LoxCompileError(message, previous().line);
}

bool Compiler::match(const TokenType &type) {
    if (peek().type == type){
        advance();
        return true;
    }

    return false;
}

//advance compiler until it reaches a new statement
void Compiler::synchronize() {
    while (peek().type != TokenType::END_OF_FILE){
        if (previous().type == TokenType::SEMICOLON) return;

        switch (peek().type) {
            case TokenType::CLASS:
            case TokenType::FUN:
            case TokenType::VAR:
            case TokenType::FOR:
            case TokenType::IF:
            case TokenType::WHILE:
            case TokenType::PRINT:
            case TokenType::RETURN:
                return;
            default: ; //do nothing
        }

        advance();
    }
}




