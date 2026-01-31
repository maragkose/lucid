#include <lucid/frontend/lexer.hpp>
#include <lucid/frontend/parser.hpp>
#include <lucid/semantic/type_checker.hpp>
#include <lucid/backend/compiler.hpp>
#include <lucid/backend/vm.hpp>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <cstdlib>

auto read_file(const std::string& path) -> std::string {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error(fmt::format("Could not open file: {}", path));
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

auto main(int argc, char* argv[]) -> int {
    bool verbose = false;
    bool compile_only = false;
    std::string input_file;
    std::string output_file;

    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-v" || arg == "--verbose") {
            verbose = true;
        } else if (arg == "-c") {
            compile_only = true;
        } else if (arg == "-o") {
            if (i + 1 < argc) {
                output_file = argv[++i];
            } else {
                fmt::print(stderr, "Error: -o requires an argument\n");
                return 1;
            }
        } else if (arg == "-h" || arg == "--help") {
            fmt::print("Usage: lucidc [options] <file.lucid>\n");
            fmt::print("Options:\n");
            fmt::print("  -c               Compile to standalone executable\n");
            fmt::print("  -o <file>        Specify output file name\n");
            fmt::print("  -v, --verbose    Show detailed compilation information\n");
            fmt::print("  -h, --help       Show this help message\n");
            fmt::print("\nExamples:\n");
            fmt::print("  lucidc hello.lucid              # Run directly (interpreter mode)\n");
            fmt::print("  lucidc -c hello.lucid -o hello  # Create standalone executable\n");
            return 0;
        } else {
            input_file = arg;
        }
    }

    if (input_file.empty()) {
        fmt::print(stderr, "Error: No input file specified\n");
        fmt::print("Usage: lucidc [options] <file.lucid>\n");
        fmt::print("Try 'lucidc --help' for more information\n");
        return 1;
    }

    // Determine output file name if compiling
    if (compile_only && output_file.empty()) {
        // Default: remove .lucid extension and use as executable name
        size_t dot_pos = input_file.find_last_of('.');
        if (dot_pos != std::string::npos) {
            output_file = input_file.substr(0, dot_pos);
        } else {
            output_file = input_file + ".out";
        }
    }

    try {
        // Read source file
        std::string source = read_file(input_file);

        if (verbose) {
            fmt::print("=== LUCID Compiler ===\n\n");
            fmt::print("Compiling: {}\n\n", input_file);
        }

        // Phase 1: Lexing
        if (verbose) fmt::print("--- Phase 1: Lexing ---\n");
        lucid::Lexer lexer(source, input_file);
        auto tokens = lexer.tokenize();

        // Check for lexer errors
        for (const auto& token : tokens) {
            if (token.type == lucid::TokenType::Error) {
                fmt::print(stderr, "Lexer error at {}:{}:{}: {}\n",
                    token.location.filename,
                    token.location.line,
                    token.location.column,
                    std::get<std::string>(*token.value)
                );
                return 1;
            }
        }

        if (verbose) fmt::print("✓ Lexed {} tokens\n\n", tokens.size());

        // Phase 2: Parsing
        if (verbose) fmt::print("--- Phase 2: Parsing ---\n");
        lucid::Parser parser(tokens);
        auto parse_result = parser.parse();

        if (!parse_result.is_ok()) {
            fmt::print(stderr, "Parse errors:\n");
            for (const auto& error : parse_result.errors) {
                fmt::print(stderr, "  {}:{}:{}: {}\n",
                    error.location.filename,
                    error.location.line,
                    error.location.column,
                    error.message
                );
            }
            return 1;
        }

        auto& program = *parse_result.program.value();
        if (verbose) fmt::print("✓ Parsed {} functions\n\n", program.functions.size());

        // Phase 3: Type Checking
        if (verbose) fmt::print("--- Phase 3: Type Checking ---\n");
        lucid::semantic::TypeChecker type_checker;
        auto type_result = type_checker.check_program(program);

        if (!type_result.success) {
            fmt::print(stderr, "Type errors:\n");
            for (const auto& error : type_result.errors) {
                fmt::print(stderr, "  {}:{}:{}: {}\n",
                    error.location.filename,
                    error.location.line,
                    error.location.column,
                    error.message
                );
            }
            return 1;
        }

        if (verbose) fmt::print("✓ Type checking passed\n\n");

        // Phase 4: Bytecode Compilation
        if (verbose) fmt::print("--- Phase 4: Bytecode Compilation ---\n");
        lucid::backend::Compiler compiler;
        auto bytecode = compiler.compile(&program);

        if (verbose) {
            fmt::print("✓ Compiled successfully!\n");
            fmt::print("  - Functions: {}\n", bytecode.functions.size());
            fmt::print("  - Constants: {}\n", bytecode.constants.size());
            fmt::print("  - Instructions: {} bytes\n\n", bytecode.instructions.size());
        }

        // Check for main() function
        if (!bytecode.has_function("main")) {
            fmt::print(stderr, "Error: No main() function found\n");
            return 1;
        }

        // Phase 5: Compilation or Execution
        if (compile_only) {
            // Compile to standalone executable
            if (verbose) fmt::print("--- Phase 5: Generating Executable ---\n");

            // Generate C++ source with embedded bytecode
            std::string cpp_source = bytecode.generate_executable_source();

            // Write to temporary file
            std::string temp_source = output_file + ".tmp.cpp";
            std::ofstream out_file(temp_source);
            if (!out_file.is_open()) {
                fmt::print(stderr, "Error: Could not create temporary file: {}\n", temp_source);
                return 1;
            }
            out_file << cpp_source;
            out_file.close();

            if (verbose) fmt::print("Generated C++ source: {}\n", temp_source);

            // Compile with g++ (match CMake flags)
            std::string compile_cmd = fmt::format(
                "g++ -std=c++20 -O2 -fsanitize=address,undefined -I../include {} -L. -llucid-core -lfmt -o {} 2>&1",
                temp_source, output_file
            );

            if (verbose) fmt::print("Compiling: {}\n", compile_cmd);

            int result = system(compile_cmd.c_str());
            if (result != 0) {
                fmt::print(stderr, "Error: Compilation failed\n");
                return 1;
            }

            // Clean up temporary file
            std::remove(temp_source.c_str());

            if (verbose) {
                fmt::print("✓ Executable created: {}\n", output_file);
            } else {
                fmt::print("Created executable: {}\n", output_file);
            }

            return 0;

        } else {
            // Execute directly (interpreter mode)
            if (verbose) fmt::print("--- Phase 5: Execution ---\n");

            lucid::backend::VM vm;
            auto result = vm.call_function(bytecode, "main", {});

            // Print result
            if (verbose) {
                fmt::print("Program returned: ");
            }

            if (result.is_int()) {
                fmt::print("{}\n", result.as_int());
                return static_cast<int>(result.as_int());
            } else if (result.is_float()) {
                fmt::print("{}\n", result.as_float());
                return 0;
            } else if (result.is_string()) {
                fmt::print("\"{}\"\n", result.as_string());
                return 0;
            } else if (result.is_bool()) {
                fmt::print("{}\n", result.as_bool() ? "true" : "false");
                return result.as_bool() ? 0 : 1;
            } else {
                fmt::print("<unknown type>\n");
                return 0;
            }
        }

    } catch (const std::exception& e) {
        fmt::print(stderr, "Error: {}\n", e.what());
        return 1;
    }
}
