#pragma once
#include <string>
#include <array>
#include "syntax_token.h"

struct Lexer {
    Lexer(std::string input_string);
    std::array<SyntaxToken, 256> tokenize();

private:
    std::string input_string;
    size_t position = 0;

    char peek() const;
    char next();
    double parseNumber();
    FixedString16 parseIdentifier();
};