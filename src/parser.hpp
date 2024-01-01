#pragma once

#include <cassert>
#include <variant>

#include "arena.hpp"
#include "tokenization.hpp"

#include <typeinfo>


// My standard for compiler error messages:
/*

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

All error messages start with 'Compiler error, line: __'. The '__' holds an integer, eg. 23, and tells you where the error was thrown. The 'Compiler error' part is for clarity -- likely not necessary, but I like it.
It finds this line position by adding a new type of token, called the line_break. This token represents the '/n', and doesn't affect anything with how it gets compiled. There's also an int variable called cur_line. The only time it's read from is when an error is given, and the only time its value is changed is when the program hits a line_break.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If we have some specific reason to believe that some token should be of a type it is not, the format is as follows:

	Compiler error, line = __: Expected [token type] because of '[that specific reason]' but instead found [the token's real type]

For example: 
	
	Compiler error, line = 18: Expected 'identity' because of 'let' but instead found 'exit'

There may also be multiple types it could be, and here's an example of that:

	Compiler error, line = 18: Expected  'minus' or 'plus' or 'multiply' or 'divide' or 'closing_paren'  because of  'open_paren' and 'int_lit' but instead found 'exit'

Notice that, in both of those examples, the  token types don't quite match how they're actually called. That's because this is, of course, humanly added into the compiler. That assumes that the compiler continues expanding in the some vein, but frankly, I don't care.

This gives a very clear reason for why there was an error, and gives the coder the needed tools to fix it -- in this case, knowing exactly where it is, and what types were expected and found.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If the compiler knows that there should be an accompanying token, it uses this layout. To be clear, by 'there should be an accompanying token', I mean that if there's an opening parenthesis, then there has to be a closing one, else the code should not compile.

	Compiler error, line = __: Unaccompanied [whatever type of thing it is] -- could not find matching [whatever type the other think is]

For example:
	
	Compiler error, line = 18: Unaccompanied 'open_square_bracket' -- could not find matching 'closing_square_bracket'

This and the last example are very similar circumstances, so I'm not 100% sure if they should be seperate like this. It's really quite arbitrary what error messages I've used for what problems the compiler finds if it should be one of the two, but for now I'll keep it like this.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The last possibility is for if the compiler failed to parse an expression or statement or scope or something of the like, but that's all we know. That's written as such:

	                Compiler error, line = __: Failed to parse [what it failed to parse]

which seems somewhat self-evident in meaning.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

*/

int cur_line = 1;

struct NodeTermIntLit {
    Token int_lit;
};

struct NodeTermIdent {
    Token ident;
};

struct NodeExpr;

struct NodeTermParen {
    NodeExpr* expr;
};

struct NodeBinExprAdd {
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeBinExprMulti {
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeBinExprSub {
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeBinExprDiv {
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeBinExpr {
    std::variant<NodeBinExprAdd*, NodeBinExprMulti*, NodeBinExprSub*, NodeBinExprDiv*> var;
};

struct NodeTerm {
    std::variant<NodeTermIntLit*, NodeTermIdent*, NodeTermParen*> var;
};

struct NodeExpr {
    std::variant<NodeTerm*, NodeBinExpr*> var;
};

struct NodeStmtExit {
    NodeExpr* expr;
};

struct NodeStmtLet {
    Token ident;
    NodeExpr* expr {};
};

struct NodeStmt;

struct NodeScope {
    std::vector<NodeStmt*> stmts;
};

struct NodeIfPred;

struct NodeIfPredElif {
    NodeExpr* expr {};
    NodeScope* scope {};
    std::optional<NodeIfPred*> pred;
};

struct NodeIfPredElse {
    NodeScope* scope;
};

struct NodeIfPred {
    std::variant<NodeIfPredElif*, NodeIfPredElse*> var;
};

struct NodeStmtIf {
    NodeExpr* expr {};
    NodeScope* scope {};
    std::optional<NodeIfPred*> pred;
};

struct NodeStmt {
    std::variant<NodeStmtExit*, NodeStmtLet*, NodeScope*, NodeStmtIf*> var;
};

struct NodeProg {
    std::vector<NodeStmt*> stmts;
};

class Parser {
public:
    explicit Parser(std::vector<Token> tokens)
        : m_tokens(std::move(tokens))
        , m_allocator(1024 * 1024 * 4) // 4 mb
    {
    }

    std::optional<NodeTerm*> parse_term() // NOLINT(*-no-recursion)
    {
        if (auto int_lit = try_consume(TokenType::int_lit)) {
            auto term_int_lit = m_allocator.emplace<NodeTermIntLit>(int_lit.value());
            auto term = m_allocator.emplace<NodeTerm>(term_int_lit);
            return term;
        }
        if (auto ident = try_consume(TokenType::ident)) {
            auto expr_ident = m_allocator.emplace<NodeTermIdent>(ident.value());
            auto term = m_allocator.emplace<NodeTerm>(expr_ident);
            return term;
        }
        if (auto open_paren = try_consume(TokenType::open_paren)) {
            auto expr = parse_expr();
            if (!expr.has_value()) {
                std::cerr << "Compiler error, line = " + std::to_string(cur_line) + ": Failed to parse expression" << std::endl;
                exit(EXIT_FAILURE);
            }
            try_consume(TokenType::close_paren, "Compiler error, line = " + std::to_string(cur_pos) + ": Expected `close_paren` because of 'open_paren' and 'expression' but instead found " + typeid(peek().value()).name());
            auto term_paren = m_allocator.emplace<NodeTermParen>(expr.value());
            auto term = m_allocator.emplace<NodeTerm>(term_paren);
            return term;
        }
        return {};
    }

    std::optional<NodeExpr*> parse_expr(const int min_prec = 0) // NOLINT(*-no-recursion)
    {
        std::optional<NodeTerm*> term_lhs = parse_term();
        if (!term_lhs.has_value()) {
            return {};
        }
        auto expr_lhs = m_allocator.emplace<NodeExpr>(term_lhs.value());

        while (true) {
            std::optional<Token> curr_tok = peek();
            std::optional<int> prec;
            if (curr_tok.has_value()) {
                prec = bin_prec(curr_tok->type);
                if (!prec.has_value() || prec < min_prec) {
                    break;
                }
            }
            else {
                break;
            }
            const auto [type, value] = consume();
            const int next_min_prec = prec.value() + 1;
            auto expr_rhs = parse_expr(next_min_prec);
            if (!expr_rhs.has_value()) {
                std::cerr << "Compiler error, line = " + std::to_string(cur_line) + ": Failed to parse expression" << std::endl;
                exit(EXIT_FAILURE);
            }
            auto expr = m_allocator.emplace<NodeBinExpr>();
            auto expr_lhs2 = m_allocator.emplace<NodeExpr>();
            if (type == TokenType::plus) {
                expr_lhs2->var = expr_lhs->var;
                auto add = m_allocator.emplace<NodeBinExprAdd>(expr_lhs2, expr_rhs.value());
                expr->var = add;
            }
            else if (type == TokenType::star) {
                expr_lhs2->var = expr_lhs->var;
                auto multi = m_allocator.emplace<NodeBinExprMulti>(expr_lhs2, expr_rhs.value());
                expr->var = multi;
            }
            else if (type == TokenType::minus) {
                expr_lhs2->var = expr_lhs->var;
                auto sub = m_allocator.emplace<NodeBinExprSub>(expr_lhs2, expr_rhs.value());
                expr->var = sub;
            }
            else if (type == TokenType::fslash) {
                expr_lhs2->var = expr_lhs->var;
                auto div = m_allocator.emplace<NodeBinExprDiv>(expr_lhs2, expr_rhs.value());
                expr->var = div;
            }
            else {
                assert(false); // Unreachable;
            }
            expr_lhs->var = expr;
        }
        return expr_lhs;
    }

    std::optional<NodeScope*> parse_scope() // NOLINT(*-no-recursion)
    {
        if (!try_consume(TokenType::open_curly).has_value()) {
            return {};
        }
        auto scope = m_allocator.emplace<NodeScope>();
        while (auto stmt = parse_stmt()) {
            scope->stmts.push_back(stmt.value());
        }
        try_consume(TokenType::close_curly, "Compiler error, line = " + std::to_string(cur_line) + ": Unaccompanied 'open_curly_bracket' -- could not find matching 'closing_curly_bracket'");
        return scope;
    }

    std::optional<NodeIfPred*> parse_if_pred() // NOLINT(*-no-recursion)
    {
        if (try_consume(TokenType::elif)) {
            try_consume(TokenType::open_paren, "Compiler error, line = " + std::to_string(cur_line) + ": Expected `open_paren` because of 'elif' but instead found " + typeid(peek().value()).name());
            const auto elif = m_allocator.alloc<NodeIfPredElif>();
            if (const auto expr = parse_expr()) {
                elif->expr = expr.value();
            }
            else {
                std::cerr << "Compiler error, line = " + std::to_string(cur_line) + ": Failed to parse expression" << std::endl;
                exit(EXIT_FAILURE);
            }
            try_consume(TokenType::close_paren,"Compiler error, line = " + std::to_string(cur_line) + ": Unaccompanied 'open_paren' -- could not find matching 'closing_paren'");
            if (const auto scope = parse_scope()) {
                elif->scope = scope.value();
            }
            else {
                std::cerr << "Compiler error, line = " + std::to_string(cur_line) + ": Failed to parse scope" << std::endl;
                exit(EXIT_FAILURE);
            }
            elif->pred = parse_if_pred();
            auto pred = m_allocator.emplace<NodeIfPred>(elif);
            return pred;
        }
        if (try_consume(TokenType::else_)) {
            auto else_ = m_allocator.alloc<NodeIfPredElse>();
            if (const auto scope = parse_scope()) {
                else_->scope = scope.value();
            }
            else {
                std::cerr << "Compiler error, line = " + std::to_string(cur_line) + ": Failed to parse scope" << std::endl;
                exit(EXIT_FAILURE);
            }
            auto pred = m_allocator.emplace<NodeIfPred>(else_);
            return pred;
        }
        return {};
    }

    std::optional<NodeStmt*> parse_stmt() // NOLINT(*-no-recursion)
    {
        if (peek().value().type == TokenType::exit && peek(1).has_value()
            && peek(1).value().type == TokenType::open_paren) {
            consume();
            consume();
            auto stmt_exit = m_allocator.emplace<NodeStmtExit>();
            if (const auto node_expr = parse_expr()) {
                stmt_exit->expr = node_expr.value();
            }
            else {
                std::cerr << "Compiler error, line = " + std::to_string(cur_line) + ": Failed to parse expression" << std::endl;
                exit(EXIT_FAILURE);
            }
            try_consume(TokenType::close_paren,"Compiler error, line = " + std::to_string(cur_line) + ": Unaccompanied 'open_paren' -- could not find matching 'close_paren'");
            try_consume(TokenType::semi, "Compiler error, line = " + std::to_string(cur_line) + ": Expected 'semicolon' because of 'close_paren' but instead found " + typeid(peek().value()).name());
            auto stmt = m_allocator.emplace<NodeStmt>();
            stmt->var = stmt_exit;
            return stmt;
        }
        if (peek().has_value() && peek().value().type == TokenType::let && peek(1).has_value()
            && peek(1).value().type == TokenType::ident && peek(2).has_value()
            && peek(2).value().type == TokenType::eq) {
            consume();
            auto stmt_let = m_allocator.emplace<NodeStmtLet>();
            stmt_let->ident = consume();
            consume();
            if (const auto expr = parse_expr()) {
                stmt_let->expr = expr.value();
            }
            else {
                std::cerr << "Compiler error, line = " + std::to_string(cur_line) + ": Failed to parse expression" << std::endl;
                exit(EXIT_FAILURE);
            }
            try_consume(TokenType::semi, "Compiler error, line = " + std::to_string(cur_line) + ": Expected 'semicolon' because of 'close_paren' but instead found " + typeid(peek().value()).name());
            auto stmt = m_allocator.emplace<NodeStmt>();
            stmt->var = stmt_let;
            return stmt;
        }
        if (peek().has_value() && peek().value().type == TokenType::open_curly) {
            if (auto scope = parse_scope()) {
                auto stmt = m_allocator.emplace<NodeStmt>(scope.value());
                return stmt;
            }
                std::cerr << "Compiler error, line = " + std::to_string(cur_line) + ": Failed to parse scope" << std::endl;
            exit(EXIT_FAILURE);
        }
        if (auto if_ = try_consume(TokenType::if_)) {
            try_consume(TokenType::open_paren, "Compiler error, line = " + std::to_string(cur_line) + ": Expected 'open_paren' because of 'if' but instead found " + typeid(peek().value()).name());
            auto stmt_if = m_allocator.emplace<NodeStmtIf>();
            if (const auto expr = parse_expr()) {
                stmt_if->expr = expr.value();
            }
            else {
                std::cerr << "Compiler error, line = " + std::to_string(cur_line) + ": Failed to parse expression" << std::endl;
                exit(EXIT_FAILURE);
            }
            try_consume(TokenType::close_paren, "Compiler error, line = " + std::to_string(cur_line) + ": Expected 'close_paren' because of 'open_paren' and 'expression' but instead found " + typeid(peek().value()).name());
            if (const auto scope = parse_scope()) {
                stmt_if->scope = scope.value();
            }
            else {
                std::cerr << "Compiler error, line = " + std::to_string(cur_line) + ": Failed to parse scope" << std::endl;
                exit(EXIT_FAILURE);
            }
            stmt_if->pred = parse_if_pred();
            auto stmt = m_allocator.emplace<NodeStmt>(stmt_if);
            return stmt;
        }
        return {};
    }

    std::optional<NodeProg> parse_prog()
    {
        NodeProg prog;
        while (peek().has_value()) {
            if (auto stmt = parse_stmt()) {
                prog.stmts.push_back(stmt.value());
            }
            else {
                std::cerr << "Compiler error, line = " + std::to_string(cur_line) + ": Failed to parse statement" << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        return prog;
    }

private:
    [[nodiscard]] std::optional<Token> peek(const size_t offset = 0) const
    {
        if (m_index + offset >= m_tokens.size()) {
            return {};
        }
        return m_tokens.at(m_index + offset);
    }

    Token consume()
    {
	if (m_tokens.at(m_index) == '/n'){
		cur_line += 1;
	}
        return m_tokens.at(m_index++);
    }

    Token try_consume(const TokenType type, const std::string& err_msg)
    {
        if (peek().has_value() && peek().value().type == type) {
            return consume();
        }
        std::cerr << err_msg << std::endl;
        exit(EXIT_FAILURE);
    }

    std::optional<Token> try_consume(const TokenType type)
    {
        if (peek().has_value() && peek().value().type == type) {
            return consume();
        }
        return {};
    }

    const std::vector<Token> m_tokens;
    size_t m_index = 0;
    ArenaAllocator m_allocator;
};
