#include <iostream>
#include <chrono>
#include <thread>
#include <functional>
#include "VM.h"
#include "FileReader.h"
#include "LoxError.h"
#include "Scanner.h"
#include "Compiler.h"
#include "DebugUtils.h"
#include "Memory.h"

#define LOG_HEAP


void displayCLoxUsage();
ExecutionResult runRepl();
ExecutionResult runScript(const std::string& filename);
ExecutionResult runCode(const std::string &code);

auto getEpochTimeMillis(){
    return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

int main(int argc, char *argv[]) {
    if (argc != 5){
        displayCLoxUsage();
        return 0;
    }

#ifdef LOG_HEAP
    std::ofstream out(argv[2]);
    auto old_rdbuf = std::clog.rdbuf();
    std::clog.rdbuf(out.rdbuf());
    std::clog << getEpochTimeMillis() << "\n";
#endif

//    Memory::nextGCByteThreshold = std::stoi(argv[3]);
//    Memory::heapGrowFactor = std::stoi(argv[4]);

    ExecutionResult result;

    result = runScript(argv[1]);

    int exitCode;

    switch (result) {
        case ExecutionResult::OK:
            exitCode = 0;
            break;
        case ExecutionResult::COMPILE_ERROR:
            exitCode = 65;
            break;
        case ExecutionResult::RUNTIME_ERROR:
            exitCode = 70;
            break;
    }

    std::clog << getEpochTimeMillis() << "\n";

#ifdef LOG_HEAP
    std::clog.rdbuf(old_rdbuf);
#endif

    return exitCode;
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
        std::cout << "Unknown error occurred while reading file\n";
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
    FunctionObj *function = compiler.compile(tokens, successFlag);
    DebugUtils::printChunk(function->chunk, "main");

    if (!successFlag){
        return ExecutionResult::COMPILE_ERROR;
    }

    VM vm;
    ExecutionResult result;
    try {
        result = vm.execute(function);
    } catch (const LoxRuntimeError &error) {
        std::cout << error.what() << "\n";
        return ExecutionResult::RUNTIME_ERROR;
    }

    return result;
}

void displayCLoxUsage(){
    std::cout << "Usage: clox [script] [GC Log File] [GC Byte Threshold] [GC Heap Grow Factor]\n";
}


