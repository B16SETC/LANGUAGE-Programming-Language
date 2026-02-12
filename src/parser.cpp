#include "parser.h"
#include <stdexcept>

Parser::Parser(const std::vector<Token>& tokens) 
    : tokens(tokens), pos(0), current_token(tokens[0]) {}

void Parser::advance() {
    pos++;
    if (pos < tokens.size()) {
        current_token = tokens[pos];
    }
}

void Parser::consume_newlines() {
    while (current_token.type == TokenType::NEWLINE) {
        advance();
    }
}

std::unique_ptr<ASTNode> Parser::factor() {
    Token token = current_token;
    
    if (token.type == TokenType::NUMBER) {
        advance();
        return std::make_unique<NumberNode>(std::stod(token.value));
    }
    
    if (token.type == TokenType::IDENTIFIER) {
        advance();
        return std::make_unique<VariableNode>(token.value);
    }
    
    throw std::runtime_error("Unexpected token in expression");
}

std::unique_ptr<ASTNode> Parser::term() {
    auto node = factor();
    
    while (current_token.type == TokenType::MULTIPLY || 
           current_token.type == TokenType::DIVIDE) {
        TokenType op = current_token.type;
        advance();
        node = std::make_unique<BinaryOpNode>(op, std::move(node), factor());
    }
    
    return node;
}

std::unique_ptr<ASTNode> Parser::expression() {
    auto node = term();
    
    while (current_token.type == TokenType::PLUS || 
           current_token.type == TokenType::MINUS) {
        TokenType op = current_token.type;
        advance();
        node = std::make_unique<BinaryOpNode>(op, std::move(node), term());
    }
    
    return node;
}

std::unique_ptr<ASTNode> Parser::comparison() {
    auto left = expression();
    
    if (current_token.type == TokenType::EQUAL ||
        current_token.type == TokenType::NOT_EQUAL ||
        current_token.type == TokenType::LESS_THAN ||
        current_token.type == TokenType::GREATER_THAN ||
        current_token.type == TokenType::LESS_EQUAL ||
        current_token.type == TokenType::GREATER_EQUAL) {
        
        TokenType op = current_token.type;
        advance();
        auto right = expression();
        return std::make_unique<ComparisonNode>(op, std::move(left), std::move(right));
    }
    
    return left;
}

std::unique_ptr<ASTNode> Parser::if_statement() {
    advance();
    
    auto condition = comparison();
    
    if (current_token.type != TokenType::NEWLINE) {
        throw std::runtime_error("Expected newline after If condition");
    }
    advance();
    
    if (current_token.type != TokenType::INDENT) {
        throw std::runtime_error("Expected indented block after If");
    }
    advance();
    
    std::vector<std::unique_ptr<ASTNode>> body;
    
    while (current_token.type != TokenType::DEDENT && 
           current_token.type != TokenType::END &&
           current_token.type != TokenType::END_OF_FILE) {
        
        auto stmt = statement();
        if (stmt) {
            body.push_back(std::move(stmt));
        }
        
        if (current_token.type == TokenType::NEWLINE) {
            advance();
        }
    }
    
    if (current_token.type == TokenType::DEDENT) {
        advance();
    }
    
    if (current_token.type == TokenType::END) {
        advance();
    }
    
    return std::make_unique<IfStatementNode>(std::move(condition), std::move(body));
}

std::unique_ptr<ASTNode> Parser::while_statement() {
    advance();

    auto condition = comparison();

    if (current_token.type != TokenType::NEWLINE) {
        throw std::runtime_error("Expected newline after While condition");
    }
    advance();

    if (current_token.type != TokenType::INDENT) {
        throw std::runtime_error("Expected indented block after While");
    }
    advance();

    std::vector<std::unique_ptr<ASTNode>> body;

    while (current_token.type != TokenType::DEDENT &&
           current_token.type != TokenType::END &&
           current_token.type != TokenType::END_OF_FILE) {

        auto stmt = statement();
        if (stmt) {
            body.push_back(std::move(stmt));
        }

        if (current_token.type == TokenType::NEWLINE) {
            advance();
        }
    }

    if (current_token.type == TokenType::DEDENT) {
        advance();
    }

    if (current_token.type == TokenType::END) {
        advance();
    }

    return std::make_unique<WhileLoopNode>(std::move(condition), std::move(body));
}

std::unique_ptr<ASTNode> Parser::statement() {
    if (current_token.type == TokenType::PRINT) {
        advance();
        auto expr = expression();
        return std::make_unique<PrintNode>(std::move(expr));
    }
    
    if (current_token.type == TokenType::IF) {
        return if_statement();
    }

    if (current_token.type == TokenType::WHILE) {
        return while_statement();
    }
    
    if (current_token.type == TokenType::IDENTIFIER) {
        std::string var_name = current_token.value;
        advance();
        
        if (current_token.type == TokenType::ASSIGN) {
            advance();
            auto value = expression();
            return std::make_unique<AssignmentNode>(var_name, std::move(value));
        }
        
        throw std::runtime_error("Expected '=' after variable name");
    }
    
    return nullptr;
}

std::vector<std::unique_ptr<ASTNode>> Parser::parse() {
    std::vector<std::unique_ptr<ASTNode>> statements;
    
    consume_newlines();
    
    while (current_token.type != TokenType::END_OF_FILE) {
        auto stmt = statement();
        if (stmt) {
            statements.push_back(std::move(stmt));
        }
        
        if (current_token.type == TokenType::NEWLINE) {
            advance();
            consume_newlines();
        } else if (current_token.type != TokenType::END_OF_FILE) {
            throw std::runtime_error("Expected newline after statement");
        }
    }
    
    return statements;
}
