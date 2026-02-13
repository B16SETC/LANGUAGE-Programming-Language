#pragma once
#include "parser.h"
#include <map>
#include <string>
#include <variant>
#include <stdexcept>

struct ReturnException {
    struct Value {
        enum class Type { NUMBER, STRING } type;
        double number = 0;
        std::string string;
        Value() : type(Type::NUMBER), number(0) {}
        Value(double n) : type(Type::NUMBER), number(n) {}
        Value(const std::string& s) : type(Type::STRING), string(s) {}
        bool is_string() const { return type == Type::STRING; }
        bool is_number() const { return type == Type::NUMBER; }
    };
    Value value;
    ReturnException(Value v) : value(v) {}
};

using Value = ReturnException::Value;

class Interpreter {
public:
    void execute(const std::vector<std::unique_ptr<ASTNode>>& statements);

private:
    std::map<std::string, Value> variables;
    std::map<std::string, FuncDefNode*> functions;

    Value evaluate(ASTNode* node);
    bool evaluate_condition(ASTNode* node);
    void execute_statement(ASTNode* node);
};
