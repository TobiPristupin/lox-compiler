#include <iostream>
#include "Chunk.h"
#include "VM.h"
#include "FileReader.h"
#include "LoxError.h"
#include "Scanner.h"
#include "Compiler.h"
#include "DebugUtils.h"


void displayCLoxUsage();
ExecutionResult runRepl();
ExecutionResult runScript(const std::string& filename);
ExecutionResult runCode(const std::string &code);

int main(int argc, char *argv[]) {
    ExecutionResult result;

    if (argc > 2) {
        displayCLoxUsage();
        result = ExecutionResult::OK;
    } else if (argc == 2) {
        result = runScript(argv[1]);
    } else {
        result = runRepl();
    }

    switch (result) {
        case ExecutionResult::OK:
            return 0;
        case ExecutionResult::COMPILE_ERROR:
            return 65;
        case ExecutionResult::RUNTIME_ERROR:
            return 70;
    }
}

ExecutionResult runScript(const std::string &filename){
    std::string code;
    try {
        FileReader reader(filename);
        code = reader.readAll();
    } catch (const LoxFileNotFoundError &error) {
        std::cout << error.what() << "\n";
        return ExecutionResult::COMPILE_ERROR;
    } catch (...) {
        std::cout << "Unknown error ocurred while reading file\n";
        return ExecutionResult::COMPILE_ERROR;
    }

    return runCode(code);
}

ExecutionResult runRepl(){
    std::cout << "Interactive Repl mode. Type \"quit()\" or press CTRL-C to exit\n";
    std::string line;
    while (true){
        std::cout << "< ";
        std::getline(std::cin, line);
        if (line == "quit()") return ExecutionResult::OK;
        try {
            runCode(line);
        } catch (const LoxError &exception){
            std::cout << exception.what() << "\n"; //use cout instead of cerr to avoid the two streams not being synchronized when printing the next '< '
        }
    }

    return ExecutionResult::OK;
}

ExecutionResult runCode(const std::string &code){
    Scanner scanner(code);
    std::vector<Token> tokens;

    try {
        tokens = scanner.scanTokens();
    } catch (const LoxScanningError& exception) {
        std::cout << exception.what() << "\n";
        return ExecutionResult::COMPILE_ERROR;
    }

//    for (Token t : tokens){
//        std::cout << t << "\n";
//    }

    Compiler compiler;
    bool successFlag;
    std::shared_ptr<Chunk> chunk = compiler.compile(tokens, successFlag);
    DebugUtils::printChunk(chunk.get(), "main");

    if (!successFlag){
        return ExecutionResult::COMPILE_ERROR;
    }

    VM vm;
    ExecutionResult result;
    try {
        result = vm.execute(chunk);
    } catch (const LoxRuntimeError &error) {
        std::cout << error.what() << "\n";
        return ExecutionResult::RUNTIME_ERROR;
    }

    return result;
}

void displayCLoxUsage(){
    std::cout << "Usage: clox [script]\n";
}
