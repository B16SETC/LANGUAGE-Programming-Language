#include "lexer.h"
#include <cctype>
#include <stdexcept>
#include <stack>

Lexer::Lexer(const std::string& source) 
    : source(source), pos(0), line(1), current_indent(0), current_char(source.empty() ? '\0' : source[0]) {}

void Lexer::advance() {
    pos++;
    if (pos >= source.length()) {
        current_char = '\0';
    } else {
        current_char = source[pos];
    }
}

void Lexer::skip_whitespace_inline() {
    while (current_char != '\0' && current_char != '\n' && std::isspace(current_char)) {
        advance();
    }
}

int Lexer::count_indent() {
    int spaces = 0;
    while (current_char == ' ') {
        spaces++;
        advance();
    }
    return spaces / 2;
}

Token Lexer::number() {
    std::string num;
    int start_line = line;
    
    while (current_char != '\0' && (std::isdigit(current_char) || current_char == '.')) {
        num += current_char;
        advance();
    }
    
    return {TokenType::NUMBER, num, start_line, current_indent};
}

Token Lexer::identifier() {
    std::string id;
    int start_line = line;
    
    while (current_char != '\0' && (std::isalnum(current_char) || current_char == '_')) {
        id += current_char;
        advance();
    }
    
    if (id == "Print") {
        return {TokenType::PRINT, id, start_line, current_indent};
    }
    if (id == "If") {
        return {TokenType::IF, id, start_line, current_indent};
    }
    if (id == "End") {
        return {TokenType::END, id, start_line, current_indent};
    }
    
    return {TokenType::IDENTIFIER, id, start_line, current_indent};
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    std::stack<int> indent_stack;
    indent_stack.push(0);
    
    bool at_line_start = true;
    
    while (current_char != '\0') {
        if (at_line_start && current_char != '\n') {
            int indent = count_indent();
            
            if (current_char == '\0' || current_char == '\n') {
                continue;
            }
            
            if (indent > indent_stack.top()) {
                indent_stack.push(indent);
                tokens.push_back({TokenType::INDENT, "", line, indent});
                current_indent = indent;
            } else if (indent < indent_stack.top()) {
                while (indent_stack.size() > 1 && indent < indent_stack.top()) {
                    indent_stack.pop();
                    tokens.push_back({TokenType::DEDENT, "", line, indent});
                }
                current_indent = indent;
            } else {
                current_indent = indent;
            }
            
            at_line_start = false;
        }
        
        if (current_char == '\n') {
            tokens.push_back({TokenType::NEWLINE, "\\n", line, current_indent});
            line++;
            advance();
            at_line_start = true;
            continue;
        }
        
        if (std::isspace(current_char)) {
            skip_whitespace_inline();
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
        
        if (current_char == '=') {
            advance();
            if (current_char == '=') {
                tokens.push_back({TokenType::EQUAL, "==", current_line, current_indent});
                advance();
            } else {
                tokens.push_back({TokenType::ASSIGN, "=", current_line, current_indent});
            }
            continue;
        }
        
        if (current_char == '!') {
            advance();
            if (current_char == '=') {
                tokens.push_back({TokenType::NOT_EQUAL, "!=", current_line, current_indent});
                advance();
            } else {
                throw std::runtime_error("Unexpected character: !");
            }
            continue;
        }
        
        if (current_char == '<') {
            advance();
            if (current_char == '=') {
                tokens.push_back({TokenType::LESS_EQUAL, "<=", current_line, current_indent});
                advance();
            } else {
                tokens.push_back({TokenType::LESS_THAN, "<", current_line, current_indent});
            }
            continue;
        }
        
        if (current_char == '>') {
            advance();
            if (current_char == '=') {
                tokens.push_back({TokenType::GREATER_EQUAL, ">=", current_line, current_indent});
                advance();
            } else {
                tokens.push_back({TokenType::GREATER_THAN, ">", current_line, current_indent});
            }
            continue;
        }
        
        switch (current_char) {
            case '+':
                tokens.push_back({TokenType::PLUS, "+", current_line, current_indent});
                advance();
                break;
            case '-':
                tokens.push_back({TokenType::MINUS, "-", current_line, current_indent});
                advance();
                break;
            case '*':
                tokens.push_back({TokenType::MULTIPLY, "*", current_line, current_indent});
                advance();
                break;
            case '/':
                tokens.push_back({TokenType::DIVIDE, "/", current_line, current_indent});
                advance();
                break;
            default:
                throw std::runtime_error("Unknown character: " + std::string(1, current_char));
        }
    }
    
    while (indent_stack.size() > 1) {
        indent_stack.pop();
        tokens.push_back({TokenType::DEDENT, "", line, 0});
    }
    
    tokens.push_back({TokenType::END_OF_FILE, "", line, 0});
    return tokens;
}
