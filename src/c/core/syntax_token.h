#pragma once
#include "syntax_kind.h"

/// @brief If there is an identifier, it is limited to 15 characters + "\0"
typedef struct {
    SyntaxKind kind;
    union {
        double number;
        char identifier[16];
    } value;
} SyntaxToken;