#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <vector>
#include <thread>

#include "./generation.hpp"
#include "./utils.hpp"

int main(int argc, char* argv[])
{
    utils::log_info("starting the hydro compiler");    
    if (argc != 2) {
        utils::log_error("incorrect usage. correct usage is -> hydro <input.hy>");
        utils::log_error("exiting the compiler with status code 1");
        return EXIT_FAILURE;
    }

    utils::Timer timer;

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
        utils::log_error("invalid program");
        utils::log_error("exiting the compiler with status code 1");
        exit(EXIT_FAILURE);
    }

    Generator generator(prog.value());
    {
        std::fstream file("out.asm", std::ios::out);
        file << generator.gen_prog();
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    system("nasm -felf64 out.asm");
    system("ld -o out out.o");

    auto timer_stop = timer.stop();

    utils::log_success("compilation finished in " + timer_stop + " secs");
    utils::log_success("exiting the compiler with status code 0");
    return EXIT_SUCCESS;
}