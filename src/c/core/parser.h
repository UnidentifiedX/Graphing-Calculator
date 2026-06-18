#pragma once

#include "syntax_token.h"
#include "expression.h"
#include <stddef.h>
#include <stdint.h>

typedef struct {
    SyntaxToken* _tokens;
    size_t _token_count;
    size_t _position;

    Expression* _expressions;
    size_t _expression_count;
    size_t _max_expressions;
} Parser;

/// @brief Parses a sequence of SyntaxToken into an array of Expression. First expression is the root expression
/// @param parser 
/// @param expressions 
/// @param max_expressions should be 256
/// @return Index of the root expression in the expressions array
size_t parse(Parser* parser, Expression* out, size_t max_expressions);
void print_AST(const Expression* exprs, size_t index, const char* prefix, uint8_t is_last);