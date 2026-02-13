#include "parser.h"
#include <stdexcept>

Parser::Parser(const std::vector<Token>& tokens)
    : tokens(tokens), pos(0), current_token(tokens[0]) {}

void Parser::advance() {
    pos++;
    if (pos < tokens.size()) current_token = tokens[pos];
}

void Parser::consume_newlines() {
    while (current_token.type == TokenType::NEWLINE) advance();
}

std::vector<std::unique_ptr<ASTNode>> Parser::parse_block() {
    std::vector<std::unique_ptr<ASTNode>> stmts;

    if (current_token.type != TokenType::INDENT) {
        throw std::runtime_error("Expected indented block");
    }
    advance();

    while (current_token.type != TokenType::DEDENT &&
           current_token.type != TokenType::END &&
           current_token.type != TokenType::ELSE &&
           current_token.type != TokenType::END_OF_FILE) {
        auto stmt = statement();
        if (stmt) stmts.push_back(std::move(stmt));
        if (current_token.type == TokenType::NEWLINE) advance();
        consume_newlines();
    }

    if (current_token.type == TokenType::DEDENT) advance();
    return stmts;
}

std::unique_ptr<ASTNode> Parser::factor() {
    Token token = current_token;

    if (token.type == TokenType::NUMBER) {
        advance();
        return std::make_unique<NumberNode>(std::stod(token.value));
    }

    if (token.type == TokenType::STRING) {
        advance();
        return std::make_unique<StringNode>(token.value);
    }

    if (token.type == TokenType::IDENTIFIER) {
        advance();
        if (current_token.type == TokenType::LPAREN) {
            advance();
            std::vector<std::unique_ptr<ASTNode>> args;
            while (current_token.type != TokenType::RPAREN &&
                   current_token.type != TokenType::END_OF_FILE) {
                args.push_back(comparison());
                if (current_token.type == TokenType::COMMA) advance();
            }
            if (current_token.type != TokenType::RPAREN) {
                throw std::runtime_error("Expected ')' after function arguments");
            }
            advance();

            if (token.value == "Length") {
                return std::make_unique<StringOpNode>("Length", std::move(args[0]), std::vector<std::unique_ptr<ASTNode>>{});
            }
            if (token.value == "Upper") {
                return std::make_unique<StringOpNode>("Upper", std::move(args[0]), std::vector<std::unique_ptr<ASTNode>>{});
            }
            if (token.value == "Lower") {
                return std::make_unique<StringOpNode>("Lower", std::move(args[0]), std::vector<std::unique_ptr<ASTNode>>{});
            }
            if (token.value == "Contains") {
                auto target = std::move(args[0]);
                std::vector<std::unique_ptr<ASTNode>> rest;
                for (size_t i = 1; i < args.size(); i++) rest.push_back(std::move(args[i]));
                return std::make_unique<StringOpNode>("Contains", std::move(target), std::move(rest));
            }
            if (token.value == "Substring") {
                auto target = std::move(args[0]);
                std::vector<std::unique_ptr<ASTNode>> rest;
                for (size_t i = 1; i < args.size(); i++) rest.push_back(std::move(args[i]));
                return std::make_unique<StringOpNode>("Substring", std::move(target), std::move(rest));
            }

            return std::make_unique<FuncCallNode>(token.value, std::move(args));
        }
        return std::make_unique<VariableNode>(token.value);
    }

    throw std::runtime_error("Unexpected token in expression: " + token.value);
}

std::unique_ptr<ASTNode> Parser::term() {
    auto node = factor();
    while (current_token.type == TokenType::MULTIPLY || current_token.type == TokenType::DIVIDE) {
        TokenType op = current_token.type;
        advance();
        node = std::make_unique<BinaryOpNode>(op, std::move(node), factor());
    }
    return node;
}

std::unique_ptr<ASTNode> Parser::expression() {
    auto node = term();
    while (current_token.type == TokenType::PLUS || current_token.type == TokenType::MINUS) {
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
        return std::make_unique<ComparisonNode>(op, std::move(left), expression());
    }
    return left;
}

std::unique_ptr<ASTNode> Parser::if_statement() {
    advance();
    auto condition = comparison();
    if (current_token.type != TokenType::NEWLINE) throw std::runtime_error("Expected newline after If condition");
    advance();

    auto body = parse_block();

    std::vector<std::unique_ptr<ASTNode>> else_body;
    if (current_token.type == TokenType::ELSE) {
        advance();
        if (current_token.type != TokenType::NEWLINE) throw std::runtime_error("Expected newline after Else");
        advance();
        else_body = parse_block();
    }

    if (current_token.type == TokenType::END) advance();
    return std::make_unique<IfStatementNode>(std::move(condition), std::move(body), std::move(else_body));
}

std::unique_ptr<ASTNode> Parser::while_statement() {
    advance();
    auto condition = comparison();
    if (current_token.type != TokenType::NEWLINE) throw std::runtime_error("Expected newline after While condition");
    advance();
    auto body = parse_block();
    if (current_token.type == TokenType::END) advance();
    return std::make_unique<WhileLoopNode>(std::move(condition), std::move(body));
}

std::unique_ptr<ASTNode> Parser::func_def() {
    advance();
    if (current_token.type != TokenType::IDENTIFIER) throw std::runtime_error("Expected function name");
    std::string name = current_token.value;
    advance();

    if (current_token.type != TokenType::LPAREN) throw std::runtime_error("Expected '(' after function name");
    advance();

    std::vector<std::string> params;
    while (current_token.type != TokenType::RPAREN && current_token.type != TokenType::END_OF_FILE) {
        if (current_token.type != TokenType::IDENTIFIER) throw std::runtime_error("Expected parameter name");
        params.push_back(current_token.value);
        advance();
        if (current_token.type == TokenType::COMMA) advance();
    }
    if (current_token.type != TokenType::RPAREN) throw std::runtime_error("Expected ')' after parameters");
    advance();

    if (current_token.type != TokenType::NEWLINE) throw std::runtime_error("Expected newline after function definition");
    advance();

    auto body = parse_block();
    if (current_token.type == TokenType::END) advance();
    return std::make_unique<FuncDefNode>(name, std::move(params), std::move(body));
}

std::unique_ptr<ASTNode> Parser::statement() {
    if (current_token.type == TokenType::PRINT) {
        advance();
        return std::make_unique<PrintNode>(expression());
    }
    if (current_token.type == TokenType::IF)    return if_statement();
    if (current_token.type == TokenType::WHILE) return while_statement();
    if (current_token.type == TokenType::FUNC)  return func_def();

    if (current_token.type == TokenType::RETURN) {
        advance();
        return std::make_unique<ReturnNode>(expression());
    }

    if (current_token.type == TokenType::IDENTIFIER) {
        std::string var_name = current_token.value;
        advance();

        if (current_token.type == TokenType::ASSIGN) {
            advance();
            return std::make_unique<AssignmentNode>(var_name, expression());
        }

        if (current_token.type == TokenType::LPAREN) {
            advance();
            std::vector<std::unique_ptr<ASTNode>> args;
            while (current_token.type != TokenType::RPAREN && current_token.type != TokenType::END_OF_FILE) {
                args.push_back(comparison());
                if (current_token.type == TokenType::COMMA) advance();
            }
            if (current_token.type != TokenType::RPAREN) throw std::runtime_error("Expected ')'");
            advance();
            return std::make_unique<FuncCallNode>(var_name, std::move(args));
        }

        throw std::runtime_error("Expected '=' or '(' after identifier");
    }

    return nullptr;
}

std::vector<std::unique_ptr<ASTNode>> Parser::parse() {
    std::vector<std::unique_ptr<ASTNode>> statements;
    consume_newlines();
    while (current_token.type != TokenType::END_OF_FILE) {
        auto stmt = statement();
        if (stmt) statements.push_back(std::move(stmt));
        if (current_token.type == TokenType::NEWLINE) {
            advance();
            consume_newlines();
        } else if (current_token.type != TokenType::END_OF_FILE) {
            throw std::runtime_error("Expected newline after statement on line " + std::to_string(current_token.line));
        }
    }
    return statements;
}
