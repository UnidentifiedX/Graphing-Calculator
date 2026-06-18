#pragma once

typedef enum {
    SYNTAX_KIND_NUMBER,            // 0
    SYNTAX_KIND_VARIABLE,          // 1
    SYNTAX_KIND_PLUS,              // 2
    SYNTAX_KIND_MINUS,             // 3
    SYNTAX_KIND_MULTIPLY,          // 4
    SYNTAX_KIND_DIVIDE,            // 5
    SYNTAX_KIND_OPEN_PARENTHESIS,  // 6
    SYNTAX_KIND_CLOSE_PARENTHESIS, // 7
    SYNTAX_KIND_COMMA,             // 8
    SYNTAX_KIND_FACTORIAL,         // 9
    SYNTAX_KIND_PERCENTAGE,        // 10
    SYNTAX_KIND_EOF,               // 11
    SYNTAX_KIND_COUNT // to keep track of the number of syntax kinds, not an actual syntax kind
} SyntaxKind;