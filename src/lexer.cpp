#include "lexer.h"
#include <cctype>
#include <stdexcept>

Lexer::Lexer(const std::string& source) 
    : source(source), pos(0), line(1), current_char(source.empty() ? '\0' : source[0]) {}

void Lexer::advance() {
    pos++;
    if (pos >= source.length()) {
        current_char = '\0';
    } else {
        current_char = source[pos];
    }
}

void Lexer::skip_whitespace() {
    while (current_char != '\0' && std::isspace(current_char) && current_char != '\n') {
        advance();
    }
}

Token Lexer::number() {
    std::string num;
    int start_line = line;
    
    while (current_char != '\0' && (std::isdigit(current_char) || current_char == '.')) {
        num += current_char;
        advance();
    }
    
    return {TokenType::NUMBER, num, start_line};
}

Token Lexer::identifier() {
    std::string id;
    int start_line = line;
    
    while (current_char != '\0' && (std::isalnum(current_char) || current_char == '_')) {
        id += current_char;
        advance();
    }
    
    if (id == "Print") {
        return {TokenType::PRINT, id, start_line};
    }
    
    return {TokenType::IDENTIFIER, id, start_line};
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    
    while (current_char != '\0') {
        if (std::isspace(current_char) && current_char != '\n') {
            skip_whitespace();
            continue;
        }
        
        if (current_char == '\n') {
            tokens.push_back({TokenType::NEWLINE, "\\n", line});
            line++;
            advance();
            continue;
        }
        
        if (std::isdigit(current_char)) {
            tokens.push_back(number());
            continue;
        }
        
        if (std::isalpha(current_char) || current_char == '_') {
            tokens.push_back(identifier());
            continue;
        }
        
        int current_line = line;
        
        switch (current_char) {
            case '+':
                tokens.push_back({TokenType::PLUS, "+", current_line});
                advance();
                break;
            case '-':
                tokens.push_back({TokenType::MINUS, "-", current_line});
                advance();
                break;
            case '*':
                tokens.push_back({TokenType::MULTIPLY, "*", current_line});
                advance();
                break;
            case '/':
                tokens.push_back({TokenType::DIVIDE, "/", current_line});
                advance();
                break;
            case '=':
                tokens.push_back({TokenType::ASSIGN, "=", current_line});
                advance();
                break;
            default:
                throw std::runtime_error("Unknown character: " + std::string(1, current_char));
        }
    }
    
    tokens.push_back({TokenType::END_OF_FILE, "", line});
    return tokens;
}
