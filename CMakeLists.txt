cmake_minimum_required(VERSION 3.21)
project(ComfyX VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Output binaries to portable-friendly location
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Platform detection
if(WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -DNOMINMAX -DWIN32_LEAN_AND_MEAN)
endif()

# ─────────────────────────────────────────────
# External Dependencies
# ─────────────────────────────────────────────

# GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(external/glfw)

# Dear ImGui (docking branch) - built as a library
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/external/imgui)
add_library(imgui STATIC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)
target_link_libraries(imgui PUBLIC glfw)
if(WIN32)
    target_link_libraries(imgui PUBLIC opengl32)
else()
    find_package(OpenGL REQUIRED)
    target_link_libraries(imgui PUBLIC OpenGL::GL)
endif()

# imgui-node-editor
set(IMGUI_NODE_EDITOR_DIR ${CMAKE_SOURCE_DIR}/external/imgui-node-editor)
add_library(imgui_node_editor STATIC
    ${IMGUI_NODE_EDITOR_DIR}/imgui_node_editor.cpp
    ${IMGUI_NODE_EDITOR_DIR}/imgui_canvas.cpp
    ${IMGUI_NODE_EDITOR_DIR}/crude_json.cpp
    ${IMGUI_NODE_EDITOR_DIR}/imgui_node_editor_api.cpp
)
target_include_directories(imgui_node_editor PUBLIC ${IMGUI_NODE_EDITOR_DIR})
target_link_libraries(imgui_node_editor PUBLIC imgui)

# nlohmann/json (header-only)
set(JSON_BuildTests OFF CACHE BOOL "" FORCE)
add_subdirectory(external/nlohmann-json)

# cpp-httplib (header-only) with SSL support
set(HTTPLIB_DIR ${CMAKE_SOURCE_DIR}/external/cpp-httplib)
add_library(httplib INTERFACE)
target_include_directories(httplib INTERFACE ${HTTPLIB_DIR})

# Try to find OpenSSL for HTTPS support (required for cloud AI APIs and license)
find_package(OpenSSL QUIET)
if(OPENSSL_FOUND)
    message(STATUS "OpenSSL found - HTTPS enabled")
    target_compile_definitions(httplib INTERFACE CPPHTTPLIB_OPENSSL_SUPPORT)
    target_link_libraries(httplib INTERFACE OpenSSL::SSL OpenSSL::Crypto)
else()
    message(WARNING "OpenSSL not found - HTTPS disabled. Cloud AI APIs and license system will not work.")
    message(WARNING "Install OpenSSL or set OPENSSL_ROOT_DIR to enable HTTPS support.")
endif()

if(WIN32)
    target_link_libraries(httplib INTERFACE ws2_32 crypt32)
endif()

# IXWebSocket
set(USE_TLS OFF CACHE BOOL "" FORCE)
set(USE_ZLIB OFF CACHE BOOL "" FORCE)
add_subdirectory(external/ixwebsocket)

# stb (header-only)
set(STB_DIR ${CMAKE_SOURCE_DIR}/external/stb)
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE ${STB_DIR})

# llama.cpp (optional, for local AI)
option(COMFYX_ENABLE_LOCAL_AI "Enable local AI via llama.cpp" OFF)
if(COMFYX_ENABLE_LOCAL_AI)
    set(LLAMA_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(LLAMA_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(LLAMA_BUILD_SERVER OFF CACHE BOOL "" FORCE)
    add_subdirectory(external/llama.cpp)
endif()

# ─────────────────────────────────────────────
# ComfyX Application
# ─────────────────────────────────────────────

file(GLOB_RECURSE COMFYX_SOURCES
    src/*.cpp
    src/*.h
)

add_executable(ComfyX WIN32 ${COMFYX_SOURCES})

target_include_directories(ComfyX PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(ComfyX PRIVATE
    imgui
    imgui_node_editor
    nlohmann_json::nlohmann_json
    httplib
    ixwebsocket
    stb
)

if(COMFYX_ENABLE_LOCAL_AI)
    target_link_libraries(ComfyX PRIVATE llama)
    target_compile_definitions(ComfyX PRIVATE COMFYX_LOCAL_AI_ENABLED)
endif()

if(WIN32)
    target_link_libraries(ComfyX PRIVATE
        dwmapi      # For DWM window effects
        shlwapi     # For path utilities
    )
endif()

# ─────────────────────────────────────────────
# Copy assets to build directory
# ─────────────────────────────────────────────

add_custom_command(TARGET ComfyX POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:ComfyX>/assets
    COMMENT "Copying assets to build directory"
)

# Create portable data directories
add_custom_command(TARGET ComfyX POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:ComfyX>/data
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:ComfyX>/data/workflows
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:ComfyX>/data/history
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:ComfyX>/data/cache
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:ComfyX>/data/ai_models
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:ComfyX>/runtime
    COMMENT "Creating portable data directories"
)
