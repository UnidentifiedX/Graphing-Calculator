#include "lexer.h"
#include "syntax_token.h"
#include "fixed_string_16.h"
#include <array>
#include <iostream>

Lexer::Lexer(std::string input_string) : input_string(input_string) {}

char Lexer::peek() const {
    if (position < input_string.size()) {
        return input_string[position];
    }

    return '\0';
}

char Lexer::next() {
    if (position < input_string.size()) {
        char c = input_string[position];
        position++;
        return c;
    }

    return '\0';
}

double Lexer::parseNumber() {
    double value = 0.0;
    double decimalFactor = 1.0;
    bool hasDigits = false;
    bool hasDecimalPoint = false;

    while (position < input_string.size()) {
        char c = peek();

        if (std::isdigit(c)) {
            hasDigits = true;
            if (!hasDecimalPoint) { // build the integer part
                value = value * 10 + (c - '0'); // '5' - '0' = 5
            } else {
                decimalFactor *= 0.1; // build the fractional part
                value += (c - '0') * decimalFactor;
            }
            next();
        } else if (c == '.' && !hasDecimalPoint) {
            hasDecimalPoint = true;
            next();
        } else {
            break;
        }
    }

    if (!hasDigits) {
        std::cout << "Error: Expected a number at position " << position << std::endl;
        return 0.0;
    }

    return value;
}

FixedString16 Lexer::parseIdentifier() {
    size_t start_position = position;
    while (position < input_string.size()) {
        char c = peek();

        if (std::isalnum(c) || c == '_') {
            next();
        } else {
            break;
        }
    }

    std::string_view identifier_view = std::string_view(input_string).substr(start_position, position - start_position);
    
    return FixedString16(identifier_view);
}

std::array<SyntaxToken, 256> Lexer::tokenize() {
    std::array<SyntaxToken, 256> tokens;
    size_t token_index = 0;

    while (position < input_string.size()) {
        char c = peek();

        if (std::isspace(c)) {
            next();
            continue;
        } else if (std::isdigit(c)) {
            double number_value = parseNumber();
            tokens[token_index] = SyntaxToken{ SyntaxKind::NumberToken, number_value };
        } else if (std::isalnum(c) || c == '_') {
            FixedString16 identifier_string = parseIdentifier();
            tokens[token_index] = SyntaxToken{ SyntaxKind::VariableToken, identifier_string };
        } else {
            switch (c) {
                case '+':
                    tokens[token_index] = SyntaxToken{ SyntaxKind::PlusToken, std::monostate{} };
                    next();
                    break;
                case '-':
                    tokens[token_index] = SyntaxToken{ SyntaxKind::MinusToken, std::monostate{} };
                    next();
                    break;
                case '*':
                    tokens[token_index] = SyntaxToken{ SyntaxKind::MultiplyToken, std::monostate{} };
                    next();
                    break;
                case '/':
                    tokens[token_index] = SyntaxToken{ SyntaxKind::DivideToken, std::monostate{} };
                    next();
                    break;
                case '(':
                    tokens[token_index] = SyntaxToken{ SyntaxKind::OpenParenthesisToken, std::monostate{} };
                    next();
                    break;
                case ')':
                    tokens[token_index] = SyntaxToken{ SyntaxKind::CloseParenthesisToken, std::monostate{} };
                    next();
                    break;
                case '!':
                    tokens[token_index] = SyntaxToken{ SyntaxKind::FactorialToken, std::monostate{} };
                    next();
                    break;
                case '%':
                    tokens[token_index] = SyntaxToken{ SyntaxKind::PercentageToken, std::monostate{} };
                    next();
                    break;
                case ',':
                    tokens[token_index] = SyntaxToken{ SyntaxKind::CommaToken, std::monostate{} };
                    next();
                    break;
                default:
                    // Handle unknown character
                    next(); // Skip the unknown character
            }
        }

        token_index++;
    }

    tokens[token_index++] = SyntaxToken{ SyntaxKind::EndOfFileToken, std::monostate{} };

    return tokens;
}