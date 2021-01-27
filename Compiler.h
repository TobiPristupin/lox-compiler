#ifndef CLOX_COMPILER_H
#define CLOX_COMPILER_H


#include <memory>
#include "Chunk.h"
#include "Token.h"

class Compiler {
public:
    std::shared_ptr<Chunk> compile(const std::vector<Token> &tokens, bool &successFlag);

private:
    int current = 0; //index of current token
    std::vector<Token> tokens;
    std::shared_ptr<Chunk> chunk; //chunk that the compiler is building

    /* Compiler has encountered an error so far. As opposed to the scanner and the VM,
     * the compiler may find an error and continue parsing. So the compiler will independently
     * handle and report all errors, and then return hadError to let the caller handle it as they wish.
     */
    bool hadError = false;

    void emitByte(std::byte byte);
    void emitByte(std::byte first, std::byte second);

    std::shared_ptr<Chunk> currentChunk();

    void number();


    Token peek(); //peeks at current token, does not consume it
    Token advance(); //returns current token and then advances by 1
    Token previous(); //returns previous token
    bool isAtEnd();
    Token expect(const TokenType &type, const std::string &errorMessage);

    void synchronize(); //synchronizes the compiler to a normal state when it finds an error

};


#endif //CLOX_COMPILER_H
