\
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>
#include <sstream> // Required for reading file content

// Include your factory and interface headers
#include "include/yaml_lexer_factory.h"
#include "include/lexer_interface.h"

// Basic argument parsing function (replace with a proper library for robustness)
void parseArgs(int argc, char* argv[], std::string& mode, std::string& general_config, std::string& backend_config, std::string& inputfile, std::string& outputfile) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--mode" && i + 1 < argc) {
            mode = argv[++i];
        } else if (arg == "--general-config" && i + 1 < argc) {
            general_config = argv[++i];
        } else if (arg == "--backend-config" && i + 1 < argc) {
            backend_config = argv[++i];
        } else if (arg == "--input-file" && i + 1 < argc) {
            inputfile = argv[++i];
        } else if (arg == "--output-file" && i + 1 < argc) {
            outputfile = argv[++i];
        }
    }

    if (mode.empty() || general_config.empty() || backend_config.empty() || inputfile.empty() || outputfile.empty()) {
        throw std::runtime_error("Missing required arguments. Usage: frontend --mode <mode> --general-config <path> --backend-config <path> --inputfile <path> --outputfile <path>");
    }

    if (mode != "FLEX" && mode != "DFA") {
        throw std::runtime_error("Invalid mode. Supported modes are: FLEX, DFA");
    }
}

// Function to read entire file content
std::string readFileContent(const std::string& filepath) {
    std::ifstream infile(filepath);
    if (!infile) {
        throw std::runtime_error("Could not open input file: " + filepath);
    }
    std::stringstream buffer;
    buffer << infile.rdbuf();
    return buffer.str();
}

int main(int argc, char* argv[]) {
    std::string mode, general_config, backend_config, inputfile, outputfile;

    try {
        parseArgs(argc, argv, mode, general_config, backend_config, inputfile, outputfile);

        std::cerr << "Mode: " << mode << std::endl;
        std::cerr << "General Config: " << general_config << std::endl;
        std::cerr << "Backend Config: " << backend_config << std::endl;
        std::cerr << "Input File: " << inputfile << std::endl;
        std::cerr << "Output File: " << outputfile << std::endl;

        // 1. Create Factory
        YAMLLexerFactory factory;

        // 2. Create Lexer
        std::unique_ptr<LexerInterface> lexer = factory.CreateLexer(mode, general_config, backend_config);
        std::cerr << "Lexer created successfully." << std::endl;

        // 3. Read Input File
        std::string input_content = readFileContent(inputfile);
        std::cerr << "Input file read successfully." << std::endl;

        // 4. Parse Input
        LexerResult result = lexer->Parse(input_content);
        std::cerr << "Parsing completed." << std::endl;

        // 5. Write Output File
        std::ofstream outfile(outputfile);
        if (!outfile) {
            throw std::runtime_error("Could not open output file: " + outputfile);
        }

        if (result.success) {
            std::cerr << "Parsing successful. Writing tokens..." << std::endl;
            for (const auto& token : result.tokens) {
                outfile << "(" << token.type << ", " << token.value << ")" << std::endl;
            }
            std::cerr << "Tokens written to " << outputfile << std::endl;
        } else {
            std::cerr << "Parsing failed. Writing error..." << std::endl;
            outfile << "Parsing Failed:" << std::endl;
            outfile << "---------------" << std::endl;
            outfile << "Error: " << result.error << std::endl;
            std::cerr << "Error message written to " << outputfile << std::endl;
        }
        outfile.close();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        // Optionally write error to output file even on setup failure
        std::ofstream outfile(outputfile, std::ios::app); // Append if file was already opened
         if (outfile) {
             outfile << "\n--- Execution Error ---" << std::endl;
             outfile << e.what() << std::endl;
         }
        return 1; // Indicate failure
    }

    return 0; // Indicate success
}
