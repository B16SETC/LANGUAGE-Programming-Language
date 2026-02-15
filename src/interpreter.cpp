#include "interpreter.h"
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <cmath>

Value Interpreter::evaluate(ASTNode* node) {
    switch (node->type) {
        case NodeType::NUMBER:
            return Value(static_cast<NumberNode*>(node)->value);

        case NodeType::STRING:
            return Value(static_cast<StringNode*>(node)->value);

        case NodeType::BOOLEAN:
            return Value(static_cast<BooleanNode*>(node)->value);

        case NodeType::ARRAY: {
            auto* arr = static_cast<ArrayNode*>(node);
            auto vec = std::make_shared<std::vector<Value>>();
            for (const auto& elem : arr->elements)
                vec->push_back(evaluate(elem.get()));
            return Value(vec);
        }

        case NodeType::ARRAY_ACCESS: {
            auto* acc = static_cast<ArrayAccessNode*>(node);
            if (variables.find(acc->name) == variables.end())
                throw std::runtime_error("Undefined variable: " + acc->name);
            Value arr = variables[acc->name];
            if (!arr.is_array()) throw std::runtime_error(acc->name + " is not an array");
            int index = (int)evaluate(acc->index.get()).number;
            if (index < 0 || index >= (int)arr.array->size())
                throw std::runtime_error("Array index out of bounds");
            return (*arr.array)[index];
        }

        case NodeType::VARIABLE: {
            auto* var = static_cast<VariableNode*>(node);
            if (variables.find(var->name) == variables.end())
                throw std::runtime_error("Undefined variable: " + var->name);
            return variables[var->name];
        }

        case NodeType::BINARY_OP: {
            auto* bin = static_cast<BinaryOpNode*>(node);
            Value left = evaluate(bin->left.get());
            Value right = evaluate(bin->right.get());

            if (bin->op == TokenType::PLUS) {
                if (left.is_string() || right.is_string())
                    return Value(left.to_string() + right.to_string());
                return Value(left.number + right.number);
            }
            if (!left.is_number() || !right.is_number())
                throw std::runtime_error("Arithmetic requires numbers");
            switch (bin->op) {
                case TokenType::MINUS:    return Value(left.number - right.number);
                case TokenType::MULTIPLY: return Value(left.number * right.number);
                case TokenType::DIVIDE:
                    if (right.number == 0) throw std::runtime_error("Division by zero");
                    return Value(left.number / right.number);
                default: throw std::runtime_error("Unknown operator");
            }
        }

        case NodeType::LOGICAL_OP: {
            auto* log = static_cast<LogicalOpNode*>(node);
            if (log->op == TokenType::AND)
                return Value(evaluate_condition(log->left.get()) && evaluate_condition(log->right.get()));
            if (log->op == TokenType::OR)
                return Value(evaluate_condition(log->left.get()) || evaluate_condition(log->right.get()));
            throw std::runtime_error("Unknown logical operator");
        }

        case NodeType::NOT_OP: {
            auto* not_node = static_cast<NotOpNode*>(node);
            return Value(!evaluate_condition(not_node->operand.get()));
        }

        case NodeType::FUNC_CALL: {
            auto* call = static_cast<FuncCallNode*>(node);
            if (functions.find(call->name) == functions.end())
                throw std::runtime_error("Undefined function: " + call->name);

            FuncDefNode* func = functions[call->name];
            if (call->args.size() != func->params.size())
                throw std::runtime_error("Function '" + call->name + "' expects " +
                    std::to_string(func->params.size()) + " arguments");

            auto saved_vars = variables;
            for (size_t i = 0; i < func->params.size(); i++)
                variables[func->params[i]] = evaluate(call->args[i].get());

            Value result;
            try {
                for (const auto& stmt : func->body)
                    execute_statement(stmt.get());
            } catch (ReturnException& ret) {
                result = ret.value;
            }
            variables = saved_vars;
            return result;
        }

        case NodeType::STRING_OP: {
            auto* op = static_cast<StringOpNode*>(node);
            Value target = evaluate(op->target.get());

            if (op->op == "Length") {
                if (target.is_string()) return Value((double)target.string.length());
                if (target.is_array())  return Value((double)target.array->size());
                throw std::runtime_error("Length requires a string or array");
            }
            if (op->op == "Upper") {
                if (!target.is_string()) throw std::runtime_error("Upper requires a string");
                std::string s = target.string;
                std::transform(s.begin(), s.end(), s.begin(), ::toupper);
                return Value(s);
            }
            if (op->op == "Lower") {
                if (!target.is_string()) throw std::runtime_error("Lower requires a string");
                std::string s = target.string;
                std::transform(s.begin(), s.end(), s.begin(), ::tolower);
                return Value(s);
            }
            if (op->op == "Contains") {
                if (!target.is_string()) throw std::runtime_error("Contains requires a string");
                Value search = evaluate(op->args[0].get());
                return Value(target.string.find(search.to_string()) != std::string::npos ? 1.0 : 0.0);
            }
            if (op->op == "Substring") {
                if (!target.is_string()) throw std::runtime_error("Substring requires a string");
                int start = (int)evaluate(op->args[0].get()).number;
                int len   = (int)evaluate(op->args[1].get()).number;
                return Value(target.string.substr(start, len));
            }
            if (op->op == "Push") {
                if (!target.is_array()) throw std::runtime_error("Push requires an array");
                Value val = evaluate(op->args[0].get());
                target.array->push_back(val);
                return target;
            }
            if (op->op == "Pop") {
                if (!target.is_array()) throw std::runtime_error("Pop requires an array");
                if (target.array->empty()) throw std::runtime_error("Cannot pop from empty array");
                Value last = target.array->back();
                target.array->pop_back();
                return last;
            }
            throw std::runtime_error("Unknown operation: " + op->op);
        }

        default:
            throw std::runtime_error("Invalid node type in expression");
    }
}

bool Interpreter::evaluate_condition(ASTNode* node) {
    if (node->type == NodeType::COMPARISON) {
        auto* cmp = static_cast<ComparisonNode*>(node);
        Value left = evaluate(cmp->left.get());
        Value right = evaluate(cmp->right.get());

        if (left.is_string() && right.is_string()) {
            switch (cmp->op) {
                case TokenType::EQUAL:     return left.string == right.string;
                case TokenType::NOT_EQUAL: return left.string != right.string;
                default: throw std::runtime_error("Only == and != supported for string comparison");
            }
        }
        if (!left.is_number() || !right.is_number())
            throw std::runtime_error("Comparison requires matching types");

        switch (cmp->op) {
            case TokenType::EQUAL:         return left.number == right.number;
            case TokenType::NOT_EQUAL:     return left.number != right.number;
            case TokenType::LESS_THAN:     return left.number <  right.number;
            case TokenType::GREATER_THAN:  return left.number >  right.number;
            case TokenType::LESS_EQUAL:    return left.number <= right.number;
            case TokenType::GREATER_EQUAL: return left.number >= right.number;
            default: throw std::runtime_error("Unknown comparison operator");
        }
    }
    return evaluate(node).truthy();
}

void Interpreter::execute_statement(ASTNode* node) {
    switch (node->type) {
        case NodeType::ASSIGNMENT: {
            auto* assign = static_cast<AssignmentNode*>(node);
            variables[assign->var_name] = evaluate(assign->value.get());
            break;
        }

        case NodeType::ARRAY_ASSIGN: {
            auto* assign = static_cast<ArrayAssignNode*>(node);
            if (variables.find(assign->name) == variables.end())
                throw std::runtime_error("Undefined variable: " + assign->name);
            Value& arr = variables[assign->name];
            if (!arr.is_array()) throw std::runtime_error(assign->name + " is not an array");
            int index = (int)evaluate(assign->index.get()).number;
            if (index < 0 || index >= (int)arr.array->size())
                throw std::runtime_error("Array index out of bounds");
            (*arr.array)[index] = evaluate(assign->value.get());
            break;
        }

        case NodeType::PRINT: {
            auto* print = static_cast<PrintNode*>(node);
            Value val = evaluate(print->expression.get());
            std::cout << val.to_string() << std::endl;
            break;
        }

        case NodeType::IF_STATEMENT: {
            auto* if_node = static_cast<IfStatementNode*>(node);
            if (evaluate_condition(if_node->condition.get())) {
                for (const auto& stmt : if_node->body)
                    execute_statement(stmt.get());
            } else {
                bool elif_matched = false;
                for (auto& clause : if_node->elif_clauses) {
                    if (evaluate_condition(clause.condition.get())) {
                        for (const auto& stmt : clause.body)
                            execute_statement(stmt.get());
                        elif_matched = true;
                        break;
                    }
                }
                if (!elif_matched) {
                    for (const auto& stmt : if_node->else_body)
                        execute_statement(stmt.get());
                }
            }
            break;
        }

        case NodeType::WHILE_LOOP: {
            auto* while_node = static_cast<WhileLoopNode*>(node);
            while (evaluate_condition(while_node->condition.get()))
                for (const auto& stmt : while_node->body)
                    execute_statement(stmt.get());
            break;
        }

        case NodeType::FOR_LOOP: {
            auto* for_node = static_cast<ForLoopNode*>(node);
            double start = evaluate(for_node->start.get()).number;
            double end   = evaluate(for_node->end.get()).number;
            for (double i = start; i <= end; i++) {
                variables[for_node->var] = Value(i);
                for (const auto& stmt : for_node->body)
                    execute_statement(stmt.get());
            }
            break;
        }

        case NodeType::FUNC_DEF: {
            auto* func = static_cast<FuncDefNode*>(node);
            functions[func->name] = func;
            break;
        }

        case NodeType::FUNC_CALL: {
            auto* call = static_cast<FuncCallNode*>(node);

            if (call->name == "Push") {
                if (call->args.size() != 2) throw std::runtime_error("Push requires 2 arguments");
                auto* var = dynamic_cast<VariableNode*>(call->args[0].get());
                if (!var) throw std::runtime_error("Push first argument must be a variable");
                if (variables.find(var->name) == variables.end())
                    throw std::runtime_error("Undefined variable: " + var->name);
                Value& arr = variables[var->name];
                if (!arr.is_array()) throw std::runtime_error(var->name + " is not an array");
                arr.array->push_back(evaluate(call->args[1].get()));
                break;
            }

            if (call->name == "Pop") {
                if (call->args.size() != 1) throw std::runtime_error("Pop requires 1 argument");
                auto* var = dynamic_cast<VariableNode*>(call->args[0].get());
                if (!var) throw std::runtime_error("Pop first argument must be a variable");
                if (variables.find(var->name) == variables.end())
                    throw std::runtime_error("Undefined variable: " + var->name);
                Value& arr = variables[var->name];
                if (!arr.is_array()) throw std::runtime_error(var->name + " is not an array");
                if (arr.array->empty()) throw std::runtime_error("Cannot Pop from empty array");
                arr.array->pop_back();
                break;
            }

            evaluate(node);
            break;
        }

        case NodeType::RETURN_STATEMENT: {
            auto* ret = static_cast<ReturnNode*>(node);
            throw ReturnException(evaluate(ret->value.get()));
        }

        default:
            throw std::runtime_error("Unknown statement type");
    }
}

void Interpreter::execute(const std::vector<std::unique_ptr<ASTNode>>& statements) {
    for (const auto& stmt : statements)
        execute_statement(stmt.get());
}
