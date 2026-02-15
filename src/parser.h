#pragma once
#include "lexer.h"
#include <memory>
#include <vector>

enum class NodeType {
    NUMBER,
    STRING,
    BOOLEAN,
    ARRAY,
    ARRAY_ACCESS,
    ARRAY_ASSIGN,
    VARIABLE,
    BINARY_OP,
    LOGICAL_OP,
    NOT_OP,
    COMPARISON,
    ASSIGNMENT,
    PRINT,
    IF_STATEMENT,
    WHILE_LOOP,
    FOR_LOOP,
    FUNC_DEF,
    FUNC_CALL,
    RETURN_STATEMENT,
    STRING_OP
};

struct ASTNode {
    NodeType type;
    virtual ~ASTNode() = default;
};

struct NumberNode : ASTNode {
    double value;
    NumberNode(double val) : value(val) { type = NodeType::NUMBER; }
};

struct StringNode : ASTNode {
    std::string value;
    StringNode(const std::string& val) : value(val) { type = NodeType::STRING; }
};

struct BooleanNode : ASTNode {
    bool value;
    BooleanNode(bool val) : value(val) { type = NodeType::BOOLEAN; }
};

struct ArrayNode : ASTNode {
    std::vector<std::unique_ptr<ASTNode>> elements;
    ArrayNode(std::vector<std::unique_ptr<ASTNode>> elems)
        : elements(std::move(elems)) { type = NodeType::ARRAY; }
};

struct ArrayAccessNode : ASTNode {
    std::string name;
    std::unique_ptr<ASTNode> index;
    ArrayAccessNode(const std::string& n, std::unique_ptr<ASTNode> i)
        : name(n), index(std::move(i)) { type = NodeType::ARRAY_ACCESS; }
};

struct ArrayAssignNode : ASTNode {
    std::string name;
    std::unique_ptr<ASTNode> index;
    std::unique_ptr<ASTNode> value;
    ArrayAssignNode(const std::string& n, std::unique_ptr<ASTNode> i, std::unique_ptr<ASTNode> v)
        : name(n), index(std::move(i)), value(std::move(v)) { type = NodeType::ARRAY_ASSIGN; }
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

struct LogicalOpNode : ASTNode {
    TokenType op;
    std::unique_ptr<ASTNode> left;
    std::unique_ptr<ASTNode> right;
    LogicalOpNode(TokenType o, std::unique_ptr<ASTNode> l, std::unique_ptr<ASTNode> r)
        : op(o), left(std::move(l)), right(std::move(r)) { type = NodeType::LOGICAL_OP; }
};

struct NotOpNode : ASTNode {
    std::unique_ptr<ASTNode> operand;
    NotOpNode(std::unique_ptr<ASTNode> op) : operand(std::move(op)) { type = NodeType::NOT_OP; }
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

struct ElifClause {
    std::unique_ptr<ASTNode> condition;
    std::vector<std::unique_ptr<ASTNode>> body;
};

struct IfStatementNode : ASTNode {
    std::unique_ptr<ASTNode> condition;
    std::vector<std::unique_ptr<ASTNode>> body;
    std::vector<ElifClause> elif_clauses;
    std::vector<std::unique_ptr<ASTNode>> else_body;
    IfStatementNode(std::unique_ptr<ASTNode> cond,
                    std::vector<std::unique_ptr<ASTNode>> b,
                    std::vector<ElifClause> elif,
                    std::vector<std::unique_ptr<ASTNode>> eb)
        : condition(std::move(cond)), body(std::move(b)),
          elif_clauses(std::move(elif)), else_body(std::move(eb)) { type = NodeType::IF_STATEMENT; }
};

struct WhileLoopNode : ASTNode {
    std::unique_ptr<ASTNode> condition;
    std::vector<std::unique_ptr<ASTNode>> body;
    WhileLoopNode(std::unique_ptr<ASTNode> cond, std::vector<std::unique_ptr<ASTNode>> b)
        : condition(std::move(cond)), body(std::move(b)) { type = NodeType::WHILE_LOOP; }
};

struct ForLoopNode : ASTNode {
    std::string var;
    std::unique_ptr<ASTNode> start;
    std::unique_ptr<ASTNode> end;
    std::vector<std::unique_ptr<ASTNode>> body;
    ForLoopNode(const std::string& v, std::unique_ptr<ASTNode> s, std::unique_ptr<ASTNode> e,
                std::vector<std::unique_ptr<ASTNode>> b)
        : var(v), start(std::move(s)), end(std::move(e)), body(std::move(b)) { type = NodeType::FOR_LOOP; }
};

struct FuncDefNode : ASTNode {
    std::string name;
    std::vector<std::string> params;
    std::vector<std::unique_ptr<ASTNode>> body;
    FuncDefNode(const std::string& n, std::vector<std::string> p, std::vector<std::unique_ptr<ASTNode>> b)
        : name(n), params(std::move(p)), body(std::move(b)) { type = NodeType::FUNC_DEF; }
};

struct FuncCallNode : ASTNode {
    std::string name;
    std::vector<std::unique_ptr<ASTNode>> args;
    FuncCallNode(const std::string& n, std::vector<std::unique_ptr<ASTNode>> a)
        : name(n), args(std::move(a)) { type = NodeType::FUNC_CALL; }
};

struct ReturnNode : ASTNode {
    std::unique_ptr<ASTNode> value;
    ReturnNode(std::unique_ptr<ASTNode> val) : value(std::move(val)) { type = NodeType::RETURN_STATEMENT; }
};

struct StringOpNode : ASTNode {
    std::string op;
    std::unique_ptr<ASTNode> target;
    std::vector<std::unique_ptr<ASTNode>> args;
    StringOpNode(const std::string& o, std::unique_ptr<ASTNode> t, std::vector<std::unique_ptr<ASTNode>> a)
        : op(o), target(std::move(t)), args(std::move(a)) { type = NodeType::STRING_OP; }
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
    std::vector<std::unique_ptr<ASTNode>> parse_block();
    std::unique_ptr<ASTNode> statement();
    std::unique_ptr<ASTNode> if_statement();
    std::unique_ptr<ASTNode> while_statement();
    std::unique_ptr<ASTNode> for_statement();
    std::unique_ptr<ASTNode> func_def();
    std::unique_ptr<ASTNode> logical();
    std::unique_ptr<ASTNode> comparison();
    std::unique_ptr<ASTNode> expression();
    std::unique_ptr<ASTNode> term();
    std::unique_ptr<ASTNode> factor();
};
