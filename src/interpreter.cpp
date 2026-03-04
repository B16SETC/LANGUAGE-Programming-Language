#include "interpreter.h"
#include "lexer.h"
#include "parser.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <algorithm>
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

        case NodeType::INPUT: {
            auto* inp = static_cast<InputNode*>(node);
            Value prompt = evaluate(inp->prompt.get());
            std::cout << prompt.to_string();
            std::string line;
            std::getline(std::cin, line);
            return Value(line);
        }

        case NodeType::READFILE: {
            auto* rf = static_cast<ReadFileNode*>(node);
            Value path = evaluate(rf->path.get());
            if (!path.is_string()) throw std::runtime_error("ReadFile requires a string path");
            std::ifstream file(path.string);
            if (!file.is_open()) throw std::runtime_error("Cannot open file: " + path.string);
            std::stringstream buf;
            buf << file.rdbuf();
            return Value(buf.str());
        }

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
            
            // Built-in zero-argument functions
            if (call->name == "Random") {
                static bool seeded = false;
                if (!seeded) {
                    std::srand(std::time(nullptr));
                    seeded = true;
                }
                return Value((double)std::rand() / RAND_MAX);
            }
            
            // Built-in statistics functions
            if (call->name == "Mean" || call->name == "Sum") {
                if (call->args.size() != 1) throw std::runtime_error(call->name + " requires 1 argument");
                Value arr = evaluate(call->args[0].get());
                if (!arr.is_array()) throw std::runtime_error(call->name + " requires an array");
                if (arr.array->empty()) throw std::runtime_error(call->name + " of empty array");
                double sum = 0;
                for (const auto& val : *arr.array) sum += val.number;
                if (call->name == "Sum") return Value(sum);
                return Value(sum / arr.array->size());
            }
            
            if (call->name == "Median") {
                if (call->args.size() != 1) throw std::runtime_error("Median requires 1 argument");
                Value arr = evaluate(call->args[0].get());
                if (!arr.is_array()) throw std::runtime_error("Median requires an array");
                if (arr.array->empty()) throw std::runtime_error("Median of empty array");
                
                std::vector<double> numbers;
                for (const auto& val : *arr.array) {
                    numbers.push_back(val.number);
                }
                std::sort(numbers.begin(), numbers.end());
                
                size_t n = numbers.size();
                if (n % 2 == 0) {
                    return Value((numbers[n/2-1] + numbers[n/2]) / 2.0);
                }
                return Value(numbers[n/2]);
            }
            
            if (call->name == "StdDev" || call->name == "Variance") {
                if (call->args.size() != 1) throw std::runtime_error(call->name + " requires 1 argument");
                Value arr = evaluate(call->args[0].get());
                if (!arr.is_array()) throw std::runtime_error(call->name + " requires an array");
                if (arr.array->empty()) throw std::runtime_error(call->name + " of empty array");
                double sum = 0;
                for (const auto& val : *arr.array) sum += val.number;
                double mean = sum / arr.array->size();
                double variance = 0;
                for (const auto& val : *arr.array) {
                    double diff = val.number - mean;
                    variance += diff * diff;
                }
                variance /= arr.array->size();
                if (call->name == "Variance") return Value(variance);
                return Value(std::sqrt(variance));
            }
            
            if (functions.find(call->name) == functions.end()) {
                // Try routing socket functions that end up as FuncCallNode
                static const std::set<std::string> socket_ops = {
                    "SocketConnect", "SocketListen", "SocketAccept",
                    "SocketSend", "SocketReceive", "SocketReceiveLine",
                    "SocketClose", "SocketIsValid", "SocketSetTimeout"
                };
                if (socket_ops.count(call->name)) {
                    // Re-route: evaluate first arg as target, rest as op->args
                    // Build a temporary StringOpNode on the fly
                    if (call->args.empty())
                        throw std::runtime_error(call->name + ": requires at least 1 argument");
                    Value target_val = evaluate(call->args[0].get());
                    struct TempOp : StringOpNode {
                        std::vector<Value> evaluated_args;
                        TempOp(const std::string& name) : StringOpNode(name, nullptr, {}) {}
                    };
                    // Manually handle each socket op inline
                    std::string sop = call->name;
                    Value target_v = evaluate(call->args[0].get());

                    if (sop == "SocketClose") {
                        int handle = (int)target_v.number;
                        if (tcp_sockets.find(handle) == tcp_sockets.end())
                            throw std::runtime_error("SocketClose: invalid socket handle");
                        LANG_CLOSE_SOCKET(tcp_sockets[handle]);
                        tcp_sockets.erase(handle);
                        return Value(0.0);
                    }
                    if (sop == "SocketIsValid") {
                        int handle = (int)target_v.number;
                        return Value(tcp_sockets.find(handle) != tcp_sockets.end());
                    }
                    if (sop == "SocketSend") {
                        int handle = (int)target_v.number;
                        if (tcp_sockets.find(handle) == tcp_sockets.end())
                            throw std::runtime_error("SocketSend: invalid socket handle");
                        Value msg_val = evaluate(call->args[1].get());
                        std::string msg = msg_val.to_string();
                        int sent = send(tcp_sockets[handle], msg.c_str(), (int)msg.size(), 0);
                        if (sent < 0) throw std::runtime_error("SocketSend: send failed");
                        return Value((double)sent);
                    }
                    if (sop == "SocketReceive") {
                        int handle = (int)target_v.number;
                        if (tcp_sockets.find(handle) == tcp_sockets.end())
                            throw std::runtime_error("SocketReceive: invalid socket handle");
                        int buf_size = 4096;
                        if (call->args.size() > 1) buf_size = (int)evaluate(call->args[1].get()).number;
                        std::vector<char> buf(buf_size);
                        int n = recv(tcp_sockets[handle], buf.data(), buf_size - 1, 0);
                        if (n < 0) throw std::runtime_error("SocketReceive: recv failed");
                        if (n == 0) return Value(std::string(""));
                        return Value(std::string(buf.data(), n));
                    }
                    if (sop == "SocketReceiveLine") {
                        int handle = (int)target_v.number;
                        if (tcp_sockets.find(handle) == tcp_sockets.end())
                            throw std::runtime_error("SocketReceiveLine: invalid socket handle");
                        std::string line; char ch;
                        while (true) {
                            int n = recv(tcp_sockets[handle], &ch, 1, 0);
                            if (n <= 0) break;
                            if (ch == '\n') break;
                            if (ch != '\r') line += ch;
                        }
                        return Value(line);
                    }
                    if (sop == "SocketAccept") {
                        int handle = (int)target_v.number;
                        if (tcp_sockets.find(handle) == tcp_sockets.end())
                            throw std::runtime_error("SocketAccept: invalid socket handle");
                        sockaddr_in client_addr{}; socklen_t client_len = sizeof(client_addr);
                        lang_socket_t cfd = accept(tcp_sockets[handle], (sockaddr*)&client_addr, &client_len);
                        if (cfd == LANG_INVALID_SOCKET) throw std::runtime_error("SocketAccept: failed");
                        int ch = next_socket_handle++;
                        tcp_sockets[ch] = cfd;
                        return Value((double)ch);
                    }
                    if (sop == "SocketSetTimeout") {
                        int handle = (int)target_v.number;
                        if (tcp_sockets.find(handle) == tcp_sockets.end())
                            throw std::runtime_error("SocketSetTimeout: invalid socket handle");
                        int ms = (int)evaluate(call->args[1].get()).number;
#ifdef _WIN32
                        DWORD timeout = ms;
                        setsockopt(tcp_sockets[handle], SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
#else
                        struct timeval tv; tv.tv_sec = ms/1000; tv.tv_usec = (ms%1000)*1000;
                        setsockopt(tcp_sockets[handle], SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
#endif
                        return Value(0.0);
                    }
                    if (sop == "SocketConnect") {
                        std::string host = target_v.string;
                        int port = (int)evaluate(call->args[1].get()).number;
                        struct addrinfo hints{}, *res = nullptr;
                        hints.ai_family = AF_INET; hints.ai_socktype = SOCK_STREAM;
                        if (getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &res) != 0 || !res)
                            throw std::runtime_error("SocketConnect: could not resolve: " + host);
                        lang_socket_t fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
                        if (fd == LANG_INVALID_SOCKET) { freeaddrinfo(res); throw std::runtime_error("SocketConnect: socket failed"); }
                        if (connect(fd, res->ai_addr, (int)res->ai_addrlen) < 0) { freeaddrinfo(res); LANG_CLOSE_SOCKET(fd); throw std::runtime_error("SocketConnect: connect failed to " + host); }
                        freeaddrinfo(res);
                        int h = next_socket_handle++; tcp_sockets[h] = fd;
                        return Value((double)h);
                    }
                    if (sop == "SocketListen") {
                        std::string host = target_v.string;
                        int port = (int)evaluate(call->args[1].get()).number;
                        lang_socket_t fd = socket(AF_INET, SOCK_STREAM, 0);
                        if (fd == LANG_INVALID_SOCKET) throw std::runtime_error("SocketListen: socket failed");
                        int opt = 1;
                        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
                        sockaddr_in addr{}; addr.sin_family = AF_INET; addr.sin_port = htons(port);
                        addr.sin_addr.s_addr = (host == "0.0.0.0" || host == "*") ? INADDR_ANY : inet_addr(host.c_str());
                        if (bind(fd, (sockaddr*)&addr, sizeof(addr)) < 0) { LANG_CLOSE_SOCKET(fd); throw std::runtime_error("SocketListen: bind failed on port " + std::to_string(port)); }
                        if (listen(fd, 10) < 0) { LANG_CLOSE_SOCKET(fd); throw std::runtime_error("SocketListen: listen failed"); }
                        int h = next_socket_handle++; tcp_sockets[h] = fd;
                        return Value((double)h);
                    }
                }
                throw std::runtime_error("Undefined function: " + call->name);
            }

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
            // Math built-ins
            if (op->op == "Floor")  return Value(std::floor(target.number));
            if (op->op == "Ceil")   return Value(std::ceil(target.number));
            if (op->op == "Round")  return Value(std::round(target.number));
            if (op->op == "Sqrt") {
                if (target.number < 0) throw std::runtime_error("Sqrt of negative number");
                return Value(std::sqrt(target.number));
            }
            if (op->op == "Abs")    return Value(std::abs(target.number));
            if (op->op == "Power") {
                Value exp = evaluate(op->args[0].get());
                return Value(std::pow(target.number, exp.number));
            }
            
            // Trigonometry (angles in radians)
            if (op->op == "Sin")    return Value(std::sin(target.number));
            if (op->op == "Cos")    return Value(std::cos(target.number));
            if (op->op == "Tan")    return Value(std::tan(target.number));
            if (op->op == "Asin") {
                if (target.number < -1 || target.number > 1)
                    throw std::runtime_error("Asin input must be between -1 and 1");
                return Value(std::asin(target.number));
            }
            if (op->op == "Acos") {
                if (target.number < -1 || target.number > 1)
                    throw std::runtime_error("Acos input must be between -1 and 1");
                return Value(std::acos(target.number));
            }
            if (op->op == "Atan")   return Value(std::atan(target.number));
            if (op->op == "Atan2") {
                Value x = evaluate(op->args[0].get());
                return Value(std::atan2(target.number, x.number));
            }
            
            // Additional math
            if (op->op == "Mod") {
                Value divisor = evaluate(op->args[0].get());
                if (divisor.number == 0) throw std::runtime_error("Modulo by zero");
                return Value(std::fmod(target.number, divisor.number));
            }
            if (op->op == "Min") {
                Value other = evaluate(op->args[0].get());
                return Value(std::min(target.number, other.number));
            }
            if (op->op == "Max") {
                Value other = evaluate(op->args[0].get());
                return Value(std::max(target.number, other.number));
            }
            if (op->op == "Clamp") {
                Value min_val = evaluate(op->args[0].get());
                Value max_val = evaluate(op->args[1].get());
                double result = target.number;
                if (result < min_val.number) result = min_val.number;
                if (result > max_val.number) result = max_val.number;
                return Value(result);
            }
            if (op->op == "Lerp") {
                Value b = evaluate(op->args[0].get());
                Value t = evaluate(op->args[1].get());
                return Value(target.number + (b.number - target.number) * t.number);
            }
            
            // Logarithms and exponentials
            if (op->op == "Log")    return Value(std::log(target.number));
            if (op->op == "Log10")  return Value(std::log10(target.number));
            if (op->op == "Log2")   return Value(std::log2(target.number));
            if (op->op == "Exp")    return Value(std::exp(target.number));
            
            // Hyperbolic trig
            if (op->op == "Sinh")   return Value(std::sinh(target.number));
            if (op->op == "Cosh")   return Value(std::cosh(target.number));
            if (op->op == "Tanh")   return Value(std::tanh(target.number));
            if (op->op == "Asinh")  return Value(std::asinh(target.number));
            if (op->op == "Acosh")  return Value(std::acosh(target.number));
            if (op->op == "Atanh")  return Value(std::atanh(target.number));
            
            // Angle conversion
            if (op->op == "Deg2Rad") return Value(target.number * 3.14159265359 / 180.0);
            if (op->op == "Rad2Deg") return Value(target.number * 180.0 / 3.14159265359);
            
            // Number theory
            if (op->op == "Factorial") {
                int n = (int)target.number;
                if (n < 0) throw std::runtime_error("Factorial of negative number");
                if (n > 170) throw std::runtime_error("Factorial too large");
                double result = 1;
                for (int i = 2; i <= n; i++) result *= i;
                return Value(result);
            }
            if (op->op == "IsPrime") {
                int n = (int)target.number;
                if (n < 2) return Value(0.0);
                if (n == 2) return Value(1.0);
                if (n % 2 == 0) return Value(0.0);
                for (int i = 3; i <= std::sqrt(n); i += 2) {
                    if (n % i == 0) return Value(0.0);
                }
                return Value(1.0);
            }
            if (op->op == "GCD") {
                Value other = evaluate(op->args[0].get());
                int a = std::abs((int)target.number);
                int b = std::abs((int)other.number);
                while (b != 0) {
                    int temp = b;
                    b = a % b;
                    a = temp;
                }
                return Value((double)a);
            }
            if (op->op == "LCM") {
                Value other = evaluate(op->args[0].get());
                int a = std::abs((int)target.number);
                int b = std::abs((int)other.number);
                int gcd_val = a;
                int b_temp = b;
                while (b_temp != 0) {
                    int temp = b_temp;
                    b_temp = gcd_val % b_temp;
                    gcd_val = temp;
                }
                return Value((double)(a / gcd_val * b));
            }
            
            // RandomInt still uses target as the minimum
            if (op->op == "RandomInt") {
                Value max_val = evaluate(op->args[0].get());
                static bool seeded = false;
                if (!seeded) {
                    std::srand(std::time(nullptr));
                    seeded = true;
                }
                int range = (int)max_val.number - (int)target.number + 1;
                return Value((double)((std::rand() % range) + (int)target.number));
            }
            
            // --- Possibly Useful ---
            if (op->op == "Sign") {
                if (target.number < 0) return Value(-1.0);
                if (target.number > 0) return Value(1.0);
                return Value(0.0);
            }
            if (op->op == "Truncate") return Value((double)(int)target.number);
            if (op->op == "Frac")     return Value(target.number - (int)target.number);
            if (op->op == "Hypot") {
                Value other = evaluate(op->args[0].get());
                return Value(std::hypot(target.number, other.number));
            }
            if (op->op == "Cbrt")     return Value(std::cbrt(target.number));
            if (op->op == "CopySign") {
                Value other = evaluate(op->args[0].get());
                return Value(std::copysign(target.number, other.number));
            }
            if (op->op == "LogBase") {
                Value base = evaluate(op->args[0].get());
                return Value(std::log(target.number) / std::log(base.number));
            }

            // --- Number Checks ---
            if (op->op == "IsNaN")   return Value(std::isnan(target.number));
            if (op->op == "IsInf")   return Value(std::isinf(target.number));
            if (op->op == "IsEven")  return Value((int)target.number % 2 == 0);
            if (op->op == "IsOdd")   return Value((int)target.number % 2 != 0);

            // --- Bitwise ---
            if (op->op == "BitAnd") {
                Value other = evaluate(op->args[0].get());
                return Value((double)((int)target.number & (int)other.number));
            }
            if (op->op == "BitOr") {
                Value other = evaluate(op->args[0].get());
                return Value((double)((int)target.number | (int)other.number));
            }
            if (op->op == "BitXor") {
                Value other = evaluate(op->args[0].get());
                return Value((double)((int)target.number ^ (int)other.number));
            }
            if (op->op == "BitNot")        return Value((double)(~(int)target.number));
            if (op->op == "BitShiftLeft") {
                Value n = evaluate(op->args[0].get());
                return Value((double)((int)target.number << (int)n.number));
            }
            if (op->op == "BitShiftRight") {
                Value n = evaluate(op->args[0].get());
                return Value((double)((int)target.number >> (int)n.number));
            }

            // --- Statistics (array-based) ---
            if (op->op == "Sum") {
                if (!target.is_array()) throw std::runtime_error("Sum requires an array");
                double sum = 0;
                for (auto& v : *target.array) sum += v.number;
                return Value(sum);
            }
            if (op->op == "Product") {
                if (!target.is_array()) throw std::runtime_error("Product requires an array");
                double prod = 1;
                for (auto& v : *target.array) prod *= v.number;
                return Value(prod);
            }
            if (op->op == "Mean") {
                if (!target.is_array() || target.array->empty())
                    throw std::runtime_error("Mean requires a non-empty array");
                double sum = 0;
                for (auto& v : *target.array) sum += v.number;
                return Value(sum / target.array->size());
            }
            if (op->op == "Median") {
                if (!target.is_array() || target.array->empty())
                    throw std::runtime_error("Median requires a non-empty array");
                std::vector<double> nums;
                for (auto& v : *target.array) nums.push_back(v.number);
                std::sort(nums.begin(), nums.end());
                size_t n = nums.size();
                if (n % 2 == 0) return Value((nums[n/2 - 1] + nums[n/2]) / 2.0);
                return Value(nums[n/2]);
            }
            if (op->op == "Variance") {
                if (!target.is_array() || target.array->empty())
                    throw std::runtime_error("Variance requires a non-empty array");
                double sum = 0;
                for (auto& v : *target.array) sum += v.number;
                double mean = sum / target.array->size();
                double var = 0;
                for (auto& v : *target.array) var += (v.number - mean) * (v.number - mean);
                return Value(var / target.array->size());
            }
            if (op->op == "StdDev") {
                if (!target.is_array() || target.array->empty())
                    throw std::runtime_error("StdDev requires a non-empty array");
                double sum = 0;
                for (auto& v : *target.array) sum += v.number;
                double mean = sum / target.array->size();
                double var = 0;
                for (auto& v : *target.array) var += (v.number - mean) * (v.number - mean);
                return Value(std::sqrt(var / target.array->size()));
            }

            // --- Pure Mathematics ---
            if (op->op == "Gamma")  return Value(std::tgamma(target.number));
            if (op->op == "Beta") {
                Value other = evaluate(op->args[0].get());
                return Value(std::tgamma(target.number) * std::tgamma(other.number)
                             / std::tgamma(target.number + other.number));
            }
            if (op->op == "Erf")    return Value(std::erf(target.number));
            if (op->op == "Erfc")   return Value(std::erfc(target.number));

            // --- TCP Sockets ---

            // SocketConnect("host", port) → handle
            if (op->op == "SocketConnect") {
                std::string host = target.string;
                Value port_val = evaluate(op->args[0].get());
                int port = (int)port_val.number;

                struct addrinfo hints{}, *res = nullptr;
                hints.ai_family   = AF_INET;
                hints.ai_socktype = SOCK_STREAM;
                if (getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &res) != 0 || !res)
                    throw std::runtime_error("SocketConnect: could not resolve host: " + host);

                lang_socket_t fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
                if (fd == LANG_INVALID_SOCKET) {
                    freeaddrinfo(res);
                    throw std::runtime_error("SocketConnect: failed to create socket");
                }
                if (connect(fd, res->ai_addr, (int)res->ai_addrlen) < 0) {
                    freeaddrinfo(res);
                    LANG_CLOSE_SOCKET(fd);
                    throw std::runtime_error("SocketConnect: failed to connect to " + host + ":" + std::to_string(port));
                }
                freeaddrinfo(res);

                int handle = next_socket_handle++;
                tcp_sockets[handle] = fd;
                return Value((double)handle);
            }

            // SocketListen("host", port) → server handle
            if (op->op == "SocketListen") {
                std::string host = target.string;
                Value port_val = evaluate(op->args[0].get());
                int port = (int)port_val.number;

                lang_socket_t fd = socket(AF_INET, SOCK_STREAM, 0);
                if (fd == LANG_INVALID_SOCKET)
                    throw std::runtime_error("SocketListen: failed to create socket");

                int opt = 1;
                setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));

                sockaddr_in addr{};
                addr.sin_family      = AF_INET;
                addr.sin_port        = htons(port);
                addr.sin_addr.s_addr = host == "0.0.0.0" || host == "*"
                                       ? INADDR_ANY
                                       : inet_addr(host.c_str());

                if (bind(fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
                    LANG_CLOSE_SOCKET(fd);
                    throw std::runtime_error("SocketListen: bind failed on port " + std::to_string(port));
                }
                if (listen(fd, 10) < 0) {
                    LANG_CLOSE_SOCKET(fd);
                    throw std::runtime_error("SocketListen: listen failed");
                }

                int handle = next_socket_handle++;
                tcp_sockets[handle] = fd;
                return Value((double)handle);
            }

            // SocketAccept(serverHandle) → client handle
            if (op->op == "SocketAccept") {
                int handle = (int)target.number;
                if (tcp_sockets.find(handle) == tcp_sockets.end())
                    throw std::runtime_error("SocketAccept: invalid socket handle " + std::to_string(handle));

                sockaddr_in client_addr{};
                socklen_t client_len = sizeof(client_addr);
                lang_socket_t client_fd = accept(tcp_sockets[handle], (sockaddr*)&client_addr, &client_len);
                if (client_fd == LANG_INVALID_SOCKET)
                    throw std::runtime_error("SocketAccept: accept failed");

                int client_handle = next_socket_handle++;
                tcp_sockets[client_handle] = client_fd;
                return Value((double)client_handle);
            }

            // SocketSend(handle, message)
            if (op->op == "SocketSend") {
                int handle = (int)target.number;
                if (tcp_sockets.find(handle) == tcp_sockets.end())
                    throw std::runtime_error("SocketSend: invalid socket handle " + std::to_string(handle));
                Value msg_val = evaluate(op->args[0].get());
                std::string msg = msg_val.to_string();
                int sent = send(tcp_sockets[handle], msg.c_str(), (int)msg.size(), 0);
                if (sent < 0)
                    throw std::runtime_error("SocketSend: send failed");
                return Value((double)sent);
            }

            // SocketReceive(handle) or SocketReceive(handle, bufferSize)
            if (op->op == "SocketReceive") {
                int handle = (int)target.number;
                if (tcp_sockets.find(handle) == tcp_sockets.end())
                    throw std::runtime_error("SocketReceive: invalid socket handle " + std::to_string(handle));

                int buf_size = 4096;
                if (!op->args.empty()) {
                    Value sz = evaluate(op->args[0].get());
                    buf_size = (int)sz.number;
                }

                std::vector<char> buf(buf_size);
                int n = recv(tcp_sockets[handle], buf.data(), buf_size - 1, 0);
                if (n < 0)
                    throw std::runtime_error("SocketReceive: recv failed");
                if (n == 0)
                    return Value(std::string(""));   // connection closed
                return Value(std::string(buf.data(), n));
            }

            // SocketReceiveLine(handle) — receive until \n
            if (op->op == "SocketReceiveLine") {
                int handle = (int)target.number;
                if (tcp_sockets.find(handle) == tcp_sockets.end())
                    throw std::runtime_error("SocketReceiveLine: invalid socket handle");

                std::string line;
                char ch;
                while (true) {
                    int n = recv(tcp_sockets[handle], &ch, 1, 0);
                    if (n <= 0) break;
                    if (ch == '\n') break;
                    if (ch != '\r') line += ch;
                }
                return Value(line);
            }

            // SocketClose(handle)
            if (op->op == "SocketClose") {
                int handle = (int)target.number;
                if (tcp_sockets.find(handle) == tcp_sockets.end())
                    throw std::runtime_error("SocketClose: invalid socket handle " + std::to_string(handle));
                LANG_CLOSE_SOCKET(tcp_sockets[handle]);
                tcp_sockets.erase(handle);
                return Value(0.0);
            }

            // SocketIsValid(handle) — check if handle is open
            if (op->op == "SocketIsValid") {
                int handle = (int)target.number;
                return Value(tcp_sockets.find(handle) != tcp_sockets.end());
            }

            // SocketSetTimeout(handle, milliseconds)
            if (op->op == "SocketSetTimeout") {
                int handle = (int)target.number;
                if (tcp_sockets.find(handle) == tcp_sockets.end())
                    throw std::runtime_error("SocketSetTimeout: invalid socket handle");
                Value ms_val = evaluate(op->args[0].get());
                int ms = (int)ms_val.number;

#ifdef _WIN32
                DWORD timeout = ms;
                setsockopt(tcp_sockets[handle], SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
#else
                struct timeval tv;
                tv.tv_sec  = ms / 1000;
                tv.tv_usec = (ms % 1000) * 1000;
                setsockopt(tcp_sockets[handle], SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
#endif
                return Value(0.0);
            }

            // Type conversion (kept here in order)
            if (op->op == "ToNumber") {
                if (target.is_number()) return target;
                if (target.is_string()) {
                    try { return Value(std::stod(target.string)); }
                    catch (...) { throw std::runtime_error("Cannot convert \"" + target.string + "\" to number"); }
                }
                if (target.is_boolean()) return Value(target.boolean ? 1.0 : 0.0);
                throw std::runtime_error("Cannot convert value to number");
            }
            if (op->op == "ToString") {
                return Value(target.to_string());
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

        case NodeType::WRITEFILE: {
            auto* wf = static_cast<WriteFileNode*>(node);
            Value path = evaluate(wf->path.get());
            Value content = evaluate(wf->content.get());
            if (!path.is_string()) throw std::runtime_error("WriteFile requires a string path");
            std::ofstream file(path.string);
            if (!file.is_open()) throw std::runtime_error("Cannot open file: " + path.string);
            file << content.to_string();
            break;
        }

        case NodeType::APPENDFILE: {
            auto* af = static_cast<AppendFileNode*>(node);
            Value path = evaluate(af->path.get());
            Value content = evaluate(af->content.get());
            if (!path.is_string()) throw std::runtime_error("AppendFile requires a string path");
            std::ofstream file(path.string, std::ios::app);
            if (!file.is_open()) throw std::runtime_error("Cannot open file: " + path.string);
            file << content.to_string();
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
            while (evaluate_condition(while_node->condition.get())) {
                try {
                    for (const auto& stmt : while_node->body)
                        execute_statement(stmt.get());
                } catch (BreakException&) {
                    break; // Exit the loop
                } catch (ContinueException&) {
                    continue; // Skip to next iteration
                }
            }
            break;
        }

        case NodeType::FOR_LOOP: {
            auto* for_node = static_cast<ForLoopNode*>(node);
            double start = evaluate(for_node->start.get()).number;
            double end   = evaluate(for_node->end.get()).number;
            for (double i = start; i <= end; i++) {
                variables[for_node->var] = Value(i);
                try {
                    for (const auto& stmt : for_node->body)
                        execute_statement(stmt.get());
                } catch (BreakException&) {
                    break; // Exit the loop
                } catch (ContinueException&) {
                    continue; // Skip to next iteration
                }
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

        case NodeType::BREAK_STATEMENT: {
            throw BreakException();
        }

        case NodeType::CONTINUE_STATEMENT: {
            throw ContinueException();
        }

        case NodeType::IMPORT_STATEMENT: {
            auto* imp = static_cast<ImportNode*>(node);
            import_file(imp->filepath);
            break;
        }

        case NodeType::TRY_CATCH: {
            auto* tc = static_cast<TryCatchNode*>(node);
            try {
                for (const auto& stmt : tc->try_body)
                    execute_statement(stmt.get());
            } catch (ReturnException&) {
                throw; // let return propagate
            } catch (const std::exception& e) {
                variables[tc->error_var] = Value(std::string(e.what()));
                for (const auto& stmt : tc->catch_body)
                    execute_statement(stmt.get());
            }
            break;
        }

        default:
            throw std::runtime_error("Unknown statement type");
    }
}

void Interpreter::execute(const std::vector<std::unique_ptr<ASTNode>>& statements) {
    for (const auto& stmt : statements)
        execute_statement(stmt.get());
}

void Interpreter::import_file(const std::string& filepath) {
    // Resolve path: if not absolute, treat as relative to current_dir
    std::filesystem::path p(filepath);
    std::string resolved;
    if (p.is_absolute()) {
        resolved = filepath;
    } else {
        std::filesystem::path base = current_dir.empty() ? std::filesystem::current_path() : std::filesystem::path(current_dir);
        resolved = std::filesystem::weakly_canonical(base / p).string();
    }

    // Check for circular imports
    if (imported_files.find(resolved) != imported_files.end()) {
        return; // Already imported, skip
    }

    // Mark as imported
    imported_files.insert(resolved);

    // Read the file
    std::ifstream file(resolved);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file for import: " + resolved);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();

    // Normalize line endings
    std::string normalized;
    for (size_t i = 0; i < source.length(); i++) {
        if (source[i] == '\r' && i + 1 < source.length() && source[i + 1] == '\n') {
            continue;
        }
        normalized += source[i];
    }

    // Lex, parse, and execute the imported file
    Lexer lexer(normalized);
    auto tokens = lexer.tokenize();

    Parser parser(tokens);
    auto ast = parser.parse();

    // Keep the AST alive (functions store raw pointers into it)
    imported_asts.push_back(std::move(ast));

    // Switch current_dir to the imported file's directory so nested imports resolve correctly
    std::string saved_dir = current_dir;
    current_dir = std::filesystem::path(resolved).parent_path().string();

    for (const auto& stmt : imported_asts.back())
        execute_statement(stmt.get());

    // Restore previous directory
    current_dir = saved_dir;
}
