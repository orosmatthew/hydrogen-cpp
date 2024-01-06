#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <vector>
#include <cstring>

#include "generation.hpp"

int main(int argc, char* argv[])
{

    std::string outputFile = "out"; // Output file
    std::string inputFile; // Input file. Cannot be empty
    bool debug = false; // Debug flag

    /*
    Arguments:
    -o :: specifies the output file. Optional. Default: out
    -h :: shows info / help command. Optional
    -d :: enables debug mode. Optional. Default: false

    Debug: When true: Will create out.asm and out.o and will not delete them
    */


    if (argc < 2) { // The command needs to have atleast two arguments. 1st: executable 2nd: input file
        std::cerr << "\u001B[31m \033[1m Error:\u001B[37m \033[0m Incorrect usage. Use -h argument for help!" << std::endl; // Use help menu instead of printing the usage
        exit(EXIT_FAILURE);
    }

    // Argument loop
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "-o") == 0) {
            // Check if there is another argument after -o
            if (i + 1 < argc) {
                outputFile = argv[i + 1];
                i++;  // Skip the next argument (output file name)
            } else {
                std::cerr << "\u001B[31m \033[1m Error:\u001B[37m \033[0m -o option requires an argument." << std::endl;
                exit(EXIT_FAILURE);
            }
        } else if (std::strcmp(argv[i], "-d") == 0) {
            // Set the flag to generate the debug files
            debug = true;
        } else if (std::strcmp(argv[i], "-h") == 0) {
            std::cout << "Welcome to the Hydrogen compiler help menu!" << std::endl << std::endl;

            std::cout << "Information:" << std::endl; // print information
            std::cout << "\tSources: https://github.com/orosmatthew/hydrogen-cpp" << std::endl;
            std::cout << "\tIssues: https://github.com/orosmatthew/hydrogen-cpp/issues" << std::endl;
            std::cout << "\tYouTube Series: https://www.youtube.com/playlist?list=PLUDlas_Zy_qC7c5tCgTMYq2idyyT241qs" << std::endl;
            std::cout << "\tLicense: MIT License: https://opensource.org/license/mit/" << std::endl << std::endl;

            std::cout << "Usage: " << std::endl; // print usage
            std::cout << "\thydro <[optional] flags> <input file> <[optional]flags>" << std::endl << std::endl;
            std::cout << "Argument List:" << std::endl; // print flag usage
            std::cout << "\t-h -- Show argument help." << std::endl;
            std::cout << "\t-o -- Specifie output file." << std::endl;
            std::cout << "\t-d -- Generate out.asm and out.asm file." << std::endl;
            exit(EXIT_SUCCESS); // So we dont generate the executable when using the help flags
        } else {
            inputFile = argv[i];
        }
    }

    // No output file, that starts with '-' that would be wierd
    if (inputFile.starts_with("-")) { 
        std::cerr << "\u001B[31m \033[1m Error:\u001B[37m \033[0m Unknown Argument: " << inputFile << "! Use -h argument for help!"  << std::endl; // print error
        exit(EXIT_FAILURE);
    }

    // Check if input file name is provided
    if (inputFile.empty()) {
        std::cerr << "\u001B[31m \033[1m Error:\u001B[37m \033[0m Input file not provided." << std::endl; 
        exit(EXIT_FAILURE);
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
        std::cerr << "\u001B[31m \033[1m Error:\u001B[37m \033[0m Invalid program" << std::endl;
        exit(EXIT_FAILURE);
    }

    {
        Generator generator(prog.value()); // Generate the assembly 
        std::fstream file("out.asm", std::ios::out); // Opening the file
        file << generator.gen_prog(); // Add input
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
