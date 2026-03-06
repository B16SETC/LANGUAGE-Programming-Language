cmake_minimum_required(VERSION 3.20)
project(LANGUAGE VERSION 2.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Optional features ─────────────────────────────────────────────────────
option(USE_CURL       "Enable HTTP/HTTPS support via libcurl"       OFF)
option(USE_WEBSOCKETS "Enable WebSocket support via libwebsockets"  OFF)

add_executable(LANGUAGE
    src/main.cpp
    src/lexer.cpp
    src/parser.cpp
    src/interpreter.cpp
)

target_include_directories(LANGUAGE PRIVATE src)

# ── libcurl (HTTP/HTTPS) ──────────────────────────────────────────────────
if(USE_CURL)
    find_package(CURL REQUIRED)
    target_compile_definitions(LANGUAGE PRIVATE USE_CURL)
    target_link_libraries(LANGUAGE PRIVATE CURL::libcurl)
    message(STATUS "HTTP/HTTPS enabled via libcurl")
endif()

# ── libwebsockets (WebSocket) ─────────────────────────────────────────────
if(USE_WEBSOCKETS)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBWEBSOCKETS REQUIRED libwebsockets)
    target_compile_definitions(LANGUAGE PRIVATE USE_WEBSOCKETS)
    target_include_directories(LANGUAGE PRIVATE ${LIBWEBSOCKETS_INCLUDE_DIRS})
    target_link_libraries(LANGUAGE PRIVATE ${LIBWEBSOCKETS_LIBRARIES})
    message(STATUS "WebSocket enabled via libwebsockets")
endif()

# ── Platform: Linux ───────────────────────────────────────────────────────
if(UNIX AND NOT APPLE)
    # -rdynamic exports symbols so LANGPACKs can call back into the interpreter
    target_link_options(LANGUAGE PRIVATE -rdynamic -static-libgcc -static-libstdc++)
endif()

# ── Platform: MinGW (Windows cross-compile) ───────────────────────────────
if(MINGW)
    target_link_options(LANGUAGE PRIVATE -static -static-libgcc -static-libstdc++)
    target_link_libraries(LANGUAGE PRIVATE ws2_32)
endif()

# ── Platform: Windows MSVC ────────────────────────────────────────────────
if(WIN32 AND NOT MINGW)
    target_link_libraries(LANGUAGE PRIVATE ws2_32)
endif()
