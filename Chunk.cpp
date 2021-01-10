
#include "Chunk.h"

size_t Chunk::write(std::byte code, int line) {
    bytecode.push_back(code);
    writeLine(line);
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

/* Writes a line into the lines array.
 *
 * ASSUMPTION: lines are added incrementally. If writeLine(4) is called, this
 * method assumes that writeLine({any number less than 4}) will never be called again.
 * Otherwise it will break the RLE encoding scheme.
 *
 * The lines vector stores line data using a run-length encoding scheme, because line numbers
 * are usually repeated given that one line contains multiple instructions. For example,
 * the following data : [1, 1, 1, 1, 2, 2, 3] is stored as [1, 4, 2, 2, 3, 1].
 * */
void Chunk::writeLine(int line) {
    if (lines.empty() || lines.at(lines.size() - 2) != line){
        lines.push_back(line);
        lines.push_back(1);
        return;
    }

    lines.back() += 1;
}

/* Returns the line number corresponding to the instruction at instructionOffset.
 *
 * Lines vector uses run-length encoding to store line data. This method takes in the offset of the instruction and
 * finds the line number corresponding to that instruction. Speed is not important in this method because it is only
 * called when the user's code crashes to give line number information to the user.
 */
int Chunk::readLine(int instructionOffset) const {
    //[1, 3, 2, 2, 3, 1], offset=5


    int currentOffset = lines[1];
    int index = 0;
    while (instructionOffset + 1 > currentOffset){
        index += 2;
        currentOffset += lines[index+1];
    }

    return lines.at(index);
}

size_t Chunk::lineCount() const {
    return lines.size();
}


