#pragma once
#include "lexer.h"
#include <memory>
#include <vector>

enum class NodeType {
    NUMBER,
    VARIABLE,
    BINARY_OP,
    COMPARISON,
    ASSIGNMENT,
    PRINT,
    IF_STATEMENT,
    WHILE_LOOP
};

struct ASTNode {
    NodeType type;
    virtual ~ASTNode() = default;
};

struct NumberNode : ASTNode {
    double value;
    NumberNode(double val) : value(val) { type = NodeType::NUMBER; }
};

struct VariableNode : ASTNode {
    std::string name;
    VariableNode(const std::string& n) : name(n) { type = NodeType::VARIABLE; }
};

struct BinaryOpNode : ASTNode {
    TokenType op;
    std::unique_ptr<ASTNode> left;
    std::unique_ptr<ASTNode> right;
    BinaryOpNode(TokenType o, std::unique_ptr<ASTNode> l, std::unique_ptr<ASTNode> r)
        : op(o), left(std::move(l)), right(std::move(r)) { type = NodeType::BINARY_OP; }
};

struct ComparisonNode : ASTNode {
    TokenType op;
    std::unique_ptr<ASTNode> left;
    std::unique_ptr<ASTNode> right;
    ComparisonNode(TokenType o, std::unique_ptr<ASTNode> l, std::unique_ptr<ASTNode> r)
        : op(o), left(std::move(l)), right(std::move(r)) { type = NodeType::COMPARISON; }
};

struct AssignmentNode : ASTNode {
    std::string var_name;
    std::unique_ptr<ASTNode> value;
    AssignmentNode(const std::string& name, std::unique_ptr<ASTNode> val)
        : var_name(name), value(std::move(val)) { type = NodeType::ASSIGNMENT; }
};

struct PrintNode : ASTNode {
    std::unique_ptr<ASTNode> expression;
    PrintNode(std::unique_ptr<ASTNode> expr) : expression(std::move(expr)) { type = NodeType::PRINT; }
};

struct IfStatementNode : ASTNode {
    std::unique_ptr<ASTNode> condition;
    std::vector<std::unique_ptr<ASTNode>> body;
    IfStatementNode(std::unique_ptr<ASTNode> cond, std::vector<std::unique_ptr<ASTNode>> b)
        : condition(std::move(cond)), body(std::move(b)) { type = NodeType::IF_STATEMENT; }
};

struct WhileLoopNode : ASTNode {
    std::unique_ptr<ASTNode> condition;
    std::vector<std::unique_ptr<ASTNode>> body;
    WhileLoopNode(std::unique_ptr<ASTNode> cond, std::vector<std::unique_ptr<ASTNode>> b)
        : condition(std::move(cond)), body(std::move(b)) { type = NodeType::WHILE_LOOP; }
};

class Parser {
public:
    Parser(const std::vector<Token>& tokens);
    std::vector<std::unique_ptr<ASTNode>> parse();

private:
    std::vector<Token> tokens;
    size_t pos;
    Token current_token;

    void advance();
    void consume_newlines();
    std::unique_ptr<ASTNode> statement();
    std::unique_ptr<ASTNode> if_statement();
    std::unique_ptr<ASTNode> while_statement();
    std::unique_ptr<ASTNode> comparison();
    std::unique_ptr<ASTNode> expression();
    std::unique_ptr<ASTNode> term();
    std::unique_ptr<ASTNode> factor();
};
