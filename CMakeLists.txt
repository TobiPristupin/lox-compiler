cmake_minimum_required(VERSION 3.16)
project(clox)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_GLIBCXX_DEBUG")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")


add_executable(clox-marksweep main.cpp Chunk.h Chunk.cpp DebugUtils.cpp DebugUtils.h LoxValue.cpp LoxValue.h VM.cpp VM.h FileReader.h FileReader.cpp Compiler.cpp Compiler.h Token.cpp Token.h Scanner.cpp Scanner.h TokenType.h TokenType.cpp LoxError.h LoxError.cpp CLoxLiteral.cpp CLoxLiteral.h Utils.cpp Utils.h Memory.cpp Memory.h)

add_executable(test test.cpp)