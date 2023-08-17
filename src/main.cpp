#include <cctype>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <optional>
#include <vector>

enum class TokenType
{
    _return,
    int_lit,
    semi
};

struct Token
{
    TokenType type;
    std::optional<std::string> value{};
};

const std::string KEYWORDS[] = {
    "return"
};

std::string make_tokenword(std::string str)
{
    std::string tokenword;
    for (char c : str)
    {
        if (!std::isspace(c))
        {
            tokenword += c;
        }
        else { break; }
    }
    return tokenword;
}

std::string make_tokenint(std::string str)
{
    std::string tokenint;
    for (char c : str)
    {
        if (std::isdigit(c))
        {
            tokenint += c;
        }
        else { break; }
    }
    return tokenint;
}

std::string caret_display(size_t pos, size_t width = 1, size_t offset = 0) {
    std::stringstream output;
    if (width == 1) {
        output << std::string(pos + offset + 1, ' ') << '^';
    } else {
        output << std::string(pos - width + offset + 1, ' ') << std::string(width, '^');
    }
    return output.str();
}

std::string caret_to_line(const std::string &str, size_t pos, size_t line = 1, size_t width = 1) {
    std::stringstream output;
    size_t line_start = 0;
    for (size_t i = 0; i < pos; i++) {
        if (str.at(i) == '\n') {
            line++;
            line_start = i + 1;
        }
    }
    std::string line_str = std::to_string(line) + ": ";
    output << line_str << str.substr(line_start, str.find('\n', line_start) - line_start)
           << std::endl;
    output << caret_display(pos - line_start - 1, width, line_str.size());
    return output.str();
}

bool is_keyword(const std::string& str)
{
    for (const std::string& keyword : KEYWORDS)
    {
        if (str == keyword)
        {
            return true;
        }
    }
    return false;
}

Token get_keyword_token(const std::string& str)
{
    if (str == "return")
    {
        return Token{TokenType::_return};
    }

    std::cerr << "INTERNAL ERROR: get_keyword_token() called with invalid keyword '" << str << "'" << std::endl;
    exit(EXIT_FAILURE);
}

std::vector<Token> tokenize(const std::string& str, const char* filename)
{
    std::vector<Token> tokens;
    std::string buf;
    size_t pos = 0;
    size_t line = 1;
    while (pos < str.size()) {
        if (std::isspace(str.at(pos)))
        {
            pos++;
            continue;
        }
        else if (std::isalpha(str.at(pos)))
        {
            buf = make_tokenword(str.substr(pos));
            pos += buf.size();
            if (is_keyword(buf))
            {
                tokens.push_back(get_keyword_token(buf));
            } else 
            {
                std::cerr << filename << ":" << line << ":" << pos - buf.size() << ": ERROR: unexpected identifier '" << buf << "'" << std::endl;
                std::cerr << caret_to_line(str, pos, line, buf.size()) << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        else if (std::isdigit(str.at(pos)))
        {
            buf = make_tokenint(str.substr(pos));
            pos += buf.size();
            tokens.push_back(Token{TokenType::int_lit, buf});
        }
        else if (str.at(pos) == ';')
        {
            tokens.push_back(Token{TokenType::semi});
            pos++;
        } 
        else if (str.at(pos) == '\n')
        {
            line++;
            pos++;
        }
        else
        {
            std::cerr << filename << ":" << line << ":" << pos + 1 << ": ERROR: unexpected character '" << str.at(pos) << "'" << std::endl;
            std::cerr << caret_to_line(str, pos, line, 1) << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    return tokens;
}

std::string display_token(const Token& token)
{
    std::stringstream output;
    switch (token.type)
    {
        case TokenType::_return:
            output << "return";
            break;
        case TokenType::int_lit:
            output << "int_lit(" << token.value.value() << ")";
            break;
        case TokenType::semi:
            output << "semi";
            break;
    }
    return output.str();
}

std::string tokens_to_asm(const std::vector<Token>& tokens)
{
    std::stringstream output;
    output << "global _start\n_start:\n";
    for (int i = 0; i < tokens.size(); i++)
    {
        const Token& token = tokens.at(i);
        if (token.type == TokenType::_return)
        {
            if (i + 1 < tokens.size() && tokens.at(i + 1).type == TokenType::int_lit)
            {
                if (i + 2 < tokens.size() && tokens.at(i + 2).type == TokenType::semi)
                {
                    output << "    mov rax, 60\n";
                    output << "    mov rdi, " << tokens.at(i + 1).value.value() << "\n";
                    output << "    syscall";
                }
            }
        }
    }
    return output.str();
}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Incorrect usage. Correct usage is..." << std::endl;
        std::cerr << "hydro <input.hy>" << std::endl;
        return EXIT_FAILURE;
    }

    std::string contents;
    {
        std::stringstream contents_stream;
        std::fstream input(argv[1], std::ios::in);
        contents_stream << input.rdbuf();
        contents = contents_stream.str();
    }

    std::vector<Token> tokens = tokenize(contents, argv[1]);
    // for (const Token& token : tokens)
    // {
    //     std::cout << display_token(token) << std::endl;
    // }

    {
        std::fstream file("out.asm", std::ios::out);
        file << tokens_to_asm(tokens);
    }

    system("nasm -felf64 out.asm");
    system("ld -o out out.o");

    return EXIT_SUCCESS;
}
