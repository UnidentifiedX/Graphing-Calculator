#pragma once

#include <stddef.h>
#include "syntax_token.h"

typedef struct {
    const char* _input_string;
    size_t _input_length;
    size_t _position;
} Lexer;

/// @brief Fill an array of SyntaxToken with tokens from the input string in the Lexer.
/// @param lexer 
/// @param tokens 
/// @param max_tokens should be 256
/// @return Number of tokens filled
size_t tokenize(Lexer* lexer, SyntaxToken* tokens, size_t max_tokens);