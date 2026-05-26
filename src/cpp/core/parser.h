#pragma once
#include <array>
#include <string>
#include <memory>
#include "syntax_token.h"
#include "expressions.h"

struct Parser {
    Parser(std::string input_string);
    Parser(std::array<SyntaxToken, 256> tokens);

    std::unique_ptr<Expression> parse();
    static void printAST(const Expression* expr, std::string prefix = "", bool is_last = true);

private:
    std::array<SyntaxToken, 256> tokens;
    size_t position;

    SyntaxToken peek() const;
    SyntaxToken consume();
    std::unique_ptr<Expression> parseExpression(uint8_t minBindingPower);
    std::unique_ptr<Expression> led(std::unique_ptr<Expression> left, SyntaxToken token);
    std::unique_ptr<Expression> nud(SyntaxToken token);
};