#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <vector>

#include "./generation.hpp"

void error(char* msg) 
{
    std::cerr << msg <<std::endl;
    exit(EXIT_FAILURE);
}

int main(int argc, char* argv[])
{
    if (argc != 2) {
        error(const_cast<char*>("Incorrect usage. Correct usage is...\nhydro <input.hy>"));
    }

    std::string contents;
    {
        std::stringstream contents_stream;
        std::fstream input(argv[1], std::ios::in);
        contents_stream << input.rdbuf();
        contents = contents_stream.str();
    }

    Tokenizer tokenizer(std::move(contents));
    std::vector<Token> tokens = tokenizer.tokenize();

    Parser parser(std::move(tokens));
    std::optional<NodeProg> prog = parser.parse_prog();

    if (!prog.has_value()) {
        error(const_cast<char*>("Invalid program"));
    }

    Generator generator(prog.value());
    {
        std::fstream file("out.asm", std::ios::out);
        file << generator.gen_prog();
    }

    system("nasm -felf64 out.asm");
    system("ld -o out out.o");

    return EXIT_SUCCESS;
}