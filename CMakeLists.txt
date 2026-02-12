cmake_minimum_required(VERSION 3.20)
project(LANGUAGE VERSION 0.3.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(LANGUAGE
    src/main.cpp
    src/lexer.cpp
    src/parser.cpp
    src/interpreter.cpp
)

target_include_directories(LANGUAGE PRIVATE src)

if(MINGW)
    target_link_options(LANGUAGE PRIVATE -static -static-libgcc -static-libstdc++)
endif()
