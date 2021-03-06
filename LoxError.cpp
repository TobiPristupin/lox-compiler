#include "LoxError.h"

LoxError::LoxError(const std::string &message, int line, int pos) : line(line), pos(pos) {
    this->message = "";
    if (line != -1){
        if (pos != -1){
            this->message += "[Line " + std::to_string(line) + ":" + std::to_string(pos) + "] ";
        } else {
            this->message += "[Line " + std::to_string(line) + "] ";
        }
    }

    this->message += "Error: " + message;
}

const char *LoxError::what() const noexcept {
    return message.c_str();
}

LoxScanningError::LoxScanningError(const std::string &message, int line, int pos) : LoxError(message, line, pos) {
    this->message = "[Line " + std::to_string(line) + ":" + std::to_string(pos) + "] " +  "Scanning Error: " + message;
}

const char *LoxScanningError::what() const noexcept {
    return message.c_str();
}

LoxRuntimeError::LoxRuntimeError(const std::string &message, int line) : LoxError(message, line) {
    if (line != -1){
        this->message = "[Line " + std::to_string(line) + "] " +  "Runtime Error: " + message;
    } else {
        this->message = "Runtime Error: " + message;
    }
}

const char *LoxRuntimeError::what() const noexcept {
    return message.c_str();
}

LoxFileNotFoundError::LoxFileNotFoundError(const std::string &message) : LoxError(message) {}

LoxCompileError::LoxCompileError(const std::string &message, int line) : LoxError(message, line) {
    this->message = "[Line " + std::to_string(line) + "] " +  "Compile Error: " + message;
}

const char *LoxCompileError::what() const noexcept {
    return message.c_str();
}



