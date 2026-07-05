#pragma once

#include "core/syntax_token.h"
#include <stddef.h>
#include <stdbool.h>

#define MAX_TOKEN_EXPRESSION_BUFFER_SIZE 256
#define MAX_NUMBER_STRING_LENGTH 128
#define MAX_FUNCTION_IDENTIFIER_LENGTH 16
#define MAX_VARIABLE_IDENTIFIER_LENGTH 16

typedef enum {
    NODE_TYPE_EMPTY,
    NODE_TYPE_NUMBER,
    NODE_TYPE_ATOM,
    NODE_TYPE_VARIABLE, // behaves just like an atom
    NODE_TYPE_FUNCTION,
    NODE_TYPE_END_OF_ARGUMENT
} NodeType;

typedef struct {
    NodeType type;
    union {
        char num_str[MAX_NUMBER_STRING_LENGTH];
        SyntaxToken token;
        char func_identifier[MAX_FUNCTION_IDENTIFIER_LENGTH];
        char var_identifier[MAX_VARIABLE_IDENTIFIER_LENGTH];
    } node;
} Node;

typedef struct {
    size_t node_index;
    size_t char_index;
    Node node_buffer[MAX_TOKEN_EXPRESSION_BUFFER_SIZE];
    size_t size;
} TokenExpression;

typedef enum {
    CURSOR_DIRECTION_LEFT,
    CURSOR_DIRECTION_RIGHT
} CursorDirection;

void init_token_expression(TokenExpression* expr);
/// @brief Shift the cursor in the given direction 
///
/// This function doesn't care if the current char_index is valid or not; as long as the node_index is valid, it will shift the cursor in the intended direction and then fix the char_index accordingly based on the new node_index
/// @param expr 
/// @param direction 
/// @return Whether the cursor was successfully shifted
bool shift_cursor(TokenExpression* expr, CursorDirection direction);
void insert_node(TokenExpression* expr, Node* node);
void delete_node(TokenExpression* expr);
void backspace_node(TokenExpression* expr);
/// @brief convert the token expression to an array of syntax tokens
/// @param expr 
/// @param tokens 
/// @param max_tokens 
/// @return number of tokens written to the tokens array
size_t to_tokens(TokenExpression* expr, SyntaxToken* tokens, size_t max_tokens);
bool is_special_display_function(char* identifier);
void reset_token_expression(TokenExpression* expr);

void output_token_expression(const TokenExpression* expr);