#pragma once
#include "parser.h"
#include <map>
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>

struct Value {
    enum class Type { NUMBER, STRING, BOOLEAN, ARRAY } type;

    double number = 0;
    std::string string;
    bool boolean = false;
    std::shared_ptr<std::vector<Value>> array;

    Value() : type(Type::NUMBER), number(0) {}
    Value(double n) : type(Type::NUMBER), number(n) {}
    Value(const std::string& s) : type(Type::STRING), string(s) {}
    Value(bool b) : type(Type::BOOLEAN), boolean(b) {}
    Value(std::shared_ptr<std::vector<Value>> a) : type(Type::ARRAY), array(a) {}

    bool is_number()  const { return type == Type::NUMBER; }
    bool is_string()  const { return type == Type::STRING; }
    bool is_boolean() const { return type == Type::BOOLEAN; }
    bool is_array()   const { return type == Type::ARRAY; }

    bool truthy() const {
        if (is_boolean()) return boolean;
        if (is_number())  return number != 0;
        if (is_string())  return !string.empty();
        if (is_array())   return !array->empty();
        return false;
    }

    std::string to_string() const {
        if (is_string())  return string;
        if (is_boolean()) return boolean ? "True" : "False";
        if (is_array()) {
            std::string s = "[";
            for (size_t i = 0; i < array->size(); i++) {
                s += (*array)[i].to_string();
                if (i + 1 < array->size()) s += ", ";
            }
            return s + "]";
        }
        if (number == (int)number) return std::to_string((int)number);
        return std::to_string(number);
    }
};

struct ReturnException {
    Value value;
    ReturnException(Value v) : value(v) {}
};

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
