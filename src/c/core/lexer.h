#pragma once

#include <stddef.h>
#include "syntax_token.h"

typedef struct {
    const char* input_string;
    size_t input_length;
    size_t position;
} Lexer;

void init_lexer(Lexer* lexer, const char* input_string);

/// @brief Fill an array of SyntaxToken with tokens from the input string in the Lexer.
/// @param lexer 
/// @param tokens 
/// @param max_tokens should be 256
/// @return Number of tokens filled
size_t tokenize(Lexer* lexer, SyntaxToken* tokens, size_t max_tokens);