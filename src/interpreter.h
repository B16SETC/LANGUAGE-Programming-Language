#pragma once
#include "parser.h"
#include <map>
#include <set>
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>
#include <filesystem>
#include <functional>

// HTTP (libcurl) and WebSocket (libwebsockets) — optional, enabled if libraries present
#if defined(USE_CURL)
  #include <curl/curl.h>
#endif
#if defined(USE_WEBSOCKETS)
  #include <libwebsockets.h>
#endif
#ifdef _WIN32
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #pragma comment(lib, "ws2_32.lib")
  #define LANG_CLOSE_SOCKET(s) closesocket(s)
  typedef SOCKET lang_socket_t;
  #define LANG_INVALID_SOCKET INVALID_SOCKET
#else
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <netdb.h>
  #include <unistd.h>
  #define LANG_CLOSE_SOCKET(s) close(s)
  typedef int lang_socket_t;
  #define LANG_INVALID_SOCKET (-1)
#endif

// Forward declare dict type
using Dict = std::map<std::string, struct Value>;

struct Value {
    enum class Type { NUMBER, STRING, BOOLEAN, ARRAY, DICT, NULL_TYPE } type;

    double number = 0;
    std::string string;
    bool boolean = false;
    std::shared_ptr<std::vector<Value>> array;
    std::shared_ptr<Dict> dict;

    Value() : type(Type::NULL_TYPE) {}  // default = Null
    Value(double n) : type(Type::NUMBER), number(n) {}
    Value(const std::string& s) : type(Type::STRING), string(s) {}
    Value(bool b) : type(Type::BOOLEAN), boolean(b) {}
    Value(std::shared_ptr<std::vector<Value>> a) : type(Type::ARRAY), array(a) {}
    Value(std::shared_ptr<Dict> d) : type(Type::DICT), dict(d) {}

    static Value make_null() { return Value(); }

    bool is_number()  const { return type == Type::NUMBER; }
    bool is_string()  const { return type == Type::STRING; }
    bool is_boolean() const { return type == Type::BOOLEAN; }
    bool is_array()   const { return type == Type::ARRAY; }
    bool is_dict()    const { return type == Type::DICT; }
    bool is_null()    const { return type == Type::NULL_TYPE; }

    bool truthy() const {
        if (is_null())    return false;
        if (is_boolean()) return boolean;
        if (is_number())  return number != 0;
        if (is_string())  return !string.empty();
        if (is_array())   return !array->empty();
        if (is_dict())    return !dict->empty();
        return false;
    }

    std::string to_string() const {
        if (is_null())    return "Null";
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
        if (is_dict()) {
            std::string s = "{";
            bool first = true;
            for (auto& [k, v] : *dict) {
                if (!first) s += ", ";
                s += "\"" + k + "\": " + v.to_string();
                first = false;
            }
            return s + "}";
        }
        if (number == (int)number) return std::to_string((int)number);
        return std::to_string(number);
    }
};

struct ReturnException {
    Value value;
    ReturnException(Value v) : value(v) {}
};

struct BreakException {};
struct ContinueException {};

class Interpreter {
public:
    void execute(const std::vector<std::unique_ptr<ASTNode>>& statements);
    void import_file(const std::string& filepath);
    void set_current_dir(const std::string& dir) { current_dir = dir; }

    // LANGPACK API — register a native function callable from LANGUAGE scripts
    // name: the function name as it appears in LANGUAGE code e.g. "QtCreateWindow"
    // fn:   called with evaluated arguments, returns a Value
    using NativeFunction = std::function<Value(std::vector<Value>)>;
    void register_function(const std::string& name, NativeFunction fn) {
        native_functions[name] = fn;
    }

private:
    std::string current_dir;
    std::map<std::string, Value> variables;
    std::map<std::string, FuncDefNode*> functions;
    std::map<std::string, NativeFunction> native_functions; // LANGPACK registered functions
    std::set<std::string> imported_files;
    std::vector<std::vector<std::unique_ptr<ASTNode>>> imported_asts;

    // TCP socket state
    std::map<int, lang_socket_t> tcp_sockets;   // handle -> fd
    int next_socket_handle = 1;

    // UDP socket state
    struct UdpSocket {
        lang_socket_t fd = LANG_INVALID_SOCKET;
    };
    std::map<int, UdpSocket*> udp_sockets;
    int next_udp_handle = 1;

    // HTTP server state
    struct HttpRequest {
        std::string method;
        std::string path;
        std::string query;       // everything after ?
        std::string body;
        std::map<std::string, std::string> headers;
        std::map<std::string, std::string> params;  // query string parsed
    };
    struct HttpServerConn {
        lang_socket_t client_fd = LANG_INVALID_SOCKET;
        HttpRequest request;
        bool responded = false;
    };
    struct HttpServerState {
        lang_socket_t server_fd = LANG_INVALID_SOCKET;
    };
    std::map<int, HttpServerState*> http_servers;   // handle -> state
    std::map<int, HttpServerConn*>  http_conns;     // conn handle -> conn
    int next_http_handle = 1;

    // HTTP server helpers
    HttpRequest parse_http_request(const std::string& raw);
    std::string url_decode(const std::string& s);
    std::map<std::string, std::string> parse_query_string(const std::string& query);

#if defined(USE_WEBSOCKETS)
    // WebSocket state
    struct WsContext {
        lws_context* ctx = nullptr;
        lws* wsi = nullptr;
        std::string send_buf;
        std::string recv_buf;
        bool connected = false;
        bool closed = false;
    };
    std::map<int, WsContext*> ws_sockets;
    int next_ws_handle = 1;
    static int ws_callback(lws* wsi, enum lws_callback_reasons reason,
                           void* user, void* in, size_t len);
#endif

    Value evaluate(ASTNode* node);
    bool evaluate_condition(ASTNode* node);
    void execute_statement(ASTNode* node);
};

// LangInterp is Interpreter — used by the C API in language_api.h
#define LANGINTERP_DEFINED
typedef Interpreter LangInterp;
