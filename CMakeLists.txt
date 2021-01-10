cmake_minimum_required(VERSION 3.16)
project(clox)

set(CMAKE_CXX_STANDARD 17)

add_executable(clox main.cpp Chunk.h Chunk.cpp DebugUtils.cpp DebugUtils.h LoxValue.cpp LoxValue.h)

add_executable(test test.cpp)