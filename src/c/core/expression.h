#pragma once

#include "syntax_token.h"
#include <stddef.h>

#define MAX_ARGUMENTS 10

typedef enum {
    EXPRESSION_TYPE_NUMBER,
    EXPRESSION_TYPE_VARIABLE,
    EXPRESSION_TYPE_UNARY,
    EXPRESSION_TYPE_POSTFIX,
    EXPRESSION_TYPE_BINARY,
    EXPRESSION_TYPE_CALL
} ExpressionType;

typedef struct {
    ExpressionType type;
    union {
        double number;
        char* variable;
        struct {
            SyntaxToken operator;
            size_t operand_index;
        } unary;
        struct {
            SyntaxToken operator;
            size_t operand_index;
        } postfix;
        struct {
            SyntaxToken operator;
            size_t left_index;
            size_t right_index;
        } binary;
        struct {
            char* identifier;
            size_t argument_indices[MAX_ARGUMENTS]; // support up to 10 arguments for function calls
            size_t argument_count;
        } call;
    } value;
} Expression;