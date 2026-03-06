/**
 * language_api.h — LANGPACK Developer API
 * LANGUAGE Programming Language v2.0.0
 * https://github.com/B16SETC/LANGUAGE-Programming-Language
 *
 * Include this header when building a LANGPACK.
 * You do NOT need any other LANGUAGE headers.
 *
 * ── Quick Start ────────────────────────────────────────────────────────────
 *
 *   #include "language_api.h"
 *
 *   // Your function: takes a list of LangValue args, returns a LangValue
 *   LangValue my_add(LangArgs args) {
 *       double a = lang_to_number(lang_arg(args, 0));
 *       double b = lang_to_number(lang_arg(args, 1));
 *       return lang_number(a + b);
 *   }
 *
 *   // Called once when the user writes: Import MYPACK
 *   LANGPACK_EXPORT void langpack_register(LangInterp* interp) {
 *       lang_register(interp, "MyAdd", my_add);
 *   }
 *
 * ── Compile ─────────────────────────────────────────────────────────────────
 *
 *   Linux:
 *     g++ -std=c++17 -shared -fPIC -o MYPACK.langpack mypack.cpp
 *
 *   Windows (MSVC):
 *     cl /LD mypack.cpp /Fe:MYPACK.langpack
 *
 *   Windows (MinGW):
 *     g++ -std=c++17 -shared -o MYPACK.langpack mypack.cpp
 *
 * ── Install ──────────────────────────────────────────────────────────────────
 *
 *   LANGUAGE --install MYPACK     (once added to registry)
 *   or drop MYPACK.langpack into:
 *     Linux:   ~/.language/packages/
 *     Windows: %APPDATA%/LANGUAGE/packages/
 *
 * ────────────────────────────────────────────────────────────────────────────
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// ── Export macro ─────────────────────────────────────────────────────────────
// LANGPACK_EXPORT ensures the symbol is exported with C linkage (no name mangling)
// Always use this on langpack_register, langpack_name, langpack_version, langpack_author
#ifdef _WIN32
  #define LANGPACK_EXPORT extern "C" __declspec(dllexport)
#else
  #define LANGPACK_EXPORT extern "C" __attribute__((visibility("default")))
#endif

// ── Opaque interpreter handle ─────────────────────────────────────────────────
// You never touch the internals — just pass it to lang_register()
// When compiling the interpreter itself, LangInterp is typedef'd to Interpreter.
// When compiling a LANGPACK, it's an opaque struct you never dereference.
#ifndef LANGINTERP_DEFINED
#define LANGINTERP_DEFINED
typedef struct LangInterp_opaque LangInterp;
#endif

// ── Value types ───────────────────────────────────────────────────────────────
typedef enum {
    LANG_NUMBER  = 0,
    LANG_STRING  = 1,
    LANG_BOOL    = 2,
    LANG_ARRAY   = 3,
    LANG_DICT    = 4,
    LANG_NULL    = 5
} LangType;

// Opaque value — use the helper functions below to read/create values
typedef struct LangValue LangValue;

// Argument list passed to your function
typedef struct LangArgs LangArgs;

// ── Your function signature ───────────────────────────────────────────────────
typedef LangValue* (*LangFunc)(LangArgs* args);

// ── Register a function ───────────────────────────────────────────────────────
// Call this inside langpack_register() for each function you want to expose.
// name: what the user types in LANGUAGE code  e.g. "QtCreateWindow"
// fn:   your C function pointer
LANGPACK_EXPORT void lang_register(LangInterp* interp, const char* name, LangFunc fn);

// ── Read arguments ────────────────────────────────────────────────────────────
// Get argument count
LANGPACK_EXPORT int          lang_argc(LangArgs* args);

// Get argument at index (0-based). Returns a null value if out of range.
LANGPACK_EXPORT LangValue*   lang_arg(LangArgs* args, int index);

// ── Check value type ──────────────────────────────────────────────────────────
LANGPACK_EXPORT LangType     lang_type(LangValue* v);
LANGPACK_EXPORT int          lang_is_number(LangValue* v);
LANGPACK_EXPORT int          lang_is_string(LangValue* v);
LANGPACK_EXPORT int          lang_is_bool(LangValue* v);
LANGPACK_EXPORT int          lang_is_array(LangValue* v);
LANGPACK_EXPORT int          lang_is_dict(LangValue* v);
LANGPACK_EXPORT int          lang_is_null(LangValue* v);

// ── Read value contents ───────────────────────────────────────────────────────
LANGPACK_EXPORT double       lang_to_number(LangValue* v);
LANGPACK_EXPORT const char*  lang_to_string(LangValue* v); // do NOT free this pointer
LANGPACK_EXPORT int          lang_to_bool(LangValue* v);   // 1 = true, 0 = false

// Array helpers
LANGPACK_EXPORT int          lang_array_len(LangValue* v);
LANGPACK_EXPORT LangValue*   lang_array_get(LangValue* v, int index);

// Dict helpers
LANGPACK_EXPORT LangValue*   lang_dict_get(LangValue* v, const char* key);  // NULL if missing
LANGPACK_EXPORT int          lang_dict_has(LangValue* v, const char* key);  // 1 if exists

// ── Create return values ──────────────────────────────────────────────────────
LANGPACK_EXPORT LangValue*   lang_number(double n);
LANGPACK_EXPORT LangValue*   lang_string(const char* s);
LANGPACK_EXPORT LangValue*   lang_bool(int b);       // 1 = true, 0 = false
LANGPACK_EXPORT LangValue*   lang_null(void);
LANGPACK_EXPORT LangValue*   lang_array_new(void);
LANGPACK_EXPORT void         lang_array_push(LangValue* arr, LangValue* v);
LANGPACK_EXPORT LangValue*   lang_dict_new(void);
LANGPACK_EXPORT void         lang_dict_set(LangValue* dict, const char* key, LangValue* v);

// ── Throw an error from your function ────────────────────────────────────────
// This works like throw in LANGUAGE — can be caught with Try/Catch
LANGPACK_EXPORT void         lang_error(LangInterp* interp, const char* message);

// ── Required entry point ──────────────────────────────────────────────────────
// You MUST implement this function in your LANGPACK.
// It is called once when the user writes: Import YOURPACK
//
// Example:
//   LANGPACK_EXPORT void langpack_register(LangInterp* interp) {
//       lang_register(interp, "MyFunc", my_func);
//   }
//
// Optional metadata functions (implement if you want):
//   LANGPACK_EXPORT const char* langpack_name()    { return "MYPACK"; }
//   LANGPACK_EXPORT const char* langpack_version() { return "1.0.0"; }
//   LANGPACK_EXPORT const char* langpack_author()  { return "YourName"; }

#ifdef __cplusplus
} // extern "C"
#endif

/**
 * ── Full Example LANGPACK ────────────────────────────────────────────────────
 *
 * Save as: mymath.cpp
 * Compile: g++ -std=c++17 -shared -fPIC -o MYMATH.langpack mymath.cpp
 *
 * ─────────────────────────────────────────────────────────────────────────────
 *
 * #include "language_api.h"
 * #include <cmath>
 *
 * // Cube: MyMathCube(x) → x³
 * LangValue* my_cube(LangArgs* args) {
 *     if (lang_argc(args) < 1) lang_error(nullptr, "MyMathCube requires 1 argument");
 *     double x = lang_to_number(lang_arg(args, 0));
 *     return lang_number(x * x * x);
 * }
 *
 * // Fibonacci: MyMathFib(n) → nth fibonacci number
 * LangValue* my_fib(LangArgs* args) {
 *     int n = (int)lang_to_number(lang_arg(args, 0));
 *     if (n <= 1) return lang_number(n);
 *     double a = 0, b = 1;
 *     for (int i = 2; i <= n; i++) { double c = a + b; a = b; b = c; }
 *     return lang_number(b);
 * }
 *
 * LANGPACK_EXPORT const char* langpack_name()    { return "MYMATH"; }
 * LANGPACK_EXPORT const char* langpack_version() { return "1.0.0"; }
 * LANGPACK_EXPORT const char* langpack_author()  { return "YourName"; }
 *
 * LANGPACK_EXPORT void langpack_register(LangInterp* interp) {
 *     lang_register(interp, "MyMathCube", my_cube);
 *     lang_register(interp, "MyMathFib",  my_fib);
 * }
 *
 * ─────────────────────────────────────────────────────────────────────────────
 *
 * Then in LANGUAGE:
 *
 *   Import MYMATH
 *   Print ToString(MyMathCube(3))   # 27
 *   Print ToString(MyMathFib(10))   # 55
 *
 * ────────────────────────────────────────────────────────────────────────────
 */
