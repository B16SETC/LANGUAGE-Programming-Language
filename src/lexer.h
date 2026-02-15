#pragma once
#include <string>
#include <vector>

enum class TokenType {
    NUMBER,
    STRING,
    IDENTIFIER,
    PLUS,
    MINUS,
    MULTIPLY,
    DIVIDE,
    ASSIGN,
    EQUAL,
    NOT_EQUAL,
    LESS_THAN,
    GREATER_THAN,
    LESS_EQUAL,
    GREATER_EQUAL,
    LPAREN,
    RPAREN,
    LBRACKET,
    RBRACKET,
    COMMA,
    PRINT,
    IF,
    ELIF,
    ELSE,
    WHILE,
    FOR,
    TO,
    FUNC,
    RETURN,
    END,
    AND,
    OR,
    NOT,
    TRUE,
    FALSE,
    NEWLINE,
    INDENT,
    DEDENT,
    END_OF_FILE
};

struct Token {
    TokenType type;
    std::string value;
    int line;
    int indent_level;
};

class Lexer {
public:
    Lexer(const std::string& source);
    std::vector<Token> tokenize();

private:
    std::string source;
    size_t pos;
    int line;
    int current_indent;
    char current_char;

    void advance();
    void skip_whitespace_inline();
    int count_indent();
    Token number();
    Token string_literal();
    Token identifier();
};
