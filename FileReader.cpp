
#include "FileReader.h"
#include <iterator>
#include "LoxError.h"


FileReader::FileReader(const std::string &filename) : filename(filename) {
    fileStream.open(filename);
    if (!fileStream.is_open()){
        throw LoxFileNotFoundError("File " + filename + " not found");
    }
}

std::string FileReader::readAll() {
    std::string data((std::istreambuf_iterator<char>(fileStream)),std::istreambuf_iterator<char>());
    return data;
}

FileReader::~FileReader() {
    fileStream.close();
}


