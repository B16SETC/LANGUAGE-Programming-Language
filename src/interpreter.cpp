#include "interpreter.h"
#include <iostream>
#include <stdexcept>

double Interpreter::evaluate(ASTNode* node) {
    switch (node->type) {
        case NodeType::NUMBER: {
            auto* num_node = static_cast<NumberNode*>(node);
            return num_node->value;
        }
        
        case NodeType::VARIABLE: {
            auto* var_node = static_cast<VariableNode*>(node);
            if (variables.find(var_node->name) == variables.end()) {
                throw std::runtime_error("Undefined variable: " + var_node->name);
            }
            return variables[var_node->name];
        }
        
        case NodeType::BINARY_OP: {
            auto* bin_node = static_cast<BinaryOpNode*>(node);
            double left = evaluate(bin_node->left.get());
            double right = evaluate(bin_node->right.get());
            
            switch (bin_node->op) {
                case TokenType::PLUS:
                    return left + right;
                case TokenType::MINUS:
                    return left - right;
                case TokenType::MULTIPLY:
                    return left * right;
                case TokenType::DIVIDE:
                    if (right == 0) {
                        throw std::runtime_error("Division by zero");
                    }
                    return left / right;
                default:
                    throw std::runtime_error("Unknown operator");
            }
        }
        
        default:
            throw std::runtime_error("Invalid node type in expression");
    }
}

bool Interpreter::evaluate_condition(ASTNode* node) {
    if (node->type == NodeType::COMPARISON) {
        auto* cmp_node = static_cast<ComparisonNode*>(node);
        double left = evaluate(cmp_node->left.get());
        double right = evaluate(cmp_node->right.get());
        
        switch (cmp_node->op) {
            case TokenType::EQUAL:
                return left == right;
            case TokenType::NOT_EQUAL:
                return left != right;
            case TokenType::LESS_THAN:
                return left < right;
            case TokenType::GREATER_THAN:
                return left > right;
            case TokenType::LESS_EQUAL:
                return left <= right;
            case TokenType::GREATER_EQUAL:
                return left >= right;
            default:
                throw std::runtime_error("Unknown comparison operator");
        }
    }
    
    double result = evaluate(node);
    return result != 0;
}

void Interpreter::execute_statement(ASTNode* node) {
    switch (node->type) {
        case NodeType::ASSIGNMENT: {
            auto* assign_node = static_cast<AssignmentNode*>(node);
            double value = evaluate(assign_node->value.get());
            variables[assign_node->var_name] = value;
            break;
        }
        
        case NodeType::PRINT: {
            auto* print_node = static_cast<PrintNode*>(node);
            double value = evaluate(print_node->expression.get());
            std::cout << value << std::endl;
            break;
        }
        
        case NodeType::IF_STATEMENT: {
            auto* if_node = static_cast<IfStatementNode*>(node);
            if (evaluate_condition(if_node->condition.get())) {
                for (const auto& stmt : if_node->body) {
                    execute_statement(stmt.get());
                }
            }
            break;
        }
        
        default:
            throw std::runtime_error("Unknown statement type");
    }
}

void Interpreter::execute(const std::vector<std::unique_ptr<ASTNode>>& statements) {
    for (const auto& stmt : statements) {
        execute_statement(stmt.get());
    }
}
