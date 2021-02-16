#ifndef CLOX_COMPILER_H
#define CLOX_COMPILER_H


#include <memory>
#include <functional>
#include <map>
#include <list>
#include "CLoxLiteral.h"
#include "Chunk.h"
#include "Token.h"



enum PrecedenceLevel {
    NONE = 0,
    ASSIGNMENT = 1,  // =
    OR = 2,          // or
    AND = 3,         // and
    EQUALITY = 4,    // == !=
    COMPARISON = 5,  // < > <= >=
    TERM = 6,        // + -
    FACTOR = 7,      // * /
    UNARY = 8,       // ! -
    CALL = 9,        // . ()
    PRIMARY = 10
};

using ParseFunction = std::optional<std::function<void(bool)>>;

//Parsing rule for a pratt parser. Explanation for pratt parsers here https://journal.stuffwithstuff.com/2011/03/19/pratt-parsers-expression-parsing-made-easy/
class ParseRule {

public:
    ParseRule(ParseFunction parseAsPrefix, ParseFunction parseAsInfix, PrecedenceLevel precedenceLevel);

    ParseFunction parseAsInfix;
    ParseFunction parseAsPrefix;
    PrecedenceLevel precedenceLevel;
};

class LocalVariables {
public:
    struct Variable {
        Token name;
        int depth;

        Variable(const Token &name, int depth);
    };

    std::vector<Variable> locals;
    int currentScopeDepth = 0;
};

class Compiler {
public:
    Compiler();
    std::shared_ptr<Chunk> compile(const std::vector<Token> &tokens, bool &successFlag);

private:
    int current = 0; //index of current token
    std::vector<Token> tokens;
    std::shared_ptr<Chunk> chunk; //chunk that the compiler is building
    LocalVariables localVariables;

    //Parselets for pratt parser
    std::unordered_map<TokenType, ParseRule> parsingRules;

    /* Compiler has encountered an error so far. As opposed to the scanner and the VM,
     * the compiler may find an error and continue parsing. So the compiler will independently
     * handle and report all errors, and then return hadError to let the caller handle it as they wish.
     */
    bool hadError = false;

    void registerParsingRules();

    void parsePrecedence(PrecedenceLevel precedence);
    void declaration();
    void statement();
    void expressionStatement();
    void varDeclaration();
    void printStatement();
    void expression();

    void number(bool canAssign);
    void grouping(bool canAssign);
    void unary(bool canAssign);
    void binary(bool canAssign);
    void literal(bool canAssign);
    void string(bool canAssign);
    void variable(bool canAssign);

    void block();

    void beginScope();
    void endScope();

    void declareVariable();
    void addLocalVariable(const Token &name);
    void namedVariable(bool canAssign, const Token &name);
    std::byte parseVariableName(); //returns the offset in the chunk where the string of the variable's name is stored
    std::byte emitIdentifierConstant(const Token &identifier);
    void defineVariable(std::byte identifierOffset);

    void emitByte(OpCode opCode);
    void emitByte(OpCode opCode1, OpCode opcode2);
    void emitByte(OpCode opCode1, std::byte byte);
    void emitByte(std::byte byte);
    void emitByte(std::byte first, std::byte second);
    std::byte emitConstant(const CLoxLiteral &constant); //returns the index in the constant pool the constant was stored at

    Obj* allocateHeapObj(std::string str);

    std::shared_ptr<Chunk> currentChunk();


    Token peek(); //peeks at current token, does not consume it
    Token advance(); //returns current token and then advances by 1
    Token previous(); //returns previous token
    bool isAtEnd();
    Token expect(const TokenType &type, const std::string &errorMessage);
    bool match(const TokenType &type); //If the current token matches type, it advances and returns true. Otherwise false

    void synchronize(); //synchronizes the compiler to a normal state when it finds an error

};


#endif //CLOX_COMPILER_H
