#include <iostream>
#include <fstream>
#include <sstream>
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"

const std::string VERSION = "0.5.0";

void print_usage() {
    std::cout << "LANGUAGE Programming Language v" << VERSION << "\n";
    std::cout << "\nUsage:\n";
    std::cout << "  LANGUAGE <script.LANGUAGE>     Run a LANGUAGE script\n";
    std::cout << "  LANGUAGE --version             Show version information\n";
    std::cout << "  LANGUAGE --help                Show this help message\n";
}

void print_version() {
    std::cout << "LANGUAGE v" << VERSION << "\n";
}

std::string read_file(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + filename);
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    
    // Normalize line endings (handle both \r\n and \n)
    std::string normalized;
    for (size_t i = 0; i < content.length(); i++) {
        if (content[i] == '\r' && i + 1 < content.length() && content[i + 1] == '\n') {
            continue; // Skip \r in \r\n
        }
        normalized += content[i];
    }
    
    return normalized;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage();
        return 1;
    }
    
    std::string arg = argv[1];
    
    if (arg == "--version") {
        print_version();
        return 0;
    }
    
    if (arg == "--help") {
        print_usage();
        return 0;
    }
    
    try {
        std::string source = read_file(arg);
        
        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        
        Parser parser(tokens);
        auto ast = parser.parse();
        
        Interpreter interpreter;
        interpreter.execute(ast);
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
