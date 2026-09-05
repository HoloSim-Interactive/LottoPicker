cmake_minimum_required(VERSION 3.21)
project(lottopicker LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Emit compile_commands.json for editor/IDE tooling (clangd, etc.) — harmless
# on generators that ignore it (e.g. the Visual Studio generator).
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(LOTTOPICKER_BUILD_TESTS "Build the Catch2 test suite" ON)

add_subdirectory(src)

if(LOTTOPICKER_BUILD_TESTS)
    enable_testing()

    include(FetchContent)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.5.4
    )
    FetchContent_MakeAvailable(Catch2)
    list(APPEND CMAKE_MODULE_PATH "${catch2_SOURCE_DIR}/extras")

    add_subdirectory(tests)
endif()
