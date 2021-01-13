#ifndef CLOX_COMPILER_H
#define CLOX_COMPILER_H


#include <memory>
#include "Chunk.h"
#include "Token.h"

class Compiler {
public:
    std::shared_ptr<Chunk> compile(const std::vector<Token> &tokens);

private:
    std::vector<Token> tokens;

};


#endif //CLOX_COMPILER_H
