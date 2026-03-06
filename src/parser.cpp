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
        // Check for string interpolation: contains {expr}
        const std::string& raw = token.value;
        if (raw.find('{') != std::string::npos)
            return parse_interp_string(raw);
        return std::make_unique<StringNode>(raw);
    }
    if (token.type == TokenType::TRUE) {
        advance();
        return std::make_unique<BooleanNode>(true);
    }
    if (token.type == TokenType::FALSE) {
        advance();
        return std::make_unique<BooleanNode>(false);
    }
    if (token.type == TokenType::NULL_TOKEN) {
        advance();
        return std::make_unique<NullNode>();
    }
    if (token.type == TokenType::LBRACE) {
        advance();
        return parse_dict();
    }
    if (token.type == TokenType::NOT) {
        advance();
        return std::make_unique<NotOpNode>(logical());
    }
    if (token.type == TokenType::MINUS) {
        advance();
        // Unary minus: wrap as 0 - factor
        return std::make_unique<BinaryOpNode>(TokenType::MINUS,
            std::make_unique<NumberNode>(0.0), factor());
    }
    if (token.type == TokenType::INPUT) {
        advance();
        if (current_token.type != TokenType::LPAREN)
            throw std::runtime_error("Expected '(' after Input");
        advance();
        std::unique_ptr<ASTNode> prompt;
        if (current_token.type != TokenType::RPAREN)
            prompt = logical();
        else
            prompt = std::make_unique<StringNode>("");
        if (current_token.type != TokenType::RPAREN)
            throw std::runtime_error("Expected ')' after Input prompt");
        advance();
        return std::make_unique<InputNode>(std::move(prompt));
    }
    
    if (token.type == TokenType::READFILE) {
        advance();
        if (current_token.type != TokenType::LPAREN)
            throw std::runtime_error("Expected '(' after ReadFile");
        advance();
        auto path = logical();
        if (current_token.type != TokenType::RPAREN)
            throw std::runtime_error("Expected ')' after ReadFile");
        advance();
        return std::make_unique<ReadFileNode>(std::move(path));
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
                throw std::runtime_error("Expected ']' after index");
            advance();
            // Could be array or dict access — interpreter handles both
            return std::make_unique<DictAccessNode>(token.value, std::move(index));
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

            // Math built-ins (single argument or with extra args)
            if (token.value == "Floor" || token.value == "Ceil" || token.value == "Round" ||
                token.value == "Sqrt"  || token.value == "Abs"  || token.value == "Power" ||
                token.value == "Sin"   || token.value == "Cos"  || token.value == "Tan" ||
                token.value == "Asin"  || token.value == "Acos" || token.value == "Atan" ||
                token.value == "Atan2" || token.value == "Mod"  || token.value == "Min" ||
                token.value == "Max"   || token.value == "Clamp" || token.value == "Lerp" ||
                token.value == "Log"   || token.value == "Log10" || token.value == "Log2" ||
                token.value == "Exp"   || token.value == "Sinh"  || token.value == "Cosh" ||
                token.value == "Tanh"  || token.value == "Asinh" || token.value == "Acosh" ||
                token.value == "Atanh" || token.value == "Deg2Rad" || token.value == "Rad2Deg" ||
                token.value == "Factorial" || token.value == "IsPrime" || token.value == "GCD" ||
                token.value == "LCM"   || token.value == "RandomInt" ||
                // Possibly useful
                token.value == "Sign"    || token.value == "Truncate"  || token.value == "Frac" ||
                token.value == "Hypot"   || token.value == "Cbrt"      || token.value == "CopySign" ||
                token.value == "LogBase" ||
                // Number checks
                token.value == "IsNaN"   || token.value == "IsInf" ||
                token.value == "IsEven"  || token.value == "IsOdd" ||
                // Type checks
                token.value == "IsNull"   || token.value == "IsDict"   ||
                token.value == "IsArray"  || token.value == "IsString" ||
                token.value == "IsNumber" || token.value == "IsBool"   ||
                // Dictionary operations
                token.value == "DictKeys"   || token.value == "DictValues" ||
                token.value == "DictHas"    || token.value == "DictRemove" ||
                token.value == "DictSize"   || token.value == "DictMerge"  ||
                // JSON
                token.value == "JsonParse"  || token.value == "JsonStringify" ||
                // Bitwise
                token.value == "BitAnd"  || token.value == "BitOr"         || token.value == "BitXor" ||
                token.value == "BitNot"  || token.value == "BitShiftLeft"  || token.value == "BitShiftRight" ||
                // Statistics
                token.value == "Sum"     || token.value == "Product"  || token.value == "Mean" ||
                token.value == "Median"  || token.value == "Variance" || token.value == "StdDev" ||
                // Pure math
                token.value == "Gamma"   || token.value == "Beta" ||
                token.value == "Erf"     || token.value == "Erfc" ||
                // TCP Sockets
                token.value == "SocketConnect"     || token.value == "SocketListen" ||
                token.value == "SocketAccept"      || token.value == "SocketSend" ||
                token.value == "SocketReceive"     || token.value == "SocketReceiveLine" ||
                token.value == "SocketClose"       || token.value == "SocketIsValid" ||
                token.value == "SocketSetTimeout"  ||
                // HTTP Server
                token.value == "HttpServerCreate"  || token.value == "HttpServerAccept" ||
                token.value == "HttpServerClose"   || token.value == "HttpConnClose" ||
                token.value == "HttpRequestMethod" || token.value == "HttpRequestPath" ||
                token.value == "HttpRequestBody"   || token.value == "HttpRequestHeader" ||
                token.value == "HttpRequestParam"  ||
                token.value == "HttpRespond"       || token.value == "HttpRespondFile" ||
                // DNS
                token.value == "DnsResolve"        || token.value == "DnsResolveAll" ||
                token.value == "DnsResolveIPv6"    || token.value == "DnsReverse" ||
                // HTTP client
                token.value == "HttpGet"           || token.value == "HttpPost" ||
                token.value == "HttpPut"           || token.value == "HttpDelete" ||
                token.value == "HttpStatusCode"    || token.value == "HttpHeaders" ||
                token.value == "HttpRequest"       || token.value == "HttpRequestStatus" ||
                token.value == "HttpDownload"      || token.value == "HttpGetJson" ||
                token.value == "HttpPostJson"      || token.value == "HttpGetWithTimeout" ||
                token.value == "HttpGetFull"       ||
                // HTTP server
                token.value == "HttpServerCreate"  || token.value == "HttpServerAccept" ||
                token.value == "HttpServerClose"   || token.value == "HttpConnClose" ||
                token.value == "HttpRequestMethod" || token.value == "HttpRequestPath" ||
                token.value == "HttpRequestBody"   || token.value == "HttpRequestHeader" ||
                token.value == "HttpRequestParam"  || token.value == "HttpRequestQuery" ||
                token.value == "HttpRequestIP"     ||
                token.value == "HttpRespond"       || token.value == "HttpRespondFile" ||
                token.value == "HttpRespondJson"   || token.value == "HttpRespondRedirect" ||
                // WebSocket
                token.value == "WsConnect"         || token.value == "WsSend" ||
                token.value == "WsReceive"         || token.value == "WsReceiveLine" ||
                token.value == "WsClose"           || token.value == "WsIsConnected" ||
                // UDP
                token.value == "UdpCreate"         || token.value == "UdpSend" ||
                token.value == "UdpReceive"        || token.value == "UdpReceiveFull" ||
                token.value == "UdpSetTimeout"     || token.value == "UdpClose" ||
                token.value == "UdpBroadcast") {
                auto target = std::move(args[0]);
                std::vector<std::unique_ptr<ASTNode>> rest;
                for (size_t i = 1; i < args.size(); i++) rest.push_back(std::move(args[i]));
                return std::make_unique<StringOpNode>(token.value, std::move(target), std::move(rest));
            }

            // Type conversion
            if (token.value == "ToNumber" || token.value == "ToString") {
                return std::make_unique<StringOpNode>(token.value, std::move(args[0]), std::vector<std::unique_ptr<ASTNode>>{});
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
    std::vector<std::unique_ptr<ASTNode>> defaults;

    while (current_token.type != TokenType::RPAREN && current_token.type != TokenType::END_OF_FILE) {
        if (current_token.type != TokenType::IDENTIFIER)
            throw std::runtime_error("Expected parameter name");
        params.push_back(current_token.value);
        advance();
        if (current_token.type == TokenType::ASSIGN) {
            // Default parameter: Func greet(name = "World")
            advance();
            defaults.push_back(logical());
        } else {
            defaults.push_back(nullptr); // no default
        }
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
    return std::make_unique<FuncDefNode>(name, std::move(params), std::move(defaults), std::move(body));
}

std::unique_ptr<ASTNode> Parser::try_statement() {
    advance(); // consume Try
    if (current_token.type != TokenType::NEWLINE)
        throw std::runtime_error("Expected newline after Try");
    advance();

    auto try_body = parse_block();

    if (current_token.type != TokenType::CATCH)
        throw std::runtime_error("Expected Catch after Try block");
    advance();

    // Optional: Catch(err)
    std::string error_var = "error";
    if (current_token.type == TokenType::LPAREN) {
        advance();
        if (current_token.type != TokenType::IDENTIFIER)
            throw std::runtime_error("Expected variable name in Catch");
        error_var = current_token.value;
        advance();
        if (current_token.type != TokenType::RPAREN)
            throw std::runtime_error("Expected ')' after Catch variable");
        advance();
    }

    if (current_token.type != TokenType::NEWLINE)
        throw std::runtime_error("Expected newline after Catch");
    advance();

    auto catch_body = parse_block();
    if (current_token.type == TokenType::END) advance();
    return std::make_unique<TryCatchNode>(std::move(try_body), error_var, std::move(catch_body));
}

std::unique_ptr<ASTNode> Parser::statement() {
    if (current_token.type == TokenType::PRINT) {
        advance();
        return std::make_unique<PrintNode>(logical());
    }
    
    if (current_token.type == TokenType::WRITEFILE) {
        advance();
        if (current_token.type != TokenType::LPAREN)
            throw std::runtime_error("Expected '(' after WriteFile");
        advance();
        auto path = logical();
        if (current_token.type != TokenType::COMMA)
            throw std::runtime_error("Expected ',' in WriteFile");
        advance();
        auto content = logical();
        if (current_token.type != TokenType::RPAREN)
            throw std::runtime_error("Expected ')' after WriteFile");
        advance();
        return std::make_unique<WriteFileNode>(std::move(path), std::move(content));
    }
    
    if (current_token.type == TokenType::APPENDFILE) {
        advance();
        if (current_token.type != TokenType::LPAREN)
            throw std::runtime_error("Expected '(' after AppendFile");
        advance();
        auto path = logical();
        if (current_token.type != TokenType::COMMA)
            throw std::runtime_error("Expected ',' in AppendFile");
        advance();
        auto content = logical();
        if (current_token.type != TokenType::RPAREN)
            throw std::runtime_error("Expected ')' after AppendFile");
        advance();
        return std::make_unique<AppendFileNode>(std::move(path), std::move(content));
    }
    
    if (current_token.type == TokenType::IF)    return if_statement();
    if (current_token.type == TokenType::WHILE) return while_statement();
    if (current_token.type == TokenType::FOR)   return for_statement();
    if (current_token.type == TokenType::FUNC)  return func_def();
    if (current_token.type == TokenType::TRY)   return try_statement();

    if (current_token.type == TokenType::RETURN) {
        advance();
        return std::make_unique<ReturnNode>(logical());
    }

    if (current_token.type == TokenType::BREAK) {
        advance();
        return std::make_unique<BreakNode>();
    }

    if (current_token.type == TokenType::CONTINUE) {
        advance();
        return std::make_unique<ContinueNode>();
    }

    if (current_token.type == TokenType::IMPORT) {
        advance();
        if (current_token.type == TokenType::STRING) {
            // Import "file.LANGUAGE" — file import
            std::string filepath = current_token.value;
            advance();
            return std::make_unique<ImportNode>(filepath);
        } else if (current_token.type == TokenType::IDENTIFIER) {
            // Import PACKAGENAME — LANGPACK import
            std::string pkg = current_token.value;
            advance();
            return std::make_unique<LangpackImportNode>(pkg);
        }
        throw std::runtime_error("Expected file path or package name after Import");
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
                throw std::runtime_error("Expected '=' after index");
            advance();
            // DictAssignNode handles both arrays and dicts at runtime
            return std::make_unique<DictAssignNode>(name, std::move(index), logical());
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

// ── Dictionary literal parsing ────────────────────────────────────────────
// Called after consuming '{', parses key: value pairs until '}'
std::unique_ptr<ASTNode> Parser::parse_dict() {
    std::vector<std::pair<std::unique_ptr<ASTNode>, std::unique_ptr<ASTNode>>> pairs;
    while (current_token.type != TokenType::RBRACE && current_token.type != TokenType::END_OF_FILE) {
        auto key = logical();
        if (current_token.type != TokenType::COLON)
            throw std::runtime_error("Expected ':' after dictionary key");
        advance();
        auto val = logical();
        pairs.emplace_back(std::move(key), std::move(val));
        if (current_token.type == TokenType::COMMA) advance();
    }
    if (current_token.type != TokenType::RBRACE)
        throw std::runtime_error("Expected '}' after dictionary entries");
    advance();
    return std::make_unique<DictNode>(std::move(pairs));
}

// ── String interpolation parsing ──────────────────────────────────────────
// Parses "Hello {name}, you are {age} years old!"
// into alternating literal/expression segments
std::unique_ptr<ASTNode> Parser::parse_interp_string(const std::string& raw) {
    auto node = std::make_unique<InterpStringNode>();
    size_t i = 0;
    while (i < raw.size()) {
        if (raw[i] == '{') {
            // Check for escaped brace {{
            if (i + 1 < raw.size() && raw[i + 1] == '{') {
                InterpStringNode::Segment seg;
                seg.is_expr = false;
                seg.literal = "{";
                node->segments.push_back(std::move(seg));
                i += 2;
                continue;
            }
            // Find closing brace
            size_t end = raw.find('}', i + 1);
            if (end == std::string::npos)
                throw std::runtime_error("Unterminated '{' in interpolated string");
            std::string expr_src = raw.substr(i + 1, end - i - 1);
            // Parse the inner expression
            Lexer inner_lexer(expr_src + "\n");
            auto inner_tokens = inner_lexer.tokenize();
            Parser inner_parser(inner_tokens);
            auto expr_ast = inner_parser.logical();
            InterpStringNode::Segment seg;
            seg.is_expr = true;
            seg.expr = std::move(expr_ast);
            node->segments.push_back(std::move(seg));
            i = end + 1;
        } else if (raw[i] == '}' && i + 1 < raw.size() && raw[i + 1] == '}') {
            // Escaped }}
            InterpStringNode::Segment seg;
            seg.is_expr = false;
            seg.literal = "}";
            node->segments.push_back(std::move(seg));
            i += 2;
        } else {
            // Literal segment
            std::string lit;
            while (i < raw.size() && raw[i] != '{' && !(raw[i] == '}' && i + 1 < raw.size() && raw[i+1] == '}')) {
                lit += raw[i++];
            }
            if (!lit.empty()) {
                InterpStringNode::Segment seg;
                seg.is_expr = false;
                seg.literal = lit;
                node->segments.push_back(std::move(seg));
            }
        }
    }
    return node;
}
