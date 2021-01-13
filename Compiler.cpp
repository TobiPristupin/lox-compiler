#include "Compiler.h"

std::shared_ptr<Chunk> Compiler::compile(const std::vector<Token> &tokens) {
    this->tokens = tokens;
    return std::make_shared<Chunk>();
}
