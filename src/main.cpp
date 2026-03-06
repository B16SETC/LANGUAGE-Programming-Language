#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <cstring>
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"

// Platform includes for update (download + replace)
#ifdef _WIN32
  #include <windows.h>
#else
  #include <unistd.h>
  #include <sys/stat.h>
#endif

const std::string VERSION      = "2.0.0";
const std::string GITHUB_OWNER = "B16SETC";
const std::string GITHUB_REPO  = "LANGUAGE-Programming-Language";
const std::string RELEASES_API = "https://api.github.com/repos/" + GITHUB_OWNER + "/" + GITHUB_REPO + "/releases/latest";
const std::string ASSET_NAME   = "binaries.zip";

#ifdef _WIN32
  const std::string BINARY_NAME = "LANGUAGE.exe";
#else
  const std::string BINARY_NAME = "LANGUAGE";
#endif

// ─────────────────────────────────────────────────────────────────────────────
// Help
// ─────────────────────────────────────────────────────────────────────────────
void print_help() {
    std::cout << "\n";
    std::cout << "  ██╗      █████╗ ███╗   ██╗ ██████╗ ██╗   ██╗ █████╗  ██████╗ ███████╗\n";
    std::cout << "  ██║     ██╔══██╗████╗  ██║██╔════╝ ██║   ██║██╔══██╗██╔════╝ ██╔════╝\n";
    std::cout << "  ██║     ███████║██╔██╗ ██║██║  ███╗██║   ██║███████║██║  ███╗█████╗  \n";
    std::cout << "  ██║     ██╔══██║██║╚██╗██║██║   ██║██║   ██║██╔══██║██║   ██║██╔══╝  \n";
    std::cout << "  ███████╗██║  ██║██║ ╚████║╚██████╔╝╚██████╔╝██║  ██║╚██████╔╝███████╗\n";
    std::cout << "  ╚══════╝╚═╝  ╚═╝╚═╝  ╚═══╝ ╚═════╝  ╚═════╝ ╚═╝  ╚═╝ ╚═════╝ ╚══════╝\n";
    std::cout << "\n";
    std::cout << "  Programming Language v" << VERSION << "\n";
    std::cout << "  https://github.com/" << GITHUB_OWNER << "/" << GITHUB_REPO << "\n";
    std::cout << "\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    std::cout << "\n";
    std::cout << "  USAGE\n";
    std::cout << "    LANGUAGE <script.LANGUAGE>       Run a script\n";
    std::cout << "    LANGUAGE --help                  Show this help message\n";
    std::cout << "    LANGUAGE --version               Show version\n";
    std::cout << "    LANGUAGE --update                Check for updates and install if available\n";
    std::cout << "\n";
    std::cout << "  PACKAGE MANAGER\n";
    std::cout << "    LANGUAGE --install <package>     Install a LANGPACK\n";
    std::cout << "    LANGUAGE --uninstall <package>   Uninstall a LANGPACK\n";
    std::cout << "    LANGUAGE --list                  List installed packages\n";
    std::cout << "    LANGUAGE --search [query]        Search available packages\n";
    std::cout << "\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    std::cout << "\n";
    std::cout << "  LANGUAGE FEATURES\n";
    std::cout << "\n";
    std::cout << "  Variables & Types\n";
    std::cout << "    x = 42              number\n";
    std::cout << "    name = \"Alice\"      string\n";
    std::cout << "    flag = True         boolean\n";
    std::cout << "    arr = [1, 2, 3]     array\n";
    std::cout << "    d = {\"k\": \"v\"}     dictionary\n";
    std::cout << "    x = Null            null\n";
    std::cout << "\n";
    std::cout << "  Strings\n";
    std::cout << "    \"Hello {name}!\"     interpolation\n";
    std::cout << "    `multi\n";
    std::cout << "    line`               multiline (backtick)\n";
    std::cout << "\n";
    std::cout << "  Control Flow\n";
    std::cout << "    If / Elif / Else / End\n";
    std::cout << "    While <condition> / End\n";
    std::cout << "    For i = 1 To 10 / End\n";
    std::cout << "    Break / Continue\n";
    std::cout << "\n";
    std::cout << "  Functions\n";
    std::cout << "    Func name(a, b)     define a function\n";
    std::cout << "      Return a + b\n";
    std::cout << "    End\n";
    std::cout << "\n";
    std::cout << "  Built-ins\n";
    std::cout << "    Print, Input\n";
    std::cout << "    ReadFile, WriteFile, AppendFile\n";
    std::cout << "    Try / Catch(err) / End\n";
    std::cout << "    Import \"file.LANGUAGE\"\n";
    std::cout << "    Import PACKAGENAME      (LANGPACK)\n";
    std::cout << "\n";
    std::cout << "  Dictionaries\n";
    std::cout << "    DictKeys, DictValues, DictHas, DictRemove\n";
    std::cout << "    DictSize, DictMerge\n";
    std::cout << "\n";
    std::cout << "  JSON\n";
    std::cout << "    JsonParse, JsonStringify\n";
    std::cout << "\n";
    std::cout << "  Type Checks\n";
    std::cout << "    IsNull, IsNumber, IsString, IsBool, IsArray, IsDict\n";
    std::cout << "\n";
    std::cout << "  Math\n";
    std::cout << "    Floor, Ceil, Round, Abs, Sqrt, Power, Mod\n";
    std::cout << "    Sin, Cos, Tan, Asin, Acos, Atan, Atan2\n";
    std::cout << "    Log, Log10, Log2, Exp, Factorial, GCD, LCM\n";
    std::cout << "    Mean, Median, StdDev, Variance, Sum, Product\n";
    std::cout << "    Min, Max, Clamp, Lerp, Sign, Hypot, Cbrt\n";
    std::cout << "    Erf, Erfc, Gamma, Beta, IsNaN, IsInf\n";
    std::cout << "    BitAnd, BitOr, BitXor, BitNot, BitShiftLeft, BitShiftRight\n";
    std::cout << "\n";
    std::cout << "  Networking\n";
    std::cout << "    DNS\n";
    std::cout << "      DnsResolve, DnsResolveAll, DnsResolveIPv6, DnsReverse\n";
    std::cout << "\n";
    std::cout << "    TCP\n";
    std::cout << "      SocketConnect, SocketListen, SocketAccept\n";
    std::cout << "      SocketSend, SocketReceive, SocketReceiveLine\n";
    std::cout << "      SocketClose, SocketIsValid, SocketSetTimeout\n";
    std::cout << "\n";
    std::cout << "    UDP\n";
    std::cout << "      UdpCreate, UdpSend, UdpReceive, UdpReceiveFull\n";
    std::cout << "      UdpBroadcast, UdpSetTimeout, UdpClose\n";
    std::cout << "\n";
    std::cout << "    HTTP Client\n";
    std::cout << "      HttpGet, HttpPost, HttpPut, HttpDelete\n";
    std::cout << "      HttpGetJson, HttpPostJson, HttpGetFull\n";
    std::cout << "      HttpGetWithTimeout, HttpDownload\n";
    std::cout << "      HttpStatusCode, HttpHeaders\n";
    std::cout << "      HttpRequest, HttpRequestStatus\n";
    std::cout << "\n";
    std::cout << "    HTTP Server\n";
    std::cout << "      HttpServerCreate, HttpServerAccept, HttpServerClose\n";
    std::cout << "      HttpRequestMethod, HttpRequestPath, HttpRequestBody\n";
    std::cout << "      HttpRequestHeader, HttpRequestParam\n";
    std::cout << "      HttpRequestQuery, HttpRequestIP\n";
    std::cout << "      HttpRespond, HttpRespondJson\n";
    std::cout << "      HttpRespondFile, HttpRespondRedirect\n";
    std::cout << "      HttpConnClose\n";
    std::cout << "\n";
    std::cout << "    WebSocket\n";
    std::cout << "      WsConnect, WsSend, WsReceive, WsReceiveLine\n";
    std::cout << "      WsClose, WsIsConnected\n";
    std::cout << "\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    std::cout << "\n";
    std::cout << "  INDENTATION\n";
    std::cout << "    Use 2 spaces per level. Tabs are not supported.\n";
    std::cout << "\n";
    std::cout << "  COMMENTS\n";
    std::cout << "    # This is a comment #\n";
    std::cout << "\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    std::cout << "\n";
}

void print_version() {
    std::cout << "LANGUAGE v" << VERSION << "\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// Update
// ─────────────────────────────────────────────────────────────────────────────

// Parse "tag_name": "v1.3.1" from GitHub API JSON response
// Simple scan, no JSON library needed
std::string parse_tag_name(const std::string& json) {
    const std::string key = "\"tag_name\"";
    size_t pos = json.find(key);
    if (pos == std::string::npos) return "";
    pos = json.find("\"", pos + key.size());
    if (pos == std::string::npos) return "";
    pos++; // skip opening quote
    size_t end = json.find("\"", pos);
    if (end == std::string::npos) return "";
    return json.substr(pos, end - pos);
}

// Parse browser_download_url for the asset named ASSET_NAME
std::string parse_download_url(const std::string& json, const std::string& asset_name) {
    size_t pos = 0;
    while (true) {
        pos = json.find(asset_name, pos);
        if (pos == std::string::npos) return "";
        // Search backwards for browser_download_url near this asset name
        size_t search_start = (pos > 2000) ? pos - 2000 : 0;
        std::string chunk = json.substr(search_start, pos - search_start + asset_name.size() + 500);
        const std::string url_key = "\"browser_download_url\"";
        size_t url_pos = chunk.rfind(url_key);
        if (url_pos != std::string::npos) {
            url_pos = chunk.find("\"", url_pos + url_key.size());
            if (url_pos != std::string::npos) {
                url_pos++;
                size_t url_end = chunk.find("\"", url_pos);
                if (url_end != std::string::npos)
                    return chunk.substr(url_pos, url_end - url_pos);
            }
        }
        pos++;
    }
}

// Compare version strings like "1.3.0" vs "1.3.1"
// Returns true if remote > local
bool is_newer(const std::string& local, const std::string& remote) {
    auto parse = [](const std::string& v) {
        std::vector<int> parts;
        std::string s = v;
        if (!s.empty() && s[0] == 'v') s = s.substr(1);
        std::istringstream ss(s);
        std::string part;
        while (std::getline(ss, part, '.'))
            parts.push_back(std::stoi(part));
        while (parts.size() < 3) parts.push_back(0);
        return parts;
    };
    auto l = parse(local);
    auto r = parse(remote);
    for (size_t i = 0; i < 3; i++) {
        if (r[i] > l[i]) return true;
        if (r[i] < l[i]) return false;
    }
    return false;
}

void do_update(const std::string& exe_path) {
#if !defined(USE_CURL)
    std::cout << "  Update requires libcurl.\n";
    std::cout << "  Recompile with -DUSE_CURL=ON to enable --update.\n";
    return;
#else
    std::cout << "  Checking for updates...\n";

    // ── Step 1: fetch latest release info ────────────────────────────────
    std::string api_response;
    CURL* curl = curl_easy_init();
    if (!curl) { std::cout << "  Error: failed to init curl\n"; return; }

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "User-Agent: LANGUAGE-updater");
    headers = curl_slist_append(headers, "Accept: application/vnd.github+json");

    auto write_cb = [](char* ptr, size_t size, size_t nmemb, std::string* data) -> size_t {
        data->append(ptr, size * nmemb);
        return size * nmemb;
    };

    curl_easy_setopt(curl, CURLOPT_URL,           RELEASES_API.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER,    headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, +write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA,     &api_response);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT,        15L);

    CURLcode res = curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        std::cout << "  Error: could not reach GitHub: " << curl_easy_strerror(res) << "\n";
        return;
    }

    std::string remote_tag = parse_tag_name(api_response);
    if (remote_tag.empty()) {
        std::cout << "  Error: could not parse release info from GitHub.\n";
        return;
    }

    std::string remote_version = remote_tag;
    if (!remote_version.empty() && remote_version[0] == 'v')
        remote_version = remote_version.substr(1);

    std::cout << "  Current version : v" << VERSION << "\n";
    std::cout << "  Latest version  : " << remote_tag << "\n";

    if (!is_newer(VERSION, remote_version)) {
        std::cout << "  You are already on the latest version!\n";
        return;
    }

    std::cout << "  New version available! Downloading...\n";

    // ── Step 2: find download URL for binaries.zip ────────────────────────
    std::string download_url = parse_download_url(api_response, ASSET_NAME);
    if (download_url.empty()) {
        std::cout << "  Error: could not find " << ASSET_NAME << " in release assets.\n";
        return;
    }

    // ── Step 3: download binaries.zip to a temp file ──────────────────────
    std::string zip_path = exe_path + ".update.zip";
    FILE* zip_file = fopen(zip_path.c_str(), "wb");
    if (!zip_file) {
        std::cout << "  Error: could not write to " << zip_path << "\n";
        return;
    }

    auto file_write_cb = [](char* ptr, size_t size, size_t nmemb, FILE* f) -> size_t {
        return fwrite(ptr, size, nmemb, f);
    };

    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL,            download_url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,  +file_write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA,      zip_file);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT,        120L);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS,     0L);

    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    fclose(zip_file);

    if (res != CURLE_OK) {
        std::cout << "  Error: download failed: " << curl_easy_strerror(res) << "\n";
        std::remove(zip_path.c_str());
        return;
    }

    std::cout << "  Download complete. Extracting...\n";

    // ── Step 4: extract binary from zip using unzip / Expand-Archive ──────
    std::string extract_dir = exe_path + ".update_tmp";
    std::string new_binary  = extract_dir + "/" + BINARY_NAME;

#ifdef _WIN32
    // Use PowerShell Expand-Archive
    std::string ps_cmd = "powershell -Command \"Expand-Archive -Force '" +
                         zip_path + "' '" + extract_dir + "'\"";
    int extract_ret = system(ps_cmd.c_str());
#else
    std::string mkdir_cmd = "mkdir -p \"" + extract_dir + "\"";
    system(mkdir_cmd.c_str());
    std::string unzip_cmd = "unzip -o \"" + zip_path + "\" \"" + BINARY_NAME +
                            "\" -d \"" + extract_dir + "\"";
    int extract_ret = system(unzip_cmd.c_str());
#endif

    if (extract_ret != 0) {
        std::cout << "  Error: extraction failed.\n";
        std::remove(zip_path.c_str());
        return;
    }

    // ── Step 5: replace the running binary ───────────────────────────────
#ifdef _WIN32
    // On Windows we can't replace a running exe directly
    // Write a batch script that replaces it after we exit
    std::string batch_path = exe_path + ".update.bat";
    std::ofstream bat(batch_path);
    bat << "@echo off\n";
    bat << "ping -n 2 127.0.0.1 > nul\n"; // wait ~1s
    bat << "copy /Y \"" << new_binary << "\" \"" << exe_path << "\"\n";
    bat << "del \"" << zip_path << "\"\n";
    bat << "rmdir /S /Q \"" << extract_dir << "\"\n";
    bat << "del \"%~f0\"\n"; // delete self
    bat.close();
    // Launch batch detached
    STARTUPINFOA si{}; PROCESS_INFORMATION pi{};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    std::string cmd = "cmd /C \"" + batch_path + "\"";
    CreateProcessA(nullptr, &cmd[0], nullptr, nullptr, FALSE,
                   DETACHED_PROCESS | CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi);
    std::cout << "  Update will complete after LANGUAGE exits.\n";
    std::cout << "  Updated to " << remote_tag << "!\n";
#else
    // On Linux/Mac we can replace directly
    if (std::rename(new_binary.c_str(), exe_path.c_str()) != 0) {
        // rename across filesystems fails, use copy
        std::ifstream src(new_binary, std::ios::binary);
        std::ofstream dst(exe_path,   std::ios::binary);
        dst << src.rdbuf();
        src.close();
        dst.close();
        std::remove(new_binary.c_str());
    }
    // Restore executable permissions
    chmod(exe_path.c_str(), 0755);
    // Cleanup
    std::remove(zip_path.c_str());
    std::filesystem::remove_all(extract_dir);
    std::cout << "  Updated to " << remote_tag << "! Restart LANGUAGE to use the new version.\n";
#endif
#endif // USE_CURL
}

const std::string REGISTRY_URL = "https://raw.githubusercontent.com/" + GITHUB_OWNER + "/" + GITHUB_REPO + "/main/registry.json";

// Get the packages directory for the current platform
std::string get_packages_dir() {
#ifdef _WIN32
    const char* appdata = getenv("APPDATA");
    return std::string(appdata ? appdata : ".") + "\\LANGUAGE\\packages";
#else
    const char* home = getenv("HOME");
    return std::string(home ? home : ".") + "/.language/packages";
#endif
}

// Ensure packages directory exists
void ensure_packages_dir() {
    std::string dir = get_packages_dir();
#ifdef _WIN32
    // Create each component
    std::filesystem::create_directories(dir);
#else
    std::filesystem::create_directories(dir);
#endif
}

// ── Package manager ───────────────────────────────────────────────────────────

#if defined(USE_CURL)

// Fetch a URL and return body as string
static std::string pm_fetch(const std::string& url, int timeout = 15) {
    std::string body;
    CURL* curl = curl_easy_init();
    if (!curl) throw std::runtime_error("curl init failed");
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "User-Agent: LANGUAGE-pm");
    auto cb = [](char* ptr, size_t size, size_t nmemb, std::string* data) -> size_t {
        data->append(ptr, size * nmemb); return size * nmemb;
    };
    curl_easy_setopt(curl, CURLOPT_URL,            url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER,     headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,  +cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA,      &body);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT,        (long)timeout);
    CURLcode res = curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    if (res != CURLE_OK)
        throw std::runtime_error("Network error: " + std::string(curl_easy_strerror(res)));
    return body;
}

// Download URL to file path
static void pm_download_file(const std::string& url, const std::string& path) {
    FILE* f = fopen(path.c_str(), "wb");
    if (!f) throw std::runtime_error("Cannot write to: " + path);
    auto cb = [](char* ptr, size_t size, size_t nmemb, FILE* fp) -> size_t {
        return fwrite(ptr, size, nmemb, fp);
    };
    CURL* curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL,            url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,  +cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA,      f);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT,        120L);
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    fclose(f);
    if (res != CURLE_OK) {
        std::remove(path.c_str());
        throw std::runtime_error("Download failed: " + std::string(curl_easy_strerror(res)));
    }
}

// Parse a string value from flat JSON: "key": "value"
static std::string pm_json_str(const std::string& json, const std::string& key) {
    std::string search = "\"" + key + "\"";
    size_t pos = json.find(search);
    if (pos == std::string::npos) return "";
    pos = json.find("\"", pos + search.size());
    if (pos == std::string::npos) return "";
    pos++;
    size_t end = json.find("\"", pos);
    if (end == std::string::npos) return "";
    return json.substr(pos, end - pos);
}

// Extract the JSON object for a named package from the registry
static std::string pm_find_package(const std::string& registry, const std::string& name) {
    std::string search = "\"" + name + "\"";
    size_t pos = registry.find(search);
    if (pos == std::string::npos) return "";
    // Find opening brace of the package object
    pos = registry.find("{", pos + search.size());
    if (pos == std::string::npos) return "";
    // Find matching closing brace
    int depth = 0;
    size_t start = pos;
    while (pos < registry.size()) {
        if (registry[pos] == '{') depth++;
        else if (registry[pos] == '}') { depth--; if (depth == 0) break; }
        pos++;
    }
    return registry.substr(start, pos - start + 1);
}

// Get all package names from the registry JSON
static std::vector<std::string> pm_list_registry(const std::string& registry) {
    std::vector<std::string> names;
    // Find "packages": { ... } block
    size_t packages_pos = registry.find("\"packages\"");
    if (packages_pos == std::string::npos) return names;
    size_t brace = registry.find("{", packages_pos + 10);
    if (brace == std::string::npos) return names;
    brace++; // skip opening brace of packages block
    // Scan for top-level string keys
    while (brace < registry.size()) {
        // Skip whitespace
        while (brace < registry.size() && std::isspace(registry[brace])) brace++;
        if (registry[brace] == '}') break; // end of packages block
        if (registry[brace] == '"') {
            brace++;
            std::string name;
            while (brace < registry.size() && registry[brace] != '"') name += registry[brace++];
            brace++; // skip closing quote
            names.push_back(name);
            // Skip to the end of this package's object
            size_t obj_start = registry.find("{", brace);
            if (obj_start == std::string::npos) break;
            int depth = 0; brace = obj_start;
            while (brace < registry.size()) {
                if (registry[brace] == '{') depth++;
                else if (registry[brace] == '}') { depth--; if (depth == 0) { brace++; break; } }
                brace++;
            }
            // Skip optional comma
            while (brace < registry.size() && (std::isspace(registry[brace]) || registry[brace] == ',')) brace++;
        } else break;
    }
    return names;
}

void do_install(const std::string& package_name) {
    std::cout << "  Fetching registry...\n";
    std::string registry;
    try { registry = pm_fetch(REGISTRY_URL); }
    catch (const std::exception& e) {
        std::cout << "  Error: could not fetch registry: " << e.what() << "\n";
        return;
    }

    std::string pkg = pm_find_package(registry, package_name);
    if (pkg.empty()) {
        std::cout << "  Error: package '" << package_name << "' not found in registry.\n";
        std::cout << "  Run 'LANGUAGE --search' to see available packages.\n";
        return;
    }

    std::string version = pm_json_str(pkg, "version");
    std::string desc    = pm_json_str(pkg, "description");
    std::string author  = pm_json_str(pkg, "author");

#ifdef _WIN32
    std::string url = pm_json_str(pkg, "windows");
    if (url.empty()) url = pm_json_str(pkg, "win");
#else
    std::string url = pm_json_str(pkg, "linux");
#endif

    if (url.empty()) {
        std::cout << "  Error: no download URL for this platform in package '" << package_name << "'.\n";
        return;
    }

    std::cout << "  Package : " << package_name << "\n";
    std::cout << "  Version : " << version << "\n";
    std::cout << "  Author  : " << author << "\n";
    std::cout << "  Info    : " << desc << "\n";
    std::cout << "  Installing...\n";

    ensure_packages_dir();
    std::string out_path = get_packages_dir() + "/" + package_name + ".langpack";

    try { pm_download_file(url, out_path); }
    catch (const std::exception& e) {
        std::cout << "  Error: " << e.what() << "\n";
        return;
    }

#ifndef _WIN32
    std::filesystem::permissions(out_path,
        std::filesystem::perms::owner_read  | std::filesystem::perms::owner_write |
        std::filesystem::perms::group_read  | std::filesystem::perms::others_read);
#endif

    std::cout << "  Installed " << package_name << " v" << version << " successfully!\n";
    std::cout << "  Use with: Import " << package_name << "\n";
}

void do_uninstall(const std::string& package_name) {
    std::string path = get_packages_dir() + "/" + package_name + ".langpack";
    if (!std::filesystem::exists(path)) {
        std::cout << "  Error: package '" << package_name << "' is not installed.\n";
        return;
    }
    std::filesystem::remove(path);
    std::cout << "  Uninstalled " << package_name << ".\n";
}

void do_list_installed() {
    std::string dir = get_packages_dir();
    std::cout << "  Installed packages in " << dir << ":\n\n";
    if (!std::filesystem::exists(dir)) {
        std::cout << "  (none)\n";
        return;
    }
    bool found = false;
    for (auto& entry : std::filesystem::directory_iterator(dir)) {
        if (entry.path().extension() == ".langpack") {
            std::cout << "    " << entry.path().stem().string() << "\n";
            found = true;
        }
    }
    if (!found) std::cout << "  (none)\n";
}

void do_search(const std::string& query) {
    std::cout << "  Fetching registry...\n";
    std::string registry;
    try { registry = pm_fetch(REGISTRY_URL); }
    catch (const std::exception& e) {
        std::cout << "  Error: could not fetch registry: " << e.what() << "\n";
        return;
    }

    auto names = pm_list_registry(registry);
    if (names.empty()) {
        std::cout << "  No packages found in registry.\n";
        return;
    }

    std::cout << "\n";
    bool any = false;
    for (auto& name : names) {
        // Filter by query if provided
        if (!query.empty()) {
            std::string lower_name = name, lower_query = query;
            std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::tolower);
            std::transform(lower_query.begin(), lower_query.end(), lower_query.begin(), ::tolower);
            if (lower_name.find(lower_query) == std::string::npos) continue;
        }
        std::string pkg     = pm_find_package(registry, name);
        std::string version = pm_json_str(pkg, "version");
        std::string desc    = pm_json_str(pkg, "description");
        std::string author  = pm_json_str(pkg, "author");

        // Check if installed
        std::string installed_path = get_packages_dir() + "/" + name + ".langpack";
        bool installed = std::filesystem::exists(installed_path);

        std::cout << "  " << name << " v" << version << (installed ? "  [installed]" : "") << "\n";
        std::cout << "    " << desc << "\n";
        std::cout << "    by " << author << "\n\n";
        any = true;
    }
    if (!any) std::cout << "  No packages matching '" << query << "'.\n";
}

#else // no curl

void do_install(const std::string&) {
    std::cout << "  --install requires libcurl.\n";
    std::cout << "  Recompile with -DUSE_CURL=ON to enable the package manager.\n";
}
void do_uninstall(const std::string& package_name) {
    std::string path = get_packages_dir() + "/" + package_name + ".langpack";
    if (!std::filesystem::exists(path)) {
        std::cout << "  Error: package '" << package_name << "' is not installed.\n";
        return;
    }
    std::filesystem::remove(path);
    std::cout << "  Uninstalled " << package_name << ".\n";
}
void do_list_installed() {
    std::string dir = get_packages_dir();
    std::cout << "  Installed packages in " << dir << ":\n\n";
    if (!std::filesystem::exists(dir)) { std::cout << "  (none)\n"; return; }
    bool found = false;
    for (auto& entry : std::filesystem::directory_iterator(dir)) {
        if (entry.path().extension() == ".langpack") {
            std::cout << "    " << entry.path().stem().string() << "\n";
            found = true;
        }
    }
    if (!found) std::cout << "  (none)\n";
}
void do_search(const std::string&) {
    std::cout << "  --search requires libcurl.\n";
    std::cout << "  Recompile with -DUSE_CURL=ON to enable the package manager.\n";
}

#endif
std::string read_file(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open())
        throw std::runtime_error("Could not open file: " + filename);

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    std::string normalized;
    for (size_t i = 0; i < content.length(); i++) {
        if (content[i] == '\r' && i + 1 < content.length() && content[i + 1] == '\n')
            continue;
        normalized += content[i];
    }
    return normalized;
}

// ─────────────────────────────────────────────────────────────────────────────
// Main
// ─────────────────────────────────────────────────────────────────────────────
int main(int argc, char* argv[]) {
#ifdef _WIN32
    // Enable ANSI escape codes for the banner
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Error: Failed to initialize Winsock\n";
        return 1;
    }
#endif

    if (argc < 2) {
        print_help();
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    std::string arg = argv[1];

    if (arg == "--version" || arg == "-v") {
        print_version();
#ifdef _WIN32
        WSACleanup();
#endif
        return 0;
    }

    if (arg == "--help" || arg == "-h") {
        print_help();
#ifdef _WIN32
        WSACleanup();
#endif
        return 0;
    }

    if (arg == "--update") {
        std::string exe_path;
#ifdef _WIN32
        char buf[MAX_PATH];
        GetModuleFileNameA(nullptr, buf, MAX_PATH);
        exe_path = std::string(buf);
#else
        char buf[4096];
        ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
        if (len > 0) { buf[len] = '\0'; exe_path = std::string(buf); }
        else exe_path = std::string(argv[0]);
#endif
        do_update(exe_path);
#ifdef _WIN32
        WSACleanup();
#endif
        return 0;
    }

    if (arg == "--install") {
        if (argc < 3) {
            std::cout << "  Usage: LANGUAGE --install <package>\n";
#ifdef _WIN32
            WSACleanup();
#endif
            return 1;
        }
        do_install(argv[2]);
#ifdef _WIN32
        WSACleanup();
#endif
        return 0;
    }

    if (arg == "--uninstall") {
        if (argc < 3) {
            std::cout << "  Usage: LANGUAGE --uninstall <package>\n";
#ifdef _WIN32
            WSACleanup();
#endif
            return 1;
        }
        do_uninstall(argv[2]);
#ifdef _WIN32
        WSACleanup();
#endif
        return 0;
    }

    if (arg == "--list") {
        do_list_installed();
#ifdef _WIN32
        WSACleanup();
#endif
        return 0;
    }

    if (arg == "--search") {
        std::string query = (argc >= 3) ? argv[2] : "";
        do_search(query);
#ifdef _WIN32
        WSACleanup();
#endif
        return 0;
    }

    try {
        std::string source = read_file(arg);

        Lexer lexer(source);
        auto tokens = lexer.tokenize();

        Parser parser(tokens);
        auto ast = parser.parse();

        Interpreter interpreter;
        std::string script_dir = std::filesystem::weakly_canonical(arg).parent_path().string();
        interpreter.set_current_dir(script_dir);
        interpreter.execute(ast);

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}
