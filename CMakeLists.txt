# nfsmw-linux: Native Linux desktop (x86_64) build of Need for Speed: Most Wanted (Black Edition)
# Built from RanggaCasper's MostWanted-Android host and the XboxXBE-Recomp SDK.
# Builds the recompiler (xbe_recomp, xbe_symbols) and the host (SDK runtime, NFSMW Direct3D 8,
# DirectSound, and SDL3 + Vulkan desktop host) as static libraries.
# Game code is recompiled and linked on player's PC via nfsmw-linux-build.

cmake_minimum_required(VERSION 3.22)
project(nfsmw_linux LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE OFF)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
set(NFSMW_MARCH "x86-64-v2" CACHE STRING "-march for every object")
add_compile_options(-march=${NFSMW_MARCH})

set(REPO ${CMAKE_CURRENT_SOURCE_DIR})
set(SDK ${REPO}/sdk)
set(GAME ${REPO}/src)
set(DESKTOP ${REPO}/desktop)
include(FetchContent)

# ------------------------------------------------------------------ Dependencies
set(ZYDIS_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(ZYDIS_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(ZYDIS_BUILD_DOXYGEN OFF CACHE BOOL "" FORCE)
set(ZYDIS_BUILD_SHARED_LIB OFF CACHE BOOL "" FORCE)
FetchContent_Declare(zydis GIT_REPOSITORY https://github.com/zyantific/zydis.git GIT_TAG v4.1.1 GIT_SHALLOW TRUE)
FetchContent_Declare(tomlplusplus GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git GIT_TAG v3.4.0 GIT_SHALLOW TRUE)
FetchContent_Declare(imgui GIT_REPOSITORY https://github.com/ocornut/imgui.git GIT_TAG v1.91.0 GIT_SHALLOW TRUE)

set(ENABLE_OPT OFF CACHE BOOL "" FORCE)
set(ENABLE_HLSL ON CACHE BOOL "" FORCE)
set(ENABLE_GLSLANG_BINARIES OFF CACHE BOOL "" FORCE)
set(ENABLE_SPVREMAPPER OFF CACHE BOOL "" FORCE)
set(GLSLANG_TESTS OFF CACHE BOOL "" FORCE)
set(GLSLANG_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
set(BUILD_EXTERNAL OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(glslang GIT_REPOSITORY https://github.com/KhronosGroup/glslang.git GIT_TAG 16.0.0 GIT_SHALLOW TRUE)

set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SDL_RENDER OFF CACHE BOOL "" FORCE)
set(SDL_GPU OFF CACHE BOOL "" FORCE)
set(SDL_CAMERA OFF CACHE BOOL "" FORCE)
set(SDL_OPENGL OFF CACHE BOOL "" FORCE)
set(SDL_OPENGLES OFF CACHE BOOL "" FORCE)
set(SDL_VULKAN ON CACHE BOOL "" FORCE)
set(SDL_X11 ON CACHE BOOL "" FORCE)
set(SDL_WAYLAND ON CACHE BOOL "" FORCE)
set(SDL_PULSEAUDIO ON CACHE BOOL "" FORCE)
set(SDL_PIPEWIRE ON CACHE BOOL "" FORCE)
set(SDL_ALSA ON CACHE BOOL "" FORCE)
set(SDL_JACK OFF CACHE BOOL "" FORCE)
set(SDL_SNDIO OFF CACHE BOOL "" FORCE)
set(SDL_KMSDRM OFF CACHE BOOL "" FORCE)
set(SDL_HIDAPI ON CACHE BOOL "" FORCE)
FetchContent_Declare(SDL3 GIT_REPOSITORY https://github.com/libsdl-org/SDL.git GIT_TAG release-3.4.16 GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(zydis tomlplusplus imgui glslang SDL3)

# ------------------------------------------------------------------ Recompiler tools
file(GLOB RECOMP_SOURCES ${SDK}/tools/recomp/*.cpp)
add_executable(xbe_recomp ${RECOMP_SOURCES})
target_include_directories(xbe_recomp PRIVATE ${SDK}/runtime ${SDK}/tools/recomp)
target_link_libraries(xbe_recomp PRIVATE Zydis tomlplusplus::tomlplusplus)
target_compile_options(xbe_recomp PRIVATE -include cstdlib)

add_executable(xbe_symbols ${SDK}/tools/symbols/main.cpp)
target_include_directories(xbe_symbols PRIVATE ${SDK}/runtime)
target_link_libraries(xbe_symbols PRIVATE Zydis tomlplusplus::tomlplusplus)

# ------------------------------------------------------------------ image_symbols.h
set(HDR ${CMAKE_CURRENT_BINARY_DIR}/image_header)
add_custom_command(OUTPUT ${HDR}/image_symbols.h
    COMMAND ${CMAKE_COMMAND} -E env XBE_HEADER_ONLY=1 $<TARGET_FILE:xbe_recomp> ${REPO}/config/game.toml
            ${REPO}/assets/default.xbe ${HDR} --xdk ${SDK}/xdk
    DEPENDS xbe_recomp ${REPO}/config/game.toml
    COMMENT "generating image_symbols.h from config/game.toml")
add_custom_target(image_header DEPENDS ${HDR}/image_symbols.h)

set(BUILD_COMMIT "0.1.0")
file(CONFIGURE OUTPUT ${HDR}/build_info.h CONTENT "#define NFSMW_COMMIT \"linux-${BUILD_COMMIT}\"\n")

# ------------------------------------------------------------------ Host static library
file(GLOB RUNTIME_SOURCES ${SDK}/runtime/*/*.cpp)
list(FILTER RUNTIME_SOURCES EXCLUDE REGEX
     "/runtime/app/|/core/input\\.cpp$|/gpu/(d3d12|render_thread)\\.cpp$|/audio/xaudio2\\.cpp$|/xdk/(dsound|d3d8)\\.cpp$")
file(GLOB GAME_HOST_SOURCES CONFIGURE_DEPENDS ${GAME}/*/*.cpp)
file(GLOB DESKTOP_SOURCES CONFIGURE_DEPENDS ${DESKTOP}/*.cpp)
set(IMGUI_SOURCES ${imgui_SOURCE_DIR}/imgui.cpp ${imgui_SOURCE_DIR}/imgui_draw.cpp ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp)

add_library(nfsmw_host STATIC ${RUNTIME_SOURCES} ${GAME_HOST_SOURCES} ${DESKTOP_SOURCES} ${IMGUI_SOURCES})
add_dependencies(nfsmw_host image_header)
target_include_directories(nfsmw_host PRIVATE ${DESKTOP}/compat ${SDK}/harness/compat ${DESKTOP} ${GAME} ${SDK}/runtime
                           ${HDR} ${glslang_SOURCE_DIR})
target_include_directories(nfsmw_host SYSTEM PRIVATE ${imgui_SOURCE_DIR})
target_compile_options(nfsmw_host PRIVATE -fno-strict-aliasing -fsigned-char)
set(HOST_SOURCES ${RUNTIME_SOURCES} ${GAME_HOST_SOURCES} ${DESKTOP_SOURCES})
set_source_files_properties(${HOST_SOURCES} PROPERTIES COMPILE_OPTIONS "-include;${DESKTOP}/compat/crt.h")
set_source_files_properties(${DESKTOP}/vk_shader.cpp PROPERTIES COMPILE_DEFINITIONS ENABLE_HLSL)
target_link_libraries(nfsmw_host PRIVATE SDL3::SDL3-static tomlplusplus::tomlplusplus glslang glslang-default-resource-limits)

# ------------------------------------------------------------------ Staging target
add_custom_target(stage ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/stage/lib
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:nfsmw_host> $<TARGET_FILE:SDL3::SDL3-static>
            $<TARGET_FILE:glslang> $<TARGET_FILE:glslang-default-resource-limits> $<TARGET_FILE:SPIRV>
            $<TARGET_FILE:MachineIndependent> $<TARGET_FILE:GenericCodeGen> $<TARGET_FILE:OSDependent>
            ${CMAKE_BINARY_DIR}/stage/lib/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:xbe_recomp> $<TARGET_FILE:xbe_symbols> ${CMAKE_BINARY_DIR}/stage/
    COMMAND ${CMAKE_COMMAND} -E copy ${HDR}/image_symbols.h ${CMAKE_BINARY_DIR}/stage/image_symbols.h
    DEPENDS nfsmw_host xbe_recomp xbe_symbols)
