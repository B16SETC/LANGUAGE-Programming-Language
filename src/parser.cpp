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
    if (current_token.type != TokenType::INDENT)
        throw std::runtime_error("Expected indented block on line " + std::to_string(current_token.line));
    advance();

    while (current_token.type != TokenType::DEDENT &&
           current_token.type != TokenType::END &&
           current_token.type != TokenType::ELSE &&
           current_token.type != TokenType::ELIF &&
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
    if (token.type == TokenType::TRUE) {
        advance();
        return std::make_unique<BooleanNode>(true);
    }
    if (token.type == TokenType::FALSE) {
        advance();
        return std::make_unique<BooleanNode>(false);
    }
    if (token.type == TokenType::NOT) {
        advance();
        return std::make_unique<NotOpNode>(logical());
    }
    if (token.type == TokenType::LBRACKET) {
        advance();
        std::vector<std::unique_ptr<ASTNode>> elements;
        while (current_token.type != TokenType::RBRACKET && current_token.type != TokenType::END_OF_FILE) {
            elements.push_back(logical());
            if (current_token.type == TokenType::COMMA) advance();
        }
        if (current_token.type != TokenType::RBRACKET)
            throw std::runtime_error("Expected ']' after array elements");
        advance();
        return std::make_unique<ArrayNode>(std::move(elements));
    }
    if (token.type == TokenType::IDENTIFIER) {
        advance();

        if (current_token.type == TokenType::LBRACKET) {
            advance();
            auto index = logical();
            if (current_token.type != TokenType::RBRACKET)
                throw std::runtime_error("Expected ']' after array index");
            advance();
            return std::make_unique<ArrayAccessNode>(token.value, std::move(index));
        }

        if (current_token.type == TokenType::LPAREN) {
            advance();
            std::vector<std::unique_ptr<ASTNode>> args;
            while (current_token.type != TokenType::RPAREN && current_token.type != TokenType::END_OF_FILE) {
                args.push_back(logical());
                if (current_token.type == TokenType::COMMA) advance();
            }
            if (current_token.type != TokenType::RPAREN)
                throw std::runtime_error("Expected ')' after arguments");
            advance();

            if (token.value == "Length")    return std::make_unique<StringOpNode>("Length",    std::move(args[0]), std::vector<std::unique_ptr<ASTNode>>{});
            if (token.value == "Upper")     return std::make_unique<StringOpNode>("Upper",     std::move(args[0]), std::vector<std::unique_ptr<ASTNode>>{});
            if (token.value == "Lower")     return std::make_unique<StringOpNode>("Lower",     std::move(args[0]), std::vector<std::unique_ptr<ASTNode>>{});
            if (token.value == "Push")      return std::make_unique<StringOpNode>("Push",      std::move(args[0]), [&]{ std::vector<std::unique_ptr<ASTNode>> r; r.push_back(std::move(args[1])); return r; }());
            if (token.value == "Pop")       return std::make_unique<StringOpNode>("Pop",       std::move(args[0]), std::vector<std::unique_ptr<ASTNode>>{});
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

    throw std::runtime_error("Unexpected token: '" + token.value + "' on line " + std::to_string(token.line));
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

std::unique_ptr<ASTNode> Parser::logical() {
    auto left = comparison();
    while (current_token.type == TokenType::AND || current_token.type == TokenType::OR) {
        TokenType op = current_token.type;
        advance();
        left = std::make_unique<LogicalOpNode>(op, std::move(left), comparison());
    }
    return left;
}

std::unique_ptr<ASTNode> Parser::if_statement() {
    advance();
    auto condition = logical();
    if (current_token.type != TokenType::NEWLINE)
        throw std::runtime_error("Expected newline after If condition");
    advance();

    auto body = parse_block();

    std::vector<ElifClause> elif_clauses;
    while (current_token.type == TokenType::ELIF) {
        advance();
        auto elif_cond = logical();
        if (current_token.type != TokenType::NEWLINE)
            throw std::runtime_error("Expected newline after Elif condition");
        advance();
        auto elif_body = parse_block();
        ElifClause clause;
        clause.condition = std::move(elif_cond);
        clause.body = std::move(elif_body);
        elif_clauses.push_back(std::move(clause));
    }

    std::vector<std::unique_ptr<ASTNode>> else_body;
    if (current_token.type == TokenType::ELSE) {
        advance();
        if (current_token.type != TokenType::NEWLINE)
            throw std::runtime_error("Expected newline after Else");
        advance();
        else_body = parse_block();
    }

    if (current_token.type == TokenType::END) advance();
    return std::make_unique<IfStatementNode>(std::move(condition), std::move(body), std::move(elif_clauses), std::move(else_body));
}

std::unique_ptr<ASTNode> Parser::while_statement() {
    advance();
    auto condition = logical();
    if (current_token.type != TokenType::NEWLINE)
        throw std::runtime_error("Expected newline after While condition");
    advance();
    auto body = parse_block();
    if (current_token.type == TokenType::END) advance();
    return std::make_unique<WhileLoopNode>(std::move(condition), std::move(body));
}

std::unique_ptr<ASTNode> Parser::for_statement() {
    advance();
    if (current_token.type != TokenType::IDENTIFIER)
        throw std::runtime_error("Expected variable name after For");
    std::string var = current_token.value;
    advance();

    if (current_token.type != TokenType::ASSIGN)
        throw std::runtime_error("Expected '=' after For variable");
    advance();

    auto start = expression();

    if (current_token.type != TokenType::TO)
        throw std::runtime_error("Expected 'To' in For loop");
    advance();

    auto end = expression();

    if (current_token.type != TokenType::NEWLINE)
        throw std::runtime_error("Expected newline after For range");
    advance();

    auto body = parse_block();
    if (current_token.type == TokenType::END) advance();
    return std::make_unique<ForLoopNode>(var, std::move(start), std::move(end), std::move(body));
}

std::unique_ptr<ASTNode> Parser::func_def() {
    advance();
    if (current_token.type != TokenType::IDENTIFIER)
        throw std::runtime_error("Expected function name");
    std::string name = current_token.value;
    advance();

    if (current_token.type != TokenType::LPAREN)
        throw std::runtime_error("Expected '(' after function name");
    advance();

    std::vector<std::string> params;
    while (current_token.type != TokenType::RPAREN && current_token.type != TokenType::END_OF_FILE) {
        if (current_token.type != TokenType::IDENTIFIER)
            throw std::runtime_error("Expected parameter name");
        params.push_back(current_token.value);
        advance();
        if (current_token.type == TokenType::COMMA) advance();
    }
    if (current_token.type != TokenType::RPAREN)
        throw std::runtime_error("Expected ')' after parameters");
    advance();

    if (current_token.type != TokenType::NEWLINE)
        throw std::runtime_error("Expected newline after function definition");
    advance();

    auto body = parse_block();
    if (current_token.type == TokenType::END) advance();
    return std::make_unique<FuncDefNode>(name, std::move(params), std::move(body));
}

std::unique_ptr<ASTNode> Parser::statement() {
    if (current_token.type == TokenType::PRINT) {
        advance();
        return std::make_unique<PrintNode>(logical());
    }
    if (current_token.type == TokenType::IF)    return if_statement();
    if (current_token.type == TokenType::WHILE) return while_statement();
    if (current_token.type == TokenType::FOR)   return for_statement();
    if (current_token.type == TokenType::FUNC)  return func_def();

    if (current_token.type == TokenType::RETURN) {
        advance();
        return std::make_unique<ReturnNode>(logical());
    }

    if (current_token.type == TokenType::IDENTIFIER) {
        std::string name = current_token.value;
        advance();

        if (current_token.type == TokenType::ASSIGN) {
            advance();
            return std::make_unique<AssignmentNode>(name, logical());
        }

        if (current_token.type == TokenType::LBRACKET) {
            advance();
            auto index = logical();
            if (current_token.type != TokenType::RBRACKET)
                throw std::runtime_error("Expected ']'");
            advance();
            if (current_token.type != TokenType::ASSIGN)
                throw std::runtime_error("Expected '=' after array index");
            advance();
            return std::make_unique<ArrayAssignNode>(name, std::move(index), logical());
        }

        if (current_token.type == TokenType::LPAREN) {
            advance();
            std::vector<std::unique_ptr<ASTNode>> args;
            while (current_token.type != TokenType::RPAREN && current_token.type != TokenType::END_OF_FILE) {
                args.push_back(logical());
                if (current_token.type == TokenType::COMMA) advance();
            }
            if (current_token.type != TokenType::RPAREN)
                throw std::runtime_error("Expected ')'");
            advance();
            return std::make_unique<FuncCallNode>(name, std::move(args));
        }

        throw std::runtime_error("Expected '=', '[', or '(' after identifier '" + name + "'");
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
