
#include "Chunk.h"

size_t Chunk::write(std::byte code, int line) {
    bytecode.push_back(code);
    lines.push_back(line);
    return bytecode.size() - 1;
}

size_t Chunk::writeInstruction(OpCode code, int line) {
    return write(std::byte(code), line);
}

std::byte Chunk::readByte(int offset) const {
    return bytecode.at(offset);
}

size_t Chunk::byteCount() const {
    return bytecode.size();
}

size_t Chunk::writeConstant(const Value &value) {
    constants.push_back(value);
    return constants.size() - 1;
}

Value Chunk::readConstant(int offset) const {
    return constants.at(offset);
}

size_t Chunk::constantCount() const {
    return constants.size();
}

size_t Chunk::writeLine(int line) {
    lines.push_back(line);
    return lines.size() - 1;
}

int Chunk::readLine(int offset) const {
    return lines.at(offset);
}

size_t Chunk::lineCount() const {
    return lines.size();
}


