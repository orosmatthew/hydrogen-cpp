#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <vector>

#include "./generation.hpp"
#include "./generationWin.hpp"

int main(int argc, char* argv[])
{
    if (argc < 2) {
        std::cerr << "Incorrect usage. Correct usage is..." << std::endl;
        std::cerr << "hydro <input.hy> <platform[win/linux]>" << std::endl;
        return EXIT_FAILURE;
    }

    std::string platform = "linux";
    if (argc > 2) {
        platform = argv[2];
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
        std::cerr << "Invalid program" << std::endl;
        exit(EXIT_FAILURE);
    }

    if (platform == "win") {
        GeneratorWin generator(prog.value());
        std::fstream file("out.asm", std::ios::out);
        file << generator.gen_prog();
        file.close();
        system("nasm -fwin64 out.asm");
        // Couldn't find a reliable linker for windows but it still generates valid windows assembly.
        // Also for windows you also have to link kernel32.dll to the linker
        // You could use GoLink for the linker if you want to (That's what I tested this with but it will work with any other windows linker).
    }
    else if (platform == "linux") {
        Generator generator(prog.value());
        std::fstream file("out.asm", std::ios::out);
        file << generator.gen_prog();
        file.close();
        system("nasm -felf64 out.asm");
        system("ld -o out out.o");
    }

    return EXIT_SUCCESS;
}