cmake_minimum_required(VERSION 3.20)
project(lucid VERSION 0.1.0 LANGUAGES CXX)

# C++20 required
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for clangd/IDE
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Compiler warnings
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Werror
        -Wconversion
        -Wsign-conversion
    )
endif()

# Build type specific flags
set(CMAKE_CXX_FLAGS_DEBUG "-g3 -O0 -fsanitize=address,undefined")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG")

# Dependencies
find_package(fmt REQUIRED)

# Core library
add_library(lucid-core STATIC
    src/frontend/lexer.cpp
    src/frontend/ast.cpp           # Phase 2
    src/frontend/parser.cpp        # Phase 2
    src/frontend/ast_printer.cpp   # Phase 2 - Debug utility
    src/semantic/type_system.cpp   # Phase 3
    src/semantic/symbol_table.cpp  # Phase 3
    src/semantic/type_checker.cpp  # Phase 3
    src/backend/value.cpp          # Phase 4
    src/backend/bytecode.cpp       # Phase 4
    src/backend/compiler.cpp       # Phase 4 - Day 2
    src/backend/vm.cpp             # Phase 5
)

target_include_directories(lucid-core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(lucid-core
    PUBLIC
        fmt::fmt
)

# Compiler executable
add_executable(lucidc
    src/main.cpp
)

target_link_libraries(lucidc
    PRIVATE
        lucid-core
)

# Compiler demo (Phase 4 demonstration)
add_executable(compiler-demo
    test_compiler_demo.cpp
)

target_link_libraries(compiler-demo
    PRIVATE
        lucid-core
)

# Tests
enable_testing()

# Check for Catch2 - either system package or local amalgamated version
find_package(Catch2 3 QUIET)

if(NOT Catch2_FOUND AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/catch2/catch_amalgamated.hpp")
    # Use local amalgamated Catch2
    add_library(Catch2 STATIC external/catch2/catch_amalgamated.cpp)
    target_include_directories(Catch2 PUBLIC external/catch2)

    # Disable warnings for external library
    if(MSVC)
        target_compile_options(Catch2 PRIVATE /W0)
    else()
        target_compile_options(Catch2 PRIVATE -w)
    endif()

    add_library(Catch2::Catch2WithMain ALIAS Catch2)
    set(Catch2_FOUND TRUE)
    message(STATUS "Using local Catch2 amalgamated version")
endif()

if(Catch2_FOUND)
    add_executable(lucid-tests
        tests/lexer_test.cpp
        tests/parser_test.cpp        # Phase 2
        tests/type_system_test.cpp   # Phase 3
        tests/symbol_table_test.cpp  # Phase 3
        tests/type_checker_test.cpp  # Phase 3
        tests/compiler_test.cpp      # Phase 4
        tests/vm_test.cpp            # Phase 5
    )

    target_link_libraries(lucid-tests
        PRIVATE
            lucid-core
            Catch2::Catch2WithMain
    )

    # Add custom target to run tests
    add_custom_target(test-lexer
        COMMAND lucid-tests "[lexer]"
        DEPENDS lucid-tests
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Running lexer tests"
    )

    message(STATUS "Tests will be built")
else()
    message(WARNING "Catch2 not found. Tests will not be built.")
endif()

# Benchmarks (optional)
find_package(benchmark QUIET)

if(benchmark_FOUND AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/lexer_bench.cpp")
    add_executable(lucid-bench
        benchmarks/lexer_bench.cpp
    )

    target_link_libraries(lucid-bench
        PRIVATE
            lucid-core
            benchmark::benchmark
    )
endif()

# Installation rules
install(TARGETS lucid-core lucidc
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/lucid
    DESTINATION include
)

# Print configuration
message(STATUS "")
message(STATUS "LUCID Compiler Configuration:")
message(STATUS "  Version:        ${PROJECT_VERSION}")
message(STATUS "  C++ Standard:   C++${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Type:     ${CMAKE_BUILD_TYPE}")
message(STATUS "  Compiler:       ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "  Tests:          ${Catch2_FOUND}")
message(STATUS "  Benchmarks:     ${benchmark_FOUND}")
message(STATUS "")
