#include "lexer.h"
#include <stdbool.h>
#include <stdio.h>
#include <ctype.h>
#include <stddef.h>

static const char peek(Lexer* lexer) {
    if (lexer->_position < lexer->_input_length) {
        return lexer->_input_string[lexer->_position];
    }

    return '\0';
}

static char next(Lexer* lexer) {
    if (lexer->_position < lexer->_input_length) {
        char c = lexer->_input_string[lexer->_position];
        lexer->_position++;
        return c;
    }

    return '\0';
}

static double parse_number(Lexer* lexer) {
    double value = 0.0;
    double decimal_factor = 1.0;
    bool has_digits = false;
    bool has_decimal_point = false;

    while (lexer->_position < lexer->_input_length) {
        char c = peek(lexer);

        if (isdigit(c)) {
            has_digits = true;
            if (!has_decimal_point) { // build the integer part
                value = value * 10 + (c - '0'); // '5' - '0' = 5
            } else {
                decimal_factor *= 0.1; // build the fractional part
                value += (c - '0') * decimal_factor;
            }
            next(lexer);
        } else if (c == '.' && !has_decimal_point) {
            has_decimal_point = true;
            next(lexer);
        } else {
            break;
        }
    }

    if (!has_digits) {
        fprintf(stderr, "Error: Expected a number at position %zu\n", lexer->_position);
        return 0.0;
    }

    return value;
}

static void parse_identifier(Lexer* lexer, char* buffer, size_t buffer_size) {
    size_t length = 0;

    while (lexer->_position < lexer->_input_length && (isalnum(peek(lexer)) || peek(lexer) == '_')) {
        if (length < buffer_size - 1) { // Leave space for null terminator
            buffer[length++] = next(lexer);
        } else {
            fprintf(stderr, "Error: Identifier exceeds maximum length of %zu characters\n", buffer_size - 1);
            break;
        }
    }

    buffer[length] = '\0'; // Null-terminate the string
}

size_t tokenize(Lexer* lexer, SyntaxToken* tokens, size_t max_tokens) {
    size_t token_index = 0;

    while (lexer->_position < lexer->_input_length && token_index < max_tokens) {
        char c = peek(lexer);

        if (isspace(c)) {
            next(lexer); // Skip whitespace
            continue;
        } else if (isdigit(c)) {
            double number = parse_number(lexer);
            tokens[token_index].kind = SYNTAX_KIND_NUMBER;
            tokens[token_index].value.number = number;
        } else if (isalnum(c) || c == '_') {
            parse_identifier(lexer, 
                tokens[token_index].value.identifier, 
                sizeof(tokens[token_index].value.identifier)
            );
            tokens[token_index].kind = SYNTAX_KIND_VARIABLE;
        } else {
            switch (c) {
                case '+':
                    tokens[token_index].kind = SYNTAX_KIND_PLUS;
                    next(lexer);
                    break;
                case '-':
                    tokens[token_index].kind = SYNTAX_KIND_MINUS;
                    next(lexer);
                    break;
                case '*':
                    tokens[token_index].kind = SYNTAX_KIND_MULTIPLY;
                    next(lexer);
                    break;
                case '/':
                    tokens[token_index].kind = SYNTAX_KIND_DIVIDE;
                    next(lexer);
                    break;
                case '(':
                    tokens[token_index].kind = SYNTAX_KIND_OPEN_PARENTHESIS;
                    next(lexer);
                    break;
                case ')':
                    tokens[token_index].kind = SYNTAX_KIND_CLOSE_PARENTHESIS;
                    next(lexer);
                    break;
                case ',':
                    tokens[token_index].kind = SYNTAX_KIND_COMMA;
                    next(lexer);
                    break;
                case '!':
                    tokens[token_index].kind = SYNTAX_KIND_FACTORIAL;
                    next(lexer);
                    break;
                case '%':
                    tokens[token_index].kind = SYNTAX_KIND_PERCENTAGE;
                    next(lexer);
                    break;
                default:
                    fprintf(stderr, "Error: Unrecognized character '%c' at position %zu\n", c, lexer->_position);
                    next(lexer); // Skip the unrecognized character
            }
        }

        token_index++;
    }

    if (token_index < max_tokens) {
        tokens[token_index++].kind = SYNTAX_KIND_EOF;
    }

    return token_index;
}