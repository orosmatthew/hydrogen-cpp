#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <vector>
#include <cstring>

#include "generation.hpp"

int main(int argc, char* argv[])
{

    std::string outputFile = "out";
    std::string inputFile;
    bool debug = false;


    if (argc < 2) {
        std::cerr << "Incorrect usage. Correct usage is..." << std::endl;
        std::cerr << "hydro <input.hy>" << std::endl;
        return EXIT_FAILURE;
    }

    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "-o") == 0) {
            // Check if there is another argument after -o
            if (i + 1 < argc) {
                outputFile = argv[i + 1];
                i++;  // Skip the next argument (output file name)
            } else {
                std::cerr << "Error: -o option requires an argument.\n";
                return 1;
            }
        } else if (std::strcmp(argv[i], "-d") == 0) {
            // Set the flag to execute the function
            debug = true;
        } else {
            // Assume the argument is an input file
            inputFile = argv[i];
        }
    }

    // Check if input file name is provided
    if (inputFile.empty()) {
        std::cerr << "Error: Input file not provided.\n";
        return 1;
    }

    std::string contents;
    {
        std::stringstream contents_stream;
        std::fstream input(inputFile, std::ios::in);
        contents_stream << input.rdbuf();
        contents = contents_stream.str();
    }

    Tokenizer tokenizer(std::move(contents));
    std::vector<Token> tokens = tokenizer.tokenize();

    Parser parser(std::move(tokens));
    std::optional<NodeProg> prog = parser.parse_prog();

    if (!prog.has_value()) {
        std::cerr << "Invalid program" << std::endl;
        exit(EXIT_FAILURE);
    }

    {
        Generator generator(prog.value());
        std::fstream file("out.asm", std::ios::out);
        file << generator.gen_prog();
    }

    std::string ldCmd = "ld out.o -o ";
    ldCmd.append(outputFile);

    system("nasm -felf64 out.asm");
    system(ldCmd.c_str());

    if (!debug) {
        system("rm out.asm");
        system("rm out.o");
    }


    return EXIT_SUCCESS;
}
