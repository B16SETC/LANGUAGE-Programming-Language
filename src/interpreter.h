#pragma once
#include "parser.h"
#include <map>
#include <string>

class Interpreter {
public:
    void execute(const std::vector<std::unique_ptr<ASTNode>>& statements);

private:
    std::map<std::string, double> variables;
    
    double evaluate(ASTNode* node);
};
