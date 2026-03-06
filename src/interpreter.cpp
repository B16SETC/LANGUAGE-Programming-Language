#include "interpreter.h"
#include "lexer.h"
#include "parser.h"
#include "language_api.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <algorithm>
#include <stdexcept>
#include <algorithm>
#include <cmath>
#include <functional>
#ifndef _WIN32
  #include <dlfcn.h>
#endif

// ── HTTP helpers (libcurl) ────────────────────────────────────────────────
#if defined(USE_CURL)
static size_t curl_write_cb(char* ptr, size_t size, size_t nmemb, std::string* data) {
    data->append(ptr, size * nmemb);
    return size * nmemb;
}

static size_t curl_header_cb(char* ptr, size_t size, size_t nmemb, std::string* data) {
    data->append(ptr, size * nmemb);
    return size * nmemb;
}

// Perform a generic curl request, returns {body, status_code, headers}
struct CurlResult {
    std::string body;
    std::string headers;
    long status_code = 0;
};

static CurlResult curl_perform(const std::string& url,
                                const std::string& method,
                                const std::string& body,
                                const std::string& extra_headers,
                                int timeout_ms = 30000) {
    CurlResult result;
    CURL* curl = curl_easy_init();
    if (!curl) throw std::runtime_error("HttpRequest: failed to init curl");

    struct curl_slist* headers = nullptr;
    if (!extra_headers.empty()) {
        // extra_headers is newline-separated "Key: Value" pairs
        std::istringstream ss(extra_headers);
        std::string line;
        while (std::getline(ss, line)) {
            if (!line.empty())
                headers = curl_slist_append(headers, line.c_str());
        }
    }

    curl_easy_setopt(curl, CURLOPT_URL,            url.c_str());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION,  1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS,      (long)timeout_ms);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,   curl_write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA,       &result.body);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION,  curl_header_cb);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA,      &result.headers);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER,  1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST,  2L);

    if (headers)
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    if (method == "POST" || method == "PUT" || method == "PATCH") {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS,    body.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)body.size());
    } else if (method == "DELETE") {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
    } else if (method == "HEAD") {
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
    }
    // GET is default

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        if (headers) curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        throw std::runtime_error("HttpRequest failed: " + std::string(curl_easy_strerror(res)));
    }

    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &result.status_code);
    if (headers) curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return result;
}
#endif // USE_CURL

// ── WebSocket callback (libwebsockets) ───────────────────────────────────
#if defined(USE_WEBSOCKETS)
int Interpreter::ws_callback(lws* wsi, enum lws_callback_reasons reason,
                              void* user, void* in, size_t len) {
    WsContext* ctx = static_cast<WsContext*>(user);
    if (!ctx) return 0;

    switch (reason) {
        case LWS_CALLBACK_CLIENT_ESTABLISHED:
            ctx->connected = true;
            break;
        case LWS_CALLBACK_CLIENT_RECEIVE:
            ctx->recv_buf.append(static_cast<char*>(in), len);
            break;
        case LWS_CALLBACK_CLIENT_WRITEABLE:
            if (!ctx->send_buf.empty()) {
                std::vector<unsigned char> buf(LWS_PRE + ctx->send_buf.size());
                memcpy(buf.data() + LWS_PRE, ctx->send_buf.data(), ctx->send_buf.size());
                lws_write(wsi, buf.data() + LWS_PRE, ctx->send_buf.size(), LWS_WRITE_TEXT);
                ctx->send_buf.clear();
            }
            break;
        case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
        case LWS_CALLBACK_CLOSED:
            ctx->closed = true;
            break;
        default:
            break;
    }
    return 0;
}
#endif // USE_WEBSOCKETS

// ── HTTP Server helpers ───────────────────────────────────────────────────
std::string Interpreter::url_decode(const std::string& s) {
    std::string result;
    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] == '%' && i + 2 < s.size()) {
            int val = std::stoi(s.substr(i + 1, 2), nullptr, 16);
            result += (char)val;
            i += 2;
        } else if (s[i] == '+') {
            result += ' ';
        } else {
            result += s[i];
        }
    }
    return result;
}

std::map<std::string, std::string> Interpreter::parse_query_string(const std::string& query) {
    std::map<std::string, std::string> params;
    std::istringstream ss(query);
    std::string pair;
    while (std::getline(ss, pair, '&')) {
        size_t eq = pair.find('=');
        if (eq != std::string::npos) {
            std::string key = url_decode(pair.substr(0, eq));
            std::string val = url_decode(pair.substr(eq + 1));
            params[key] = val;
        } else if (!pair.empty()) {
            params[url_decode(pair)] = "";
        }
    }
    return params;
}

Interpreter::HttpRequest Interpreter::parse_http_request(const std::string& raw) {
    HttpRequest req;
    std::istringstream ss(raw);
    std::string line;

    // Request line: METHOD /path?query HTTP/1.1
    if (std::getline(ss, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        std::istringstream rl(line);
        std::string path_full;
        rl >> req.method >> path_full;

        size_t q = path_full.find('?');
        if (q != std::string::npos) {
            req.path  = url_decode(path_full.substr(0, q));
            req.query = path_full.substr(q + 1);
            req.params = parse_query_string(req.query);
        } else {
            req.path = url_decode(path_full);
        }
    }

    // Headers
    while (std::getline(ss, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) break; // blank line = end of headers
        size_t colon = line.find(':');
        if (colon != std::string::npos) {
            std::string key = line.substr(0, colon);
            std::string val = line.substr(colon + 1);
            // trim leading space from value
            if (!val.empty() && val[0] == ' ') val = val.substr(1);
            // lowercase key for easy lookup
            std::transform(key.begin(), key.end(), key.begin(), ::tolower);
            req.headers[key] = val;
        }
    }

    // Body (if Content-Length present)
    if (req.headers.count("content-length")) {
        int len = std::stoi(req.headers["content-length"]);
        req.body.resize(len);
        ss.read(&req.body[0], len);
    }

    return req;
}

Value Interpreter::evaluate(ASTNode* node) {
    switch (node->type) {
        case NodeType::NUMBER:
            return Value(static_cast<NumberNode*>(node)->value);

        case NodeType::STRING:
            return Value(static_cast<StringNode*>(node)->value);

        case NodeType::BOOLEAN:
            return Value(static_cast<BooleanNode*>(node)->value);

        case NodeType::NULL_LITERAL:
            return Value::make_null();

        case NodeType::INTERP_STRING: {
            auto* isn = static_cast<InterpStringNode*>(node);
            std::string result;
            for (auto& seg : isn->segments) {
                if (seg.is_expr)
                    result += evaluate(seg.expr.get()).to_string();
                else
                    result += seg.literal;
            }
            return Value(result);
        }

        case NodeType::DICT: {
            auto* dn = static_cast<DictNode*>(node);
            auto d = std::make_shared<Dict>();
            for (auto& [k, v] : dn->pairs) {
                Value key = evaluate(k.get());
                Value val = evaluate(v.get());
                (*d)[key.to_string()] = val;
            }
            return Value(d);
        }

        case NodeType::DICT_ACCESS: {
            auto* da = static_cast<DictAccessNode*>(node);
            if (variables.find(da->name) == variables.end())
                throw std::runtime_error("Undefined variable: " + da->name);
            Value container = variables[da->name];
            Value key = evaluate(da->key.get());
            if (container.is_dict()) {
                std::string k = key.to_string();
                if (container.dict->find(k) == container.dict->end())
                    return Value::make_null();
                return (*container.dict)[k];
            }
            if (container.is_array()) {
                int index = (int)key.number;
                if (index < 0 || index >= (int)container.array->size())
                    throw std::runtime_error("Array index out of bounds");
                return (*container.array)[index];
            }
            throw std::runtime_error(da->name + " is not an array or dictionary");
        }

        case NodeType::LANGPACK_IMPORT: {
            auto* lp = static_cast<LangpackImportNode*>(node);
            // LANGPACK loading — Phase 1: look for <name>.langpack in known dirs
            std::vector<std::string> search_paths = {
                current_dir + "/" + lp->package_name + ".langpack",
#ifndef _WIN32
                std::string(getenv("HOME") ? getenv("HOME") : "") + "/.language/packages/" + lp->package_name + ".langpack",
                "/usr/local/lib/language/packages/" + lp->package_name + ".langpack",
#else
                std::string(getenv("APPDATA") ? getenv("APPDATA") : "") + "/LANGUAGE/packages/" + lp->package_name + ".langpack",
#endif
            };
            std::string found_path;
            for (auto& p : search_paths) {
                std::ifstream f(p);
                if (f.good()) { found_path = p; break; }
            }
            if (found_path.empty())
                throw std::runtime_error("LANGPACK not found: " + lp->package_name +
                    "\nSearched: " + search_paths[0] + "\nInstall with: LANGUAGE --install " + lp->package_name);
#ifdef _WIN32
            HMODULE handle = LoadLibraryA(found_path.c_str());
            if (!handle)
                throw std::runtime_error("Failed to load LANGPACK: " + lp->package_name);
            typedef void (*RegisterFn)(void*);
            RegisterFn reg = (RegisterFn)GetProcAddress(handle, "langpack_register");
#else
            void* handle = dlopen(found_path.c_str(), RTLD_LAZY);
            if (!handle)
                throw std::runtime_error("Failed to load LANGPACK: " + lp->package_name + " — " + dlerror());
            typedef void (*RegisterFn)(void*);
            RegisterFn reg = (RegisterFn)dlsym(handle, "langpack_register");
#endif
            if (!reg)
                throw std::runtime_error("LANGPACK missing langpack_register: " + lp->package_name);
            reg(reinterpret_cast<LangInterp*>(this));
            return Value(0.0);
        }

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
            
            // Check LANGPACK native functions first
            if (native_functions.find(call->name) != native_functions.end()) {
                std::vector<Value> args;
                for (auto& arg : call->args)
                    args.push_back(evaluate(arg.get()));
                return native_functions[call->name](args);
            }

            if (functions.find(call->name) == functions.end()) {
                // Try routing socket functions that end up as FuncCallNode
                static const std::set<std::string> socket_ops = {
                    "SocketConnect", "SocketListen", "SocketAccept",
                    "SocketSend", "SocketReceive", "SocketReceiveLine",
                    "SocketClose", "SocketIsValid", "SocketSetTimeout",
                    "DnsResolve", "DnsResolveAll", "DnsResolveIPv6", "DnsReverse",
                    "HttpGet", "HttpPost", "HttpPut", "HttpDelete",
                    "HttpStatusCode", "HttpHeaders", "HttpRequest", "HttpRequestStatus",
                    "HttpDownload", "HttpGetJson", "HttpPostJson",
                    "HttpGetWithTimeout", "HttpGetFull",
                    "HttpServerCreate", "HttpServerAccept", "HttpServerClose", "HttpConnClose",
                    "HttpRequestMethod", "HttpRequestPath", "HttpRequestBody",
                    "HttpRequestHeader", "HttpRequestParam", "HttpRequestQuery", "HttpRequestIP",
                    "HttpRespond", "HttpRespondFile", "HttpRespondJson", "HttpRespondRedirect",
                    "WsConnect", "WsSend", "WsReceive", "WsReceiveLine", "WsClose", "WsIsConnected",
                    "UdpCreate", "UdpSend", "UdpReceive", "UdpReceiveFull",
                    "UdpSetTimeout", "UdpClose", "UdpBroadcast",
                    "IsNull", "IsDict", "IsArray", "IsString", "IsNumber", "IsBool",
                    "DictKeys", "DictValues", "DictHas", "DictRemove", "DictSize", "DictMerge",
                    "JsonParse", "JsonStringify"
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
                    // All other socket/http/dns/ws ops: build a real StringOpNode and evaluate it
                    {
                        std::vector<std::unique_ptr<ASTNode>> arg_nodes;
                        for (size_t i = 0; i < call->args.size(); i++)
                            arg_nodes.push_back(std::move(call->args[i]));
                        // target is args[0], rest are remaining args
                        std::unique_ptr<ASTNode> target_node = std::move(arg_nodes[0]);
                        std::vector<std::unique_ptr<ASTNode>> rest_nodes;
                        for (size_t i = 1; i < arg_nodes.size(); i++)
                            rest_nodes.push_back(std::move(arg_nodes[i]));
                        auto snode = std::make_unique<StringOpNode>(call->name,
                                         std::move(target_node), std::move(rest_nodes));
                        return evaluate(snode.get());
                    }
                }
                throw std::runtime_error("Undefined function: " + call->name);
            }

            FuncDefNode* func = functions[call->name];

            // Validate argument count considering defaults
            size_t required = 0;
            for (size_t i = 0; i < func->defaults.size(); i++)
                if (!func->defaults[i]) required++;

            if (call->args.size() < required || call->args.size() > func->params.size())
                throw std::runtime_error("Function '" + call->name + "' expects " +
                    std::to_string(required) + "-" + std::to_string(func->params.size()) + " arguments, got " +
                    std::to_string(call->args.size()));

            auto saved_vars = variables;
            for (size_t i = 0; i < func->params.size(); i++) {
                if (i < call->args.size())
                    variables[func->params[i]] = evaluate(call->args[i].get());
                else if (func->defaults[i])
                    variables[func->params[i]] = evaluate(func->defaults[i].get());
                else
                    throw std::runtime_error("Missing argument: " + func->params[i]);
            }

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

            // ── Type checks ───────────────────────────────────────────────
            if (op->op == "IsNull")   return Value(target.is_null());
            if (op->op == "IsDict")   return Value(target.is_dict());
            if (op->op == "IsArray")  return Value(target.is_array());
            if (op->op == "IsString") return Value(target.is_string());
            if (op->op == "IsNumber") return Value(target.is_number());
            if (op->op == "IsBool")   return Value(target.is_boolean());

            // ── Dictionary operations ─────────────────────────────────────
            // DictKeys(dict) → array of keys
            if (op->op == "DictKeys") {
                if (!target.is_dict()) throw std::runtime_error("DictKeys requires a dictionary");
                auto arr = std::make_shared<std::vector<Value>>();
                for (auto& [k, v] : *target.dict) arr->push_back(Value(k));
                return Value(arr);
            }
            // DictValues(dict) → array of values
            if (op->op == "DictValues") {
                if (!target.is_dict()) throw std::runtime_error("DictValues requires a dictionary");
                auto arr = std::make_shared<std::vector<Value>>();
                for (auto& [k, v] : *target.dict) arr->push_back(v);
                return Value(arr);
            }
            // DictHas(dict, key) → boolean
            if (op->op == "DictHas") {
                if (!target.is_dict()) throw std::runtime_error("DictHas requires a dictionary");
                std::string key = evaluate(op->args[0].get()).to_string();
                return Value(target.dict->find(key) != target.dict->end());
            }
            // DictRemove(dict, key) — removes key in place
            if (op->op == "DictRemove") {
                if (!target.is_dict()) throw std::runtime_error("DictRemove requires a dictionary");
                std::string key = evaluate(op->args[0].get()).to_string();
                target.dict->erase(key);
                return Value(0.0);
            }
            // DictSize(dict) → number of keys
            if (op->op == "DictSize") {
                if (!target.is_dict()) throw std::runtime_error("DictSize requires a dictionary");
                return Value((double)target.dict->size());
            }
            // DictMerge(dict1, dict2) → new merged dict (dict2 wins on conflict)
            if (op->op == "DictMerge") {
                if (!target.is_dict()) throw std::runtime_error("DictMerge requires a dictionary");
                Value other = evaluate(op->args[0].get());
                if (!other.is_dict()) throw std::runtime_error("DictMerge second argument must be a dictionary");
                auto merged = std::make_shared<Dict>(*target.dict);
                for (auto& [k, v] : *other.dict) (*merged)[k] = v;
                return Value(merged);
            }

            // ── JSON ──────────────────────────────────────────────────────
            // JsonStringify(value) → JSON string
            if (op->op == "JsonStringify") {
                std::function<std::string(const Value&)> to_json = [&](const Value& v) -> std::string {
                    if (v.is_null())    return "null";
                    if (v.is_boolean()) return v.boolean ? "true" : "false";
                    if (v.is_number()) {
                        if (v.number == (int)v.number) return std::to_string((int)v.number);
                        return std::to_string(v.number);
                    }
                    if (v.is_string()) {
                        std::string s = "\"";
                        for (char c : v.string) {
                            if      (c == '"')  s += "\\\"";
                            else if (c == '\\') s += "\\\\";
                            else if (c == '\n') s += "\\n";
                            else if (c == '\r') s += "\\r";
                            else if (c == '\t') s += "\\t";
                            else s += c;
                        }
                        return s + "\"";
                    }
                    if (v.is_array()) {
                        std::string s = "[";
                        for (size_t i = 0; i < v.array->size(); i++) {
                            s += to_json((*v.array)[i]);
                            if (i + 1 < v.array->size()) s += ",";
                        }
                        return s + "]";
                    }
                    if (v.is_dict()) {
                        std::string s = "{";
                        bool first = true;
                        for (auto& [k, val] : *v.dict) {
                            if (!first) s += ",";
                            s += "\"" + k + "\":" + to_json(val);
                            first = false;
                        }
                        return s + "}";
                    }
                    return "null";
                };
                return Value(to_json(target));
            }

            // JsonParse(string) → value (number, string, bool, null, array, dict)
            if (op->op == "JsonParse") {
                if (!target.is_string()) throw std::runtime_error("JsonParse requires a string");
                const std::string& json = target.string;
                size_t pos = 0;

                std::function<Value(void)> parse_json = [&]() -> Value {
                    // Skip whitespace
                    while (pos < json.size() && std::isspace(json[pos])) pos++;
                    if (pos >= json.size()) throw std::runtime_error("JsonParse: unexpected end of input");

                    char c = json[pos];

                    // null
                    if (c == 'n' && json.substr(pos, 4) == "null")  { pos += 4; return Value::make_null(); }
                    // true
                    if (c == 't' && json.substr(pos, 4) == "true")  { pos += 4; return Value(true); }
                    // false
                    if (c == 'f' && json.substr(pos, 5) == "false") { pos += 5; return Value(false); }

                    // string
                    if (c == '"') {
                        pos++; // skip "
                        std::string s;
                        while (pos < json.size() && json[pos] != '"') {
                            if (json[pos] == '\\' && pos + 1 < json.size()) {
                                pos++;
                                switch (json[pos]) {
                                    case '"':  s += '"';  break;
                                    case '\\': s += '\\'; break;
                                    case 'n':  s += '\n'; break;
                                    case 'r':  s += '\r'; break;
                                    case 't':  s += '\t'; break;
                                    default:   s += json[pos]; break;
                                }
                            } else {
                                s += json[pos];
                            }
                            pos++;
                        }
                        if (pos < json.size()) pos++; // skip closing "
                        return Value(s);
                    }

                    // number
                    if (c == '-' || std::isdigit(c)) {
                        size_t start = pos;
                        if (json[pos] == '-') pos++;
                        while (pos < json.size() && std::isdigit(json[pos])) pos++;
                        if (pos < json.size() && json[pos] == '.') {
                            pos++;
                            while (pos < json.size() && std::isdigit(json[pos])) pos++;
                        }
                        if (pos < json.size() && (json[pos] == 'e' || json[pos] == 'E')) {
                            pos++;
                            if (pos < json.size() && (json[pos] == '+' || json[pos] == '-')) pos++;
                            while (pos < json.size() && std::isdigit(json[pos])) pos++;
                        }
                        return Value(std::stod(json.substr(start, pos - start)));
                    }

                    // array
                    if (c == '[') {
                        pos++;
                        auto arr = std::make_shared<std::vector<Value>>();
                        while (pos < json.size()) {
                            while (pos < json.size() && std::isspace(json[pos])) pos++;
                            if (json[pos] == ']') { pos++; break; }
                            arr->push_back(parse_json());
                            while (pos < json.size() && std::isspace(json[pos])) pos++;
                            if (pos < json.size() && json[pos] == ',') pos++;
                        }
                        return Value(arr);
                    }

                    // object
                    if (c == '{') {
                        pos++;
                        auto d = std::make_shared<Dict>();
                        while (pos < json.size()) {
                            while (pos < json.size() && std::isspace(json[pos])) pos++;
                            if (json[pos] == '}') { pos++; break; }
                            Value key = parse_json(); // should be string
                            while (pos < json.size() && std::isspace(json[pos])) pos++;
                            if (pos < json.size() && json[pos] == ':') pos++;
                            Value val = parse_json();
                            (*d)[key.to_string()] = val;
                            while (pos < json.size() && std::isspace(json[pos])) pos++;
                            if (pos < json.size() && json[pos] == ',') pos++;
                        }
                        return Value(d);
                    }

                    throw std::runtime_error("JsonParse: unexpected character '" + std::string(1, c) + "' at position " + std::to_string(pos));
                };

                return parse_json();
            }
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

            // ── DNS ──────────────────────────────────────────────────────────────
            // DnsResolve("hostname") → "ip.addr.string"
            if (op->op == "DnsResolve") {
                std::string hostname = target.string;
                struct addrinfo hints{}, *res = nullptr;
                hints.ai_family   = AF_INET;
                hints.ai_socktype = SOCK_STREAM;
                if (getaddrinfo(hostname.c_str(), nullptr, &hints, &res) != 0 || !res)
                    throw std::runtime_error("DnsResolve: failed to resolve: " + hostname);
                char ip[INET_ADDRSTRLEN] = {};
                auto* addr = (struct sockaddr_in*)res->ai_addr;
                inet_ntop(AF_INET, &addr->sin_addr, ip, sizeof(ip));
                freeaddrinfo(res);
                return Value(std::string(ip));
            }

            // DnsResolveAll("hostname") → array of IP strings
            if (op->op == "DnsResolveAll") {
                std::string hostname = target.string;
                struct addrinfo hints{}, *res = nullptr, *cur = nullptr;
                hints.ai_family   = AF_INET;
                hints.ai_socktype = SOCK_STREAM;
                if (getaddrinfo(hostname.c_str(), nullptr, &hints, &res) != 0 || !res)
                    throw std::runtime_error("DnsResolveAll: failed to resolve: " + hostname);
                auto arr = std::make_shared<std::vector<Value>>();
                for (cur = res; cur != nullptr; cur = cur->ai_next) {
                    char ip[INET_ADDRSTRLEN] = {};
                    auto* addr = (struct sockaddr_in*)cur->ai_addr;
                    inet_ntop(AF_INET, &addr->sin_addr, ip, sizeof(ip));
                    arr->push_back(Value(std::string(ip)));
                }
                freeaddrinfo(res);
                return Value(arr);
            }

            // ── HTTP (requires libcurl — compile with -DUSE_CURL) ─────────────
#if defined(USE_CURL)
            // HttpGet(url) → body string
            if (op->op == "HttpGet") {
                auto r = curl_perform(target.string, "GET", "", "", 30000);
                return Value(r.body);
            }

            // HttpPost(url, body) → body string
            if (op->op == "HttpPost") {
                Value body_val = evaluate(op->args[0].get());
                auto r = curl_perform(target.string, "POST", body_val.to_string(), "", 30000);
                return Value(r.body);
            }

            // HttpPut(url, body) → body string
            if (op->op == "HttpPut") {
                Value body_val = evaluate(op->args[0].get());
                auto r = curl_perform(target.string, "PUT", body_val.to_string(), "", 30000);
                return Value(r.body);
            }

            // HttpDelete(url) → body string
            if (op->op == "HttpDelete") {
                auto r = curl_perform(target.string, "DELETE", "", "", 30000);
                return Value(r.body);
            }

            // HttpStatusCode(url) → number
            if (op->op == "HttpStatusCode") {
                auto r = curl_perform(target.string, "GET", "", "", 30000);
                return Value((double)r.status_code);
            }

            // HttpHeaders(url) → headers string
            if (op->op == "HttpHeaders") {
                auto r = curl_perform(target.string, "HEAD", "", "", 30000);
                return Value(r.headers);
            }

            // HttpRequest(url, method, body, headers) → body string
            // Most flexible — specify everything
            if (op->op == "HttpRequest") {
                std::string method  = op->args.size() > 0 ? evaluate(op->args[0].get()).to_string() : "GET";
                std::string body    = op->args.size() > 1 ? evaluate(op->args[1].get()).to_string() : "";
                std::string headers = op->args.size() > 2 ? evaluate(op->args[2].get()).to_string() : "";
                int timeout         = op->args.size() > 3 ? (int)evaluate(op->args[3].get()).number  : 30000;
                auto r = curl_perform(target.string, method, body, headers, timeout);
                return Value(r.body);
            }

            // HttpRequestFull(url, method, body, headers) → returns status code
            // Same as HttpRequest but returns status code instead of body
            if (op->op == "HttpRequestStatus") {
                std::string method  = op->args.size() > 0 ? evaluate(op->args[0].get()).to_string() : "GET";
                std::string body    = op->args.size() > 1 ? evaluate(op->args[1].get()).to_string() : "";
                std::string headers = op->args.size() > 2 ? evaluate(op->args[2].get()).to_string() : "";
                auto r = curl_perform(target.string, method, body, headers, 30000);
                return Value((double)r.status_code);
            }

            // HttpDownload(url, filepath) → bytes written
            if (op->op == "HttpDownload") {
                std::string filepath = evaluate(op->args[0].get()).to_string();
                FILE* f = fopen(filepath.c_str(), "wb");
                if (!f) throw std::runtime_error("HttpDownload: cannot open file: " + filepath);
                auto file_write_cb = [](char* ptr, size_t size, size_t nmemb, FILE* fp) -> size_t {
                    return fwrite(ptr, size, nmemb, fp);
                };
                CURL* curl = curl_easy_init();
                curl_easy_setopt(curl, CURLOPT_URL,            target.string.c_str());
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,  +file_write_cb);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA,      f);
                curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
                curl_easy_setopt(curl, CURLOPT_TIMEOUT,        120L);
                CURLcode res = curl_easy_perform(curl);
                double downloaded = 0;
                curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD, &downloaded);
                curl_easy_cleanup(curl);
                fclose(f);
                if (res != CURLE_OK)
                    throw std::runtime_error("HttpDownload failed: " + std::string(curl_easy_strerror(res)));
                return Value(downloaded);
            }

            // HttpGetJson(url) → parsed dict/array (GET + JsonParse)
            if (op->op == "HttpGetJson") {
                auto r = curl_perform(target.string, "GET", "", "", 30000);
                // reuse JsonParse logic
                Value str_val(r.body);
                // build a fake StringOpNode eval call
                size_t pos = 0;
                const std::string& json = r.body;
                std::function<Value(void)> parse_json = [&]() -> Value {
                    while (pos < json.size() && std::isspace(json[pos])) pos++;
                    if (pos >= json.size()) throw std::runtime_error("HttpGetJson: empty response");
                    char c = json[pos];
                    if (c == 'n' && json.substr(pos,4)=="null")  { pos+=4; return Value::make_null(); }
                    if (c == 't' && json.substr(pos,4)=="true")  { pos+=4; return Value(true); }
                    if (c == 'f' && json.substr(pos,5)=="false") { pos+=5; return Value(false); }
                    if (c == '"') {
                        pos++; std::string s;
                        while (pos < json.size() && json[pos] != '"') {
                            if (json[pos]=='\\' && pos+1<json.size()) {
                                pos++;
                                switch(json[pos]) { case '"': s+='"'; break; case '\\': s+='\\'; break;
                                    case 'n': s+='\n'; break; case 'r': s+='\r'; break; case 't': s+='\t'; break;
                                    default: s+=json[pos]; }
                            } else s+=json[pos];
                            pos++;
                        }
                        if (pos<json.size()) pos++;
                        return Value(s);
                    }
                    if (c=='-'||std::isdigit(c)) {
                        size_t start=pos; if(json[pos]=='-') pos++;
                        while(pos<json.size()&&std::isdigit(json[pos])) pos++;
                        if(pos<json.size()&&json[pos]=='.'){pos++;while(pos<json.size()&&std::isdigit(json[pos]))pos++;}
                        return Value(std::stod(json.substr(start,pos-start)));
                    }
                    if (c=='[') {
                        pos++; auto arr=std::make_shared<std::vector<Value>>();
                        while(pos<json.size()){while(pos<json.size()&&std::isspace(json[pos]))pos++;
                            if(json[pos]==']'){pos++;break;}
                            arr->push_back(parse_json());
                            while(pos<json.size()&&std::isspace(json[pos]))pos++;
                            if(pos<json.size()&&json[pos]==',')pos++;}
                        return Value(arr);
                    }
                    if (c=='{') {
                        pos++; auto d=std::make_shared<Dict>();
                        while(pos<json.size()){while(pos<json.size()&&std::isspace(json[pos]))pos++;
                            if(json[pos]=='}'){pos++;break;}
                            Value key=parse_json();
                            while(pos<json.size()&&std::isspace(json[pos]))pos++;
                            if(pos<json.size()&&json[pos]==':')pos++;
                            (*d)[key.to_string()]=parse_json();
                            while(pos<json.size()&&std::isspace(json[pos]))pos++;
                            if(pos<json.size()&&json[pos]==',')pos++;}
                        return Value(d);
                    }
                    throw std::runtime_error("HttpGetJson: invalid JSON");
                };
                return parse_json();
            }

            // HttpPostJson(url, dict) → response body (auto stringify + content-type)
            if (op->op == "HttpPostJson") {
                Value data = evaluate(op->args[0].get());
                // Use JsonStringify logic inline via the existing STRING_OP path
                // Build JSON string from value
                std::function<std::string(const Value&)> to_json = [&](const Value& v) -> std::string {
                    if (v.is_null())    return "null";
                    if (v.is_boolean()) return v.boolean ? "true" : "false";
                    if (v.is_number())  return v.number==(int)v.number ? std::to_string((int)v.number) : std::to_string(v.number);
                    if (v.is_string()) {
                        std::string s="\"";
                        for(char c:v.string){if(c=='"')s+="\\\"";else if(c=='\\')s+="\\\\";else if(c=='\n')s+="\\n";else s+=c;}
                        return s+"\"";
                    }
                    if (v.is_array()) {
                        std::string s="[";
                        for(size_t i=0;i<v.array->size();i++){s+=to_json((*v.array)[i]);if(i+1<v.array->size())s+=",";}
                        return s+"]";
                    }
                    if (v.is_dict()) {
                        std::string s="{"; bool first=true;
                        for(auto&[k,val]:*v.dict){if(!first)s+=",";s+="\""+k+"\":"+to_json(val);first=false;}
                        return s+"}";
                    }
                    return "null";
                };
                std::string json_body = to_json(data);
                auto r = curl_perform(target.string, "POST", json_body, "Content-Type: application/json", 30000);
                return Value(r.body);
            }

            // HttpGetWithTimeout(url, ms) → body
            if (op->op == "HttpGetWithTimeout") {
                int timeout = (int)evaluate(op->args[0].get()).number;
                auto r = curl_perform(target.string, "GET", "", "", timeout);
                return Value(r.body);
            }

            // HttpGetFull(url) → dict with {body, status, headers}
            if (op->op == "HttpGetFull") {
                auto r = curl_perform(target.string, "GET", "", "", 30000);
                auto d = std::make_shared<Dict>();
                (*d)["body"]    = Value(r.body);
                (*d)["status"]  = Value((double)r.status_code);
                (*d)["headers"] = Value(r.headers);
                return Value(d);
            }
#else
            if (op->op == "HttpGet"    || op->op == "HttpPost"    || op->op == "HttpPut" ||
                op->op == "HttpDelete" || op->op == "HttpStatusCode" || op->op == "HttpHeaders" ||
                op->op == "HttpRequest" || op->op == "HttpRequestStatus") {
                throw std::runtime_error(op->op + ": HTTP support requires libcurl. "
                    "Recompile with -DUSE_CURL and link -lcurl.");
            }
#endif // USE_CURL

            // ── WebSocket (requires libwebsockets — compile with -DUSE_WEBSOCKETS) ──
#if defined(USE_WEBSOCKETS)
            // WsConnect("ws://host/path") → handle
            if (op->op == "WsConnect") {
                std::string url = target.string;

                // Parse ws:// or wss:// URL
                bool use_ssl = url.substr(0, 6) == "wss://";
                std::string rest = url.substr(use_ssl ? 6 : 5); // strip ws(s)://
                std::string host, path;
                int port = use_ssl ? 443 : 80;

                size_t slash = rest.find('/');
                if (slash == std::string::npos) { host = rest; path = "/"; }
                else { host = rest.substr(0, slash); path = rest.substr(slash); }

                size_t colon = host.find(':');
                if (colon != std::string::npos) {
                    port = std::stoi(host.substr(colon + 1));
                    host = host.substr(0, colon);
                }

                WsContext* wctx = new WsContext();

                lws_protocols protocols[] = {
                    { "default", ws_callback, sizeof(WsContext*), 4096, 0, wctx, 0 },
                    { nullptr, nullptr, 0, 0, 0, nullptr, 0 }
                };

                lws_context_creation_info info{};
                info.port      = CONTEXT_PORT_NO_LISTEN;
                info.protocols = protocols;
                info.options   = use_ssl ? LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT : 0;

                wctx->ctx = lws_create_context(&info);
                if (!wctx->ctx) { delete wctx; throw std::runtime_error("WsConnect: failed to create context"); }

                lws_client_connect_info cci{};
                cci.context    = wctx->ctx;
                cci.address    = host.c_str();
                cci.port       = port;
                cci.path       = path.c_str();
                cci.host       = host.c_str();
                cci.origin     = host.c_str();
                cci.protocol   = protocols[0].name;
                cci.ssl_connection = use_ssl ? LCCSCF_USE_SSL : 0;
                cci.userdata   = wctx;

                wctx->wsi = lws_client_connect_via_info(&cci);
                if (!wctx->wsi) {
                    lws_context_destroy(wctx->ctx);
                    delete wctx;
                    throw std::runtime_error("WsConnect: failed to connect to " + url);
                }

                // Wait for connection
                for (int i = 0; i < 100 && !wctx->connected && !wctx->closed; i++)
                    lws_service(wctx->ctx, 10);

                if (!wctx->connected) {
                    lws_context_destroy(wctx->ctx);
                    delete wctx;
                    throw std::runtime_error("WsConnect: connection timed out: " + url);
                }

                int handle = next_ws_handle++;
                ws_sockets[handle] = wctx;
                return Value((double)handle);
            }

            // WsSend(handle, message)
            if (op->op == "WsSend") {
                int handle = (int)target.number;
                if (ws_sockets.find(handle) == ws_sockets.end())
                    throw std::runtime_error("WsSend: invalid handle");
                WsContext* wctx = ws_sockets[handle];
                Value msg_val = evaluate(op->args[0].get());
                wctx->send_buf = msg_val.to_string();
                lws_callback_on_writable(wctx->wsi);
                lws_service(wctx->ctx, 50);
                return Value(0.0);
            }

            // WsReceive(handle) → string (waits up to 2s)
            if (op->op == "WsReceive") {
                int handle = (int)target.number;
                if (ws_sockets.find(handle) == ws_sockets.end())
                    throw std::runtime_error("WsReceive: invalid handle");
                WsContext* wctx = ws_sockets[handle];
                int timeout_ms = op->args.empty() ? 2000 : (int)evaluate(op->args[0].get()).number;
                int elapsed = 0;
                while (wctx->recv_buf.empty() && !wctx->closed && elapsed < timeout_ms) {
                    lws_service(wctx->ctx, 50);
                    elapsed += 50;
                }
                std::string data = wctx->recv_buf;
                wctx->recv_buf.clear();
                return Value(data);
            }

            // WsReceiveLine(handle) → string (waits for \n)
            if (op->op == "WsReceiveLine") {
                int handle = (int)target.number;
                if (ws_sockets.find(handle) == ws_sockets.end())
                    throw std::runtime_error("WsReceiveLine: invalid handle");
                WsContext* wctx = ws_sockets[handle];
                int elapsed = 0;
                while (!wctx->closed && elapsed < 5000) {
                    lws_service(wctx->ctx, 50);
                    elapsed += 50;
                    if (wctx->recv_buf.find('\n') != std::string::npos) break;
                }
                size_t nl = wctx->recv_buf.find('\n');
                std::string line;
                if (nl != std::string::npos) {
                    line = wctx->recv_buf.substr(0, nl);
                    if (!line.empty() && line.back() == '\r') line.pop_back();
                    wctx->recv_buf = wctx->recv_buf.substr(nl + 1);
                } else {
                    line = wctx->recv_buf;
                    wctx->recv_buf.clear();
                }
                return Value(line);
            }

            // WsClose(handle)
            if (op->op == "WsClose") {
                int handle = (int)target.number;
                if (ws_sockets.find(handle) == ws_sockets.end())
                    throw std::runtime_error("WsClose: invalid handle");
                WsContext* wctx = ws_sockets[handle];
                lws_context_destroy(wctx->ctx);
                delete wctx;
                ws_sockets.erase(handle);
                return Value(0.0);
            }

            // WsIsConnected(handle) → boolean
            if (op->op == "WsIsConnected") {
                int handle = (int)target.number;
                if (ws_sockets.find(handle) == ws_sockets.end()) return Value(false);
                WsContext* wctx = ws_sockets[handle];
                return Value(wctx->connected && !wctx->closed);
            }
#else
            if (op->op == "WsConnect"  || op->op == "WsSend"    ||
                op->op == "WsReceive"  || op->op == "WsClose"   ||
                op->op == "WsIsConnected") {
                throw std::runtime_error(op->op + ": WebSocket support requires libwebsockets. "
                    "Recompile with -DUSE_WEBSOCKETS and link -lwebsockets.");
            }
#endif // USE_WEBSOCKETS

            // ── HTTP Server ───────────────────────────────────────────────────────

            // HttpServerCreate("0.0.0.0", 8080) → server handle
            if (op->op == "HttpServerCreate") {
                std::string host = target.string;
                Value port_val = evaluate(op->args[0].get());
                int port = (int)port_val.number;

                lang_socket_t fd = socket(AF_INET, SOCK_STREAM, 0);
                if (fd == LANG_INVALID_SOCKET)
                    throw std::runtime_error("HttpServerCreate: failed to create socket");

                int opt = 1;
                setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));

                sockaddr_in addr{};
                addr.sin_family      = AF_INET;
                addr.sin_port        = htons(port);
                addr.sin_addr.s_addr = (host == "0.0.0.0" || host == "*")
                                       ? INADDR_ANY : inet_addr(host.c_str());

                if (bind(fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
                    LANG_CLOSE_SOCKET(fd);
                    throw std::runtime_error("HttpServerCreate: bind failed on port " + std::to_string(port));
                }
                if (listen(fd, 32) < 0) {
                    LANG_CLOSE_SOCKET(fd);
                    throw std::runtime_error("HttpServerCreate: listen failed");
                }

                auto* state = new HttpServerState();
                state->server_fd = fd;
                int handle = next_http_handle++;
                http_servers[handle] = state;
                return Value((double)handle);
            }

            // HttpServerAccept(serverHandle) → conn handle
            // Blocks until a request comes in, returns a connection handle
            if (op->op == "HttpServerAccept") {
                int handle = (int)target.number;
                if (http_servers.find(handle) == http_servers.end())
                    throw std::runtime_error("HttpServerAccept: invalid server handle");

                lang_socket_t server_fd = http_servers[handle]->server_fd;
                sockaddr_in client_addr{};
                socklen_t client_len = sizeof(client_addr);
                lang_socket_t client_fd = accept(server_fd, (sockaddr*)&client_addr, &client_len);
                if (client_fd == LANG_INVALID_SOCKET)
                    throw std::runtime_error("HttpServerAccept: accept failed");

                // Capture client IP
                char client_ip[INET_ADDRSTRLEN] = {};
                inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));

                // Read the full request
                std::string raw;
                char buf[4096];
                while (true) {
                    int n = recv(client_fd, buf, sizeof(buf) - 1, 0);
                    if (n <= 0) break;
                    buf[n] = '\0';
                    raw += buf;
                    // Stop when we have headers + body
                    if (raw.find("\r\n\r\n") != std::string::npos) {
                        // Check if we have full body based on content-length
                        size_t header_end = raw.find("\r\n\r\n") + 4;
                        std::string lower_raw = raw;
                        std::transform(lower_raw.begin(), lower_raw.end(), lower_raw.begin(), ::tolower);
                        size_t cl_pos = lower_raw.find("content-length:");
                        if (cl_pos == std::string::npos) break; // no body
                        size_t cl_end = raw.find("\r\n", cl_pos);
                        int content_length = std::stoi(raw.substr(cl_pos + 15, cl_end - cl_pos - 15));
                        if ((int)(raw.size() - header_end) >= content_length) break;
                    }
                }

                auto* conn = new HttpServerConn();
                conn->client_fd = client_fd;
                conn->request   = parse_http_request(raw);
                conn->request.headers["x-client-ip"] = std::string(client_ip);
                conn->responded = false;

                int conn_handle = next_http_handle++;
                http_conns[conn_handle] = conn;
                return Value((double)conn_handle);
            }

            // HttpRequestMethod(connHandle) → string e.g. "GET"
            if (op->op == "HttpRequestMethod") {
                int handle = (int)target.number;
                if (http_conns.find(handle) == http_conns.end())
                    throw std::runtime_error("HttpRequestMethod: invalid connection handle");
                return Value(http_conns[handle]->request.method);
            }

            // HttpRequestPath(connHandle) → string e.g. "/api/data"
            if (op->op == "HttpRequestPath") {
                int handle = (int)target.number;
                if (http_conns.find(handle) == http_conns.end())
                    throw std::runtime_error("HttpRequestPath: invalid connection handle");
                return Value(http_conns[handle]->request.path);
            }

            // HttpRequestBody(connHandle) → string
            if (op->op == "HttpRequestBody") {
                int handle = (int)target.number;
                if (http_conns.find(handle) == http_conns.end())
                    throw std::runtime_error("HttpRequestBody: invalid connection handle");
                return Value(http_conns[handle]->request.body);
            }

            // HttpRequestHeader(connHandle, "header-name") → string
            if (op->op == "HttpRequestHeader") {
                int handle = (int)target.number;
                if (http_conns.find(handle) == http_conns.end())
                    throw std::runtime_error("HttpRequestHeader: invalid connection handle");
                Value header_name = evaluate(op->args[0].get());
                std::string key = header_name.to_string();
                std::transform(key.begin(), key.end(), key.begin(), ::tolower);
                auto& hdrs = http_conns[handle]->request.headers;
                if (hdrs.count(key)) return Value(hdrs.at(key));
                return Value(std::string(""));
            }

            // HttpRequestParam(connHandle, "key") → string (from query string)
            if (op->op == "HttpRequestParam") {
                int handle = (int)target.number;
                if (http_conns.find(handle) == http_conns.end())
                    throw std::runtime_error("HttpRequestParam: invalid connection handle");
                Value key_val = evaluate(op->args[0].get());
                std::string key = key_val.to_string();
                auto& params = http_conns[handle]->request.params;
                if (params.count(key)) return Value(params.at(key));
                return Value(std::string(""));
            }

            // HttpRespond(connHandle, statusCode, body)
            // HttpRespond(connHandle, statusCode, body, contentType)
            if (op->op == "HttpRespond") {
                int handle = (int)target.number;
                if (http_conns.find(handle) == http_conns.end())
                    throw std::runtime_error("HttpRespond: invalid connection handle");
                HttpServerConn* conn = http_conns[handle];

                Value status_val = evaluate(op->args[0].get());
                Value body_val   = evaluate(op->args[1].get());
                std::string content_type = "text/plain";
                if (op->args.size() > 2)
                    content_type = evaluate(op->args[2].get()).to_string();

                std::string status_text = "OK";
                int status_code = (int)status_val.number;
                if      (status_code == 200) status_text = "OK";
                else if (status_code == 201) status_text = "Created";
                else if (status_code == 204) status_text = "No Content";
                else if (status_code == 301) status_text = "Moved Permanently";
                else if (status_code == 302) status_text = "Found";
                else if (status_code == 400) status_text = "Bad Request";
                else if (status_code == 401) status_text = "Unauthorized";
                else if (status_code == 403) status_text = "Forbidden";
                else if (status_code == 404) status_text = "Not Found";
                else if (status_code == 405) status_text = "Method Not Allowed";
                else if (status_code == 500) status_text = "Internal Server Error";

                std::string body = body_val.to_string();
                std::string response =
                    "HTTP/1.1 " + std::to_string(status_code) + " " + status_text + "\r\n"
                    "Content-Type: " + content_type + "; charset=utf-8\r\n"
                    "Content-Length: " + std::to_string(body.size()) + "\r\n"
                    "Access-Control-Allow-Origin: *\r\n"
                    "Connection: close\r\n"
                    "\r\n" + body;

                send(conn->client_fd, response.c_str(), (int)response.size(), 0);
                conn->responded = true;
                LANG_CLOSE_SOCKET(conn->client_fd);
                conn->client_fd = LANG_INVALID_SOCKET;
                return Value(0.0);
            }

            // HttpRespondFile(connHandle, statusCode, filepath)
            // Serve a file with auto content-type detection
            if (op->op == "HttpRespondFile") {
                int handle = (int)target.number;
                if (http_conns.find(handle) == http_conns.end())
                    throw std::runtime_error("HttpRespondFile: invalid connection handle");
                HttpServerConn* conn = http_conns[handle];

                Value status_val = evaluate(op->args[0].get());
                Value path_val   = evaluate(op->args[1].get());
                std::string filepath = path_val.to_string();

                std::ifstream file(filepath, std::ios::binary);
                if (!file.is_open())
                    throw std::runtime_error("HttpRespondFile: cannot open file: " + filepath);
                std::string body((std::istreambuf_iterator<char>(file)),
                                  std::istreambuf_iterator<char>());

                // Detect content type from extension
                std::string ct = "application/octet-stream";
                if (filepath.size() > 5 && filepath.substr(filepath.size()-5) == ".html") ct = "text/html";
                else if (filepath.size() > 4 && filepath.substr(filepath.size()-4) == ".css") ct = "text/css";
                else if (filepath.size() > 3 && filepath.substr(filepath.size()-3) == ".js")  ct = "application/javascript";
                else if (filepath.size() > 5 && filepath.substr(filepath.size()-5) == ".json") ct = "application/json";
                else if (filepath.size() > 4 && filepath.substr(filepath.size()-4) == ".png") ct = "image/png";
                else if (filepath.size() > 4 && filepath.substr(filepath.size()-4) == ".jpg") ct = "image/jpeg";
                else if (filepath.size() > 4 && filepath.substr(filepath.size()-4) == ".svg") ct = "image/svg+xml";
                else if (filepath.size() > 4 && filepath.substr(filepath.size()-4) == ".ico") ct = "image/x-icon";
                else if (filepath.size() > 4 && filepath.substr(filepath.size()-4) == ".txt") ct = "text/plain";

                std::string response =
                    "HTTP/1.1 " + std::to_string((int)status_val.number) + " OK\r\n"
                    "Content-Type: " + ct + "\r\n"
                    "Content-Length: " + std::to_string(body.size()) + "\r\n"
                    "Access-Control-Allow-Origin: *\r\n"
                    "Connection: close\r\n"
                    "\r\n" + body;

                send(conn->client_fd, response.c_str(), (int)response.size(), 0);
                conn->responded = true;
                LANG_CLOSE_SOCKET(conn->client_fd);
                conn->client_fd = LANG_INVALID_SOCKET;
                return Value(0.0);
            }

            // HttpConnClose(connHandle) — close without responding (e.g. after error)
            if (op->op == "HttpConnClose") {
                int handle = (int)target.number;
                if (http_conns.find(handle) == http_conns.end())
                    throw std::runtime_error("HttpConnClose: invalid connection handle");
                HttpServerConn* conn = http_conns[handle];
                if (conn->client_fd != LANG_INVALID_SOCKET)
                    LANG_CLOSE_SOCKET(conn->client_fd);
                delete conn;
                http_conns.erase(handle);
                return Value(0.0);
            }

            // HttpRespondJson(connHandle, statusCode, dict) — auto stringify
            if (op->op == "HttpRespondJson") {
                int handle = (int)target.number;
                if (http_conns.find(handle) == http_conns.end())
                    throw std::runtime_error("HttpRespondJson: invalid connection handle");
                Value status_val = evaluate(op->args[0].get());
                Value data_val   = evaluate(op->args[1].get());
                std::function<std::string(const Value&)> to_json = [&](const Value& v) -> std::string {
                    if (v.is_null())    return "null";
                    if (v.is_boolean()) return v.boolean ? "true" : "false";
                    if (v.is_number())  return v.number==(int)v.number ? std::to_string((int)v.number) : std::to_string(v.number);
                    if (v.is_string()) {
                        std::string s="\"";
                        for(char c:v.string){if(c=='"')s+="\\\"";else if(c=='\\')s+="\\\\";else if(c=='\n')s+="\\n";else s+=c;}
                        return s+"\"";
                    }
                    if (v.is_array()) {
                        std::string s="[";
                        for(size_t i=0;i<v.array->size();i++){s+=to_json((*v.array)[i]);if(i+1<v.array->size())s+=",";}
                        return s+"]";
                    }
                    if (v.is_dict()) {
                        std::string s="{"; bool first=true;
                        for(auto&[k,val]:*v.dict){if(!first)s+=",";s+="\""+k+"\":"+to_json(val);first=false;}
                        return s+"}";
                    }
                    return "null";
                };
                std::string body = to_json(data_val);
                HttpServerConn* conn = http_conns[handle];
                std::string response =
                    "HTTP/1.1 " + std::to_string((int)status_val.number) + " OK\r\n"
                    "Content-Type: application/json; charset=utf-8\r\n"
                    "Content-Length: " + std::to_string(body.size()) + "\r\n"
                    "Access-Control-Allow-Origin: *\r\n"
                    "Connection: close\r\n\r\n" + body;
                send(conn->client_fd, response.c_str(), (int)response.size(), 0);
                LANG_CLOSE_SOCKET(conn->client_fd);
                conn->client_fd = LANG_INVALID_SOCKET;
                return Value(0.0);
            }

            // HttpRespondRedirect(connHandle, url) — 302 redirect
            if (op->op == "HttpRespondRedirect") {
                int handle = (int)target.number;
                if (http_conns.find(handle) == http_conns.end())
                    throw std::runtime_error("HttpRespondRedirect: invalid connection handle");
                std::string url = evaluate(op->args[0].get()).to_string();
                HttpServerConn* conn = http_conns[handle];
                std::string response =
                    "HTTP/1.1 302 Found\r\n"
                    "Location: " + url + "\r\n"
                    "Content-Length: 0\r\n"
                    "Connection: close\r\n\r\n";
                send(conn->client_fd, response.c_str(), (int)response.size(), 0);
                LANG_CLOSE_SOCKET(conn->client_fd);
                conn->client_fd = LANG_INVALID_SOCKET;
                return Value(0.0);
            }

            // HttpRequestQuery(connHandle) → raw query string e.g. "name=James&age=21"
            if (op->op == "HttpRequestQuery") {
                int handle = (int)target.number;
                if (http_conns.find(handle) == http_conns.end())
                    throw std::runtime_error("HttpRequestQuery: invalid connection handle");
                return Value(http_conns[handle]->request.query);
            }

            // HttpRequestIP(connHandle) → client IP string
            if (op->op == "HttpRequestIP") {
                int handle = (int)target.number;
                if (http_conns.find(handle) == http_conns.end())
                    throw std::runtime_error("HttpRequestIP: invalid connection handle");
                // Store IP in request at accept time — for now return stored value
                return Value(http_conns[handle]->request.headers.count("x-client-ip")
                    ? http_conns[handle]->request.headers["x-client-ip"]
                    : std::string("unknown"));
            }

            // HttpServerClose(serverHandle) — shut down the server
            if (op->op == "HttpServerClose") {
                int handle = (int)target.number;
                if (http_servers.find(handle) == http_servers.end())
                    throw std::runtime_error("HttpServerClose: invalid server handle");
                LANG_CLOSE_SOCKET(http_servers[handle]->server_fd);
                delete http_servers[handle];
                http_servers.erase(handle);
                return Value(0.0);
            }

            // ── DNS extras ────────────────────────────────────────────────────────
            // DnsResolveIPv6("hostname") → "ipv6:addr:string"
            if (op->op == "DnsResolveIPv6") {
                std::string hostname = target.string;
                struct addrinfo hints{}, *res = nullptr;
                hints.ai_family   = AF_INET6;
                hints.ai_socktype = SOCK_STREAM;
                if (getaddrinfo(hostname.c_str(), nullptr, &hints, &res) != 0 || !res)
                    throw std::runtime_error("DnsResolveIPv6: failed to resolve: " + hostname);
                char ip[INET6_ADDRSTRLEN] = {};
                auto* addr = (struct sockaddr_in6*)res->ai_addr;
                inet_ntop(AF_INET6, &addr->sin6_addr, ip, sizeof(ip));
                freeaddrinfo(res);
                return Value(std::string(ip));
            }

            // DnsReverse("1.2.3.4") → "hostname"
            if (op->op == "DnsReverse") {
                std::string ip_str = target.string;
                struct sockaddr_in sa{};
                sa.sin_family = AF_INET;
                inet_pton(AF_INET, ip_str.c_str(), &sa.sin_addr);
                char host[NI_MAXHOST] = {};
                if (getnameinfo((struct sockaddr*)&sa, sizeof(sa), host, sizeof(host), nullptr, 0, 0) != 0)
                    throw std::runtime_error("DnsReverse: failed to resolve: " + ip_str);
                return Value(std::string(host));
            }

            // ── UDP ───────────────────────────────────────────────────────────────
            // UdpCreate(port) → handle  — creates a bound UDP socket
            if (op->op == "UdpCreate") {
                int port = (int)target.number;
                lang_socket_t fd = socket(AF_INET, SOCK_DGRAM, 0);
                if (fd == LANG_INVALID_SOCKET)
                    throw std::runtime_error("UdpCreate: failed to create socket");
                int opt = 1;
                setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
                if (port > 0) {
                    sockaddr_in addr{};
                    addr.sin_family      = AF_INET;
                    addr.sin_port        = htons(port);
                    addr.sin_addr.s_addr = INADDR_ANY;
                    if (bind(fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
                        LANG_CLOSE_SOCKET(fd);
                        throw std::runtime_error("UdpCreate: bind failed on port " + std::to_string(port));
                    }
                }
                auto* udp = new UdpSocket();
                udp->fd = fd;
                int handle = next_udp_handle++;
                udp_sockets[handle] = udp;
                return Value((double)handle);
            }

            // UdpSend(handle, host, port, message) → bytes sent
            if (op->op == "UdpSend") {
                int handle = (int)target.number;
                if (udp_sockets.find(handle) == udp_sockets.end())
                    throw std::runtime_error("UdpSend: invalid handle");
                std::string host = evaluate(op->args[0].get()).to_string();
                int port         = (int)evaluate(op->args[1].get()).number;
                std::string msg  = evaluate(op->args[2].get()).to_string();

                struct addrinfo hints{}, *res = nullptr;
                hints.ai_family   = AF_INET;
                hints.ai_socktype = SOCK_DGRAM;
                if (getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &res) != 0 || !res)
                    throw std::runtime_error("UdpSend: failed to resolve host: " + host);

                int sent = (int)sendto(udp_sockets[handle]->fd, msg.c_str(), (int)msg.size(), 0,
                                       res->ai_addr, (int)res->ai_addrlen);
                freeaddrinfo(res);
                if (sent < 0) throw std::runtime_error("UdpSend: sendto failed");
                return Value((double)sent);
            }

            // UdpReceive(handle) → string
            // UdpReceive(handle, bufSize) → string
            if (op->op == "UdpReceive") {
                int handle = (int)target.number;
                if (udp_sockets.find(handle) == udp_sockets.end())
                    throw std::runtime_error("UdpReceive: invalid handle");
                int buf_size = op->args.empty() ? 4096 : (int)evaluate(op->args[0].get()).number;
                std::vector<char> buf(buf_size);
                sockaddr_in sender{};
                socklen_t sender_len = sizeof(sender);
                int n = (int)recvfrom(udp_sockets[handle]->fd, buf.data(), buf_size - 1, 0,
                                      (sockaddr*)&sender, &sender_len);
                if (n < 0) throw std::runtime_error("UdpReceive: recvfrom failed");
                return Value(std::string(buf.data(), n));
            }

            // UdpReceiveFull(handle) → dict {data, ip, port}
            if (op->op == "UdpReceiveFull") {
                int handle = (int)target.number;
                if (udp_sockets.find(handle) == udp_sockets.end())
                    throw std::runtime_error("UdpReceiveFull: invalid handle");
                std::vector<char> buf(4096);
                sockaddr_in sender{};
                socklen_t sender_len = sizeof(sender);
                int n = (int)recvfrom(udp_sockets[handle]->fd, buf.data(), 4095, 0,
                                      (sockaddr*)&sender, &sender_len);
                if (n < 0) throw std::runtime_error("UdpReceiveFull: recvfrom failed");
                char ip[INET_ADDRSTRLEN] = {};
                inet_ntop(AF_INET, &sender.sin_addr, ip, sizeof(ip));
                auto d = std::make_shared<Dict>();
                (*d)["data"] = Value(std::string(buf.data(), n));
                (*d)["ip"]   = Value(std::string(ip));
                (*d)["port"] = Value((double)ntohs(sender.sin_port));
                return Value(d);
            }

            // UdpSetTimeout(handle, ms) — set receive timeout
            if (op->op == "UdpSetTimeout") {
                int handle = (int)target.number;
                if (udp_sockets.find(handle) == udp_sockets.end())
                    throw std::runtime_error("UdpSetTimeout: invalid handle");
                int ms = (int)evaluate(op->args[0].get()).number;
#ifdef _WIN32
                DWORD timeout = ms;
                setsockopt(udp_sockets[handle]->fd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
#else
                struct timeval tv{ ms / 1000, (ms % 1000) * 1000 };
                setsockopt(udp_sockets[handle]->fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
#endif
                return Value(0.0);
            }

            // UdpClose(handle)
            if (op->op == "UdpClose") {
                int handle = (int)target.number;
                if (udp_sockets.find(handle) == udp_sockets.end())
                    throw std::runtime_error("UdpClose: invalid handle");
                LANG_CLOSE_SOCKET(udp_sockets[handle]->fd);
                delete udp_sockets[handle];
                udp_sockets.erase(handle);
                return Value(0.0);
            }

            // UdpBroadcast(handle, port, message) — send to 255.255.255.255
            if (op->op == "UdpBroadcast") {
                int handle = (int)target.number;
                if (udp_sockets.find(handle) == udp_sockets.end())
                    throw std::runtime_error("UdpBroadcast: invalid handle");
                int port   = (int)evaluate(op->args[0].get()).number;
                std::string msg = evaluate(op->args[1].get()).to_string();
                int broadcastEnable = 1;
                setsockopt(udp_sockets[handle]->fd, SOL_SOCKET, SO_BROADCAST,
                           (char*)&broadcastEnable, sizeof(broadcastEnable));
                sockaddr_in addr{};
                addr.sin_family      = AF_INET;
                addr.sin_port        = htons(port);
                addr.sin_addr.s_addr = INADDR_BROADCAST;
                int sent = (int)sendto(udp_sockets[handle]->fd, msg.c_str(), (int)msg.size(), 0,
                                       (sockaddr*)&addr, sizeof(addr));
                if (sent < 0) throw std::runtime_error("UdpBroadcast: sendto failed");
                return Value((double)sent);
            }

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

        // Null comparisons
        if (left.is_null() || right.is_null()) {
            bool both_null = left.is_null() && right.is_null();
            switch (cmp->op) {
                case TokenType::EQUAL:     return both_null;
                case TokenType::NOT_EQUAL: return !both_null;
                default: throw std::runtime_error("Only == and != supported for Null comparison");
            }
        }

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

        case NodeType::ARRAY_ASSIGN:  // legacy fallthrough
        case NodeType::DICT_ASSIGN: {
            auto* assign = static_cast<DictAssignNode*>(node);
            if (variables.find(assign->name) == variables.end())
                throw std::runtime_error("Undefined variable: " + assign->name);
            Value& container = variables[assign->name];
            Value key = evaluate(assign->key.get());
            Value val = evaluate(assign->value.get());
            if (container.is_array()) {
                int index = (int)key.number;
                if (index < 0 || index >= (int)container.array->size())
                    throw std::runtime_error("Array index out of bounds");
                (*container.array)[index] = val;
            } else if (container.is_dict()) {
                (*container.dict)[key.to_string()] = val;
            } else {
                throw std::runtime_error(assign->name + " is not an array or dictionary");
            }
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

        case NodeType::LANGPACK_IMPORT:
            evaluate(node); // handled in evaluate
            break;

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

// ─────────────────────────────────────────────────────────────────────────────
// LANGPACK C API implementation
// These functions are the bridge between language_api.h and the interpreter
// ─────────────────────────────────────────────────────────────────────────────

// LangValue is just Value under the hood
struct LangValue : public Value {
    LangValue() : Value() {}
    LangValue(const Value& v) : Value(v) {}
};

// LangArgs wraps a vector<Value>
struct LangArgs {
    std::vector<Value> args;
};

// LangInterp is just Interpreter under the hood
// (already forward-declared as opaque in language_api.h)

extern "C" {

void lang_register(LangInterp* interp, const char* name, LangFunc fn) {
    Interpreter* i = reinterpret_cast<Interpreter*>(interp);
    std::string fn_name(name);
    i->register_function(fn_name, [fn](std::vector<Value> args) -> Value {
        LangArgs la;
        la.args = args;
        LangValue* result = fn(&la);
        if (!result) return Value::make_null();
        Value v = *result;
        delete result;
        return v;
    });
}

int lang_argc(LangArgs* args) {
    return (int)args->args.size();
}

LangValue* lang_arg(LangArgs* args, int index) {
    if (index < 0 || index >= (int)args->args.size()) {
        auto* v = new LangValue(); return v; // null
    }
    return new LangValue(args->args[index]);
}

LangType lang_type(LangValue* v) {
    if (!v) return LANG_NULL;
    if (v->is_number())  return LANG_NUMBER;
    if (v->is_string())  return LANG_STRING;
    if (v->is_boolean()) return LANG_BOOL;
    if (v->is_array())   return LANG_ARRAY;
    if (v->is_dict())    return LANG_DICT;
    return LANG_NULL;
}

int lang_is_number(LangValue* v) { return v && v->is_number()  ? 1 : 0; }
int lang_is_string(LangValue* v) { return v && v->is_string()  ? 1 : 0; }
int lang_is_bool  (LangValue* v) { return v && v->is_boolean() ? 1 : 0; }
int lang_is_array (LangValue* v) { return v && v->is_array()   ? 1 : 0; }
int lang_is_dict  (LangValue* v) { return v && v->is_dict()    ? 1 : 0; }
int lang_is_null  (LangValue* v) { return (!v || v->is_null()) ? 1 : 0; }

double lang_to_number(LangValue* v) {
    if (!v || v->is_null()) return 0.0;
    if (v->is_number())  return v->number;
    if (v->is_boolean()) return v->boolean ? 1.0 : 0.0;
    if (v->is_string())  { try { return std::stod(v->string); } catch(...) { return 0.0; } }
    return 0.0;
}

// We store the string result in a thread-local buffer so it's safe to return const char*
static thread_local std::string _lang_str_buf;
const char* lang_to_string(LangValue* v) {
    if (!v) { _lang_str_buf = "Null"; return _lang_str_buf.c_str(); }
    _lang_str_buf = v->to_string();
    return _lang_str_buf.c_str();
}

int lang_to_bool(LangValue* v) {
    if (!v) return 0;
    return v->truthy() ? 1 : 0;
}

int lang_array_len(LangValue* v) {
    if (!v || !v->is_array()) return 0;
    return (int)v->array->size();
}

LangValue* lang_array_get(LangValue* v, int index) {
    if (!v || !v->is_array()) return new LangValue();
    if (index < 0 || index >= (int)v->array->size()) return new LangValue();
    return new LangValue((*v->array)[index]);
}

LangValue* lang_dict_get(LangValue* v, const char* key) {
    if (!v || !v->is_dict()) return nullptr;
    auto it = v->dict->find(std::string(key));
    if (it == v->dict->end()) return nullptr;
    return new LangValue(it->second);
}

int lang_dict_has(LangValue* v, const char* key) {
    if (!v || !v->is_dict()) return 0;
    return v->dict->find(std::string(key)) != v->dict->end() ? 1 : 0;
}

LangValue* lang_number(double n) { return new LangValue(Value(n)); }
LangValue* lang_string(const char* s) { return new LangValue(Value(std::string(s))); }
LangValue* lang_bool  (int b) { return new LangValue(Value(b != 0)); }
LangValue* lang_null  (void) { return new LangValue(Value::make_null()); }

LangValue* lang_array_new(void) {
    return new LangValue(Value(std::make_shared<std::vector<Value>>()));
}

void lang_array_push(LangValue* arr, LangValue* v) {
    if (!arr || !arr->is_array() || !v) return;
    arr->array->push_back(*v);
}

LangValue* lang_dict_new(void) {
    return new LangValue(Value(std::make_shared<Dict>()));
}

void lang_dict_set(LangValue* dict, const char* key, LangValue* v) {
    if (!dict || !dict->is_dict() || !v) return;
    (*dict->dict)[std::string(key)] = *v;
}

void lang_error(LangInterp* interp, const char* message) {
    (void)interp; // not needed — exception propagates naturally
    throw std::runtime_error(std::string(message));
}

} // extern "C"
