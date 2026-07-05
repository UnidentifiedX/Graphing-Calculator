#include "token_expression.h"
#include "core/inbuilt_function.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/*
Logic for the entire token expression editor now that i wrote working spaghetti but it's time for a refactor

The main idea is that, from now on, ANY operation on the buffer operates on the node to the right of the cursor, for both the node and number buffer. 
What this means is that there is no such thing as a cursor at the end of a number node; 
the cursor instead belongs to the next node, which is None if the cursor is at the end of the entire expression.

From now on there will only be one check: is_between_nodes. This is a more confusion invariant, because this encompasses being at the start of a
number OR being at an atomic/function node. If we want to check if the current node is a number node just check the type

This goes for deletion as well. It only deletes the current node, so if we want to delete the previous node, shift left then delete.
This means we need to hard-code some special rules though but it makes the logic much simpler

We also ENFORCE that by the end of all insertion operations, the cursor is ALWAYS one position before the the new intended cursor position
So it's kinda like we've only ever been working on the node before the new cursor position
*/

typedef enum {
    STRING_MERGE_PREPEND,
    STRING_MERGE_APPEND
} StringMergeDirection;

bool is_special_display_function(char* identifier) {
    InbuiltFunction func = get_inbuilt_function_from_enum(identifier);

    return func == INBUILT_FUNCTION_SQRT ||
            func == INBUILT_FUNCTION_NROOT ||
            func == INBUILT_FUNCTION_POW ||
            func == INBUILT_FUNCTION_EXP ||
            func == INBUILT_FUNCTION_FRAC;
}

static bool is_between_nodes(TokenExpression* expr);
static void merge_number_strings(char* dst, const char* src, StringMergeDirection direction);
static bool can_merge_number_strings(const char* dst, const char* src);
static void shift_tokens_right(TokenExpression* expr, size_t index, size_t places);
static void shift_tokens_left(TokenExpression* expr, size_t index, size_t places);
static void insert_node_atomically(TokenExpression* expr, Node* node);
static void insert_atomic_node_within_number(TokenExpression* expr, Node* current_node, Node* node);
static void delete_node_atomically(TokenExpression* expr);

void init_token_expression(TokenExpression* expr) {
    expr->node_index = 0;
    expr->char_index = __SIZE_MAX__;
    expr->size = 0;

    for (size_t i = 0; i < MAX_TOKEN_EXPRESSION_BUFFER_SIZE; i++) {
        expr->node_buffer[i].type = NODE_TYPE_EMPTY;
    }
}

bool shift_cursor(TokenExpression* expr, CursorDirection direction) {
    Node* current_node = &expr->node_buffer[expr->node_index];
    bool is_between = is_between_nodes(expr);

    if (direction == CURSOR_DIRECTION_RIGHT) {
        if (current_node->type == NODE_TYPE_EMPTY)
            return false;
        if (is_between && expr->node_index == MAX_TOKEN_EXPRESSION_BUFFER_SIZE - 1)
            return false; // can't shift right if we're at the end of the buffer

        if (is_between && current_node->type != NODE_TYPE_NUMBER) {
            Node* next_node = &expr->node_buffer[expr->node_index + 1];
            if (next_node->type == NODE_TYPE_NUMBER) {
                expr->node_index++;
                expr->char_index = 0;
            } else {
                expr->node_index++;
                expr->char_index = __SIZE_MAX__;
            }
        } else if (current_node->type == NODE_TYPE_NUMBER) {
            expr->char_index++;
            if (expr->char_index >= strlen(current_node->node.num_str)) {
                expr->char_index = __SIZE_MAX__;
                expr->node_index++;
            }
        } else {
            // this means the buffer is in an intermediate invalid state because of some shifting but we can move the cursor right
            expr->node_index++;
            expr->char_index = expr->node_buffer[expr->node_index].type == NODE_TYPE_NUMBER ? 0 : __SIZE_MAX__;
        }
    } else if (direction == CURSOR_DIRECTION_LEFT) {
        if (expr->node_index == 0 && is_between)
            return false; // can't shift left if we're at the start of the buffer
        if (is_between) {
            Node* previous_node = &expr->node_buffer[expr->node_index - 1];
            if (previous_node->type == NODE_TYPE_NUMBER) {
                expr->node_index--;
                expr->char_index = strlen(previous_node->node.num_str) - 1;
            } else {
                expr->node_index--;
                expr->char_index = __SIZE_MAX__;
            }
        } else if (current_node->type == NODE_TYPE_NUMBER) {
            expr->char_index--;
        }
    }
    
    return true;
}

void insert_node(TokenExpression* expr, Node* node) {
    Node* current_node = &expr->node_buffer[expr->node_index];
    bool is_between = is_between_nodes(expr);
    
    size_t space_required = 1;
    if (node->type == NODE_TYPE_NUMBER && current_node->type == NODE_TYPE_NUMBER)
        space_required = 0; // if both the new node and the current node are numbers, we can just merge them without needing to shift anything in the buffer, so no additional space is required
    else if (!is_between && node->type != NODE_TYPE_NUMBER)
        space_required = 2; // in a number and inserting a non-number node, need to split the number node into two and insert the new node in between
    else if (node->type == NODE_TYPE_FUNCTION && is_special_display_function(node->node.func_identifier))
        space_required += get_function_arity(get_inbuilt_function_from_enum(node->node.func_identifier)); // function node + end of argument nodes

    if (expr->size + space_required > MAX_TOKEN_EXPRESSION_BUFFER_SIZE) {
        fprintf(stderr, "Error: Cannot insert node, buffer overflow\n");
        return;
    }

    if (current_node->type == NODE_TYPE_EMPTY) {
        if (node->type == NODE_TYPE_FUNCTION && is_special_display_function(node->node.func_identifier)) {
            size_t arity = get_function_arity(get_inbuilt_function_from_enum(node->node.func_identifier));
            expr->node_buffer[expr->node_index] = *node;
            
            for (size_t i = 0; i < arity; i++) {
                expr->node_buffer[expr->node_index + 1 + i].type = NODE_TYPE_END_OF_ARGUMENT;
            }
            expr->size += (1 + arity);
        } else {
            if (node->type == NODE_TYPE_NUMBER && expr->node_index > 0) {
                Node* previous_node = &expr->node_buffer[expr->node_index - 1];
                if (previous_node->type == NODE_TYPE_NUMBER) {
                    merge_number_strings(previous_node->node.num_str, node->node.num_str, STRING_MERGE_APPEND);
                    return;
                }
            }

            expr->node_buffer[expr->node_index] = *node;
            expr->size++;

            if (node->type == NODE_TYPE_NUMBER) 
                expr->char_index = 0;
        }

        shift_cursor(expr, CURSOR_DIRECTION_RIGHT); // shift cursor to the right of the newly inserted node
        return;        
    }

    bool between_nodes = is_between_nodes(expr);

    switch (node->type) {
        case NODE_TYPE_NUMBER: {
            if (between_nodes) {
                Node* previous_node = (expr->node_index > 0) ? &expr->node_buffer[expr->node_index - 1] : NULL;

                if (previous_node && previous_node->type == NODE_TYPE_NUMBER) {
                    merge_number_strings(previous_node->node.num_str, node->node.num_str, STRING_MERGE_APPEND);
                    return; // cursor remains in the same position
                } else if (current_node->type == NODE_TYPE_NUMBER) { // check if current node is a number if it is just prepend
                    merge_number_strings(current_node->node.num_str, node->node.num_str, STRING_MERGE_PREPEND);
                } else {
                    insert_node_atomically(expr, node);
                    expr->char_index = 0;
                }
            } else {
                // new_num_str = num_str[:self.char_index] + node.num_str + num_str[self.char_index:]
                if (!can_merge_number_strings(current_node->node.num_str, node->node.num_str)) {
                    fprintf(stderr, "Error: Cannot merge number strings, resulting string would exceed maximum length\n");
                    return;
                }

                char new_num_str[MAX_NUMBER_STRING_LENGTH];
                size_t current_length = strlen(current_node->node.num_str);
                size_t src_length = strlen(node->node.num_str);
                memcpy(new_num_str, current_node->node.num_str, expr->char_index);
                memcpy(new_num_str + expr->char_index, node->node.num_str, src_length);
                memcpy(new_num_str + expr->char_index + src_length, current_node->node.num_str + expr->char_index, current_length - expr->char_index);
                new_num_str[current_length + src_length] = '\0';
                strcpy(current_node->node.num_str, new_num_str);
            }
            break;
        }
        case NODE_TYPE_ATOM: {
            if (between_nodes) {
                insert_node_atomically(expr, node);
            } else {
                insert_atomic_node_within_number(expr, current_node, node);
            }
            break;
        }
        case NODE_TYPE_FUNCTION: {
            if (between_nodes) {
                if (is_special_display_function(node->node.func_identifier)) {
                    size_t arity = get_function_arity(get_inbuilt_function_from_enum(node->node.func_identifier));

                    shift_tokens_right(expr, expr->node_index, arity + 1);
                    expr->node_buffer[expr->node_index] = *node;
                    for (size_t i = 0; i < arity; i++) {
                        expr->node_buffer[expr->node_index + 1 + i].type = NODE_TYPE_END_OF_ARGUMENT;
                    }
                    expr->size += (arity + 1);
                } else {
                    insert_node_atomically(expr, node);
                }
            } else {
                if (is_special_display_function(node->node.func_identifier)) {
                    size_t arity = get_function_arity(get_inbuilt_function_from_enum(node->node.func_identifier));
                    
                    char* current_num_str = current_node->node.num_str;
                    char left_num_str[MAX_NUMBER_STRING_LENGTH];
                    memcpy(left_num_str, current_num_str, expr->char_index);
                    left_num_str[expr->char_index] = '\0';
                    char right_num_str[MAX_NUMBER_STRING_LENGTH];
                    memcpy(right_num_str, current_num_str + expr->char_index, strlen(current_num_str) - expr->char_index);
                    right_num_str[strlen(current_num_str) - expr->char_index] = '\0';
                    strcpy(current_node->node.num_str, left_num_str); // current number node is the left part of the split number
                    
                    expr->node_index++; // move to the right of the current number node
                    shift_tokens_right(expr, expr->node_index, arity + 2); // arity + function node + right number node
                    expr->node_buffer[expr->node_index] = *node; // insert the function node
                    for (size_t i = 0; i < arity; i++) {
                        expr->node_buffer[expr->node_index + 1 + i].type = NODE_TYPE_END_OF_ARGUMENT;
                    }
                    Node* right_number_node = &expr->node_buffer[expr->node_index + arity + 1];
                    right_number_node->type = NODE_TYPE_NUMBER;
                    strcpy(right_number_node->node.num_str, right_num_str);
                    expr->size += (arity + 2);
                } else {
                    insert_atomic_node_within_number(expr, current_node, node);
                }
            }
            break;
        }
    }

    shift_cursor(expr, CURSOR_DIRECTION_RIGHT); // shift cursor to the right to reset all positional invariants
}

void delete_node(TokenExpression* expr) {
    Node* current_node = &expr->node_buffer[expr->node_index];

    if (expr->size == 0)
        return; // nothing to delete
    if (current_node->type == NODE_TYPE_EMPTY)
        return; // nothing to delete

    bool is_between = is_between_nodes(expr);
    if (is_between) {
        if (current_node->type == NODE_TYPE_NUMBER && strlen(current_node->node.num_str) != 1) {
            memmove(current_node->node.num_str, current_node->node.num_str + 1, strlen(current_node->node.num_str)); // delete the first character of the number string
        } else if (current_node->type == NODE_TYPE_FUNCTION) {
            if (is_special_display_function(current_node->node.func_identifier)) {
                size_t arity = get_function_arity(get_inbuilt_function_from_enum(current_node->node.func_identifier));
                size_t original_node_index = expr->node_index;
                size_t depth_counter = 0;
                size_t eoa_delete_count = 0;
                size_t i = expr->node_index + 1;

                while (i < expr->size && eoa_delete_count < arity) {
                    Node *node = &expr->node_buffer[i];
                    if (node->type == NODE_TYPE_FUNCTION && is_special_display_function(node->node.func_identifier)) {
                        depth_counter += get_function_arity(get_inbuilt_function_from_enum(node->node.func_identifier));
                    } else if (node->type == NODE_TYPE_END_OF_ARGUMENT) {
                        if (depth_counter == 0) {
                            expr->node_index = i;
                            delete_node_atomically(expr);
                            eoa_delete_count++;
                            continue; // basically once everything is shifted we can't update the node index just continue from here
                        }

                        depth_counter--;
                    }

                    i++;
                }

                if (eoa_delete_count < arity) {
                    fprintf(stderr, "Error: Not enough end of argument nodes to delete for function '%s'\n", current_node->node.func_identifier);
                }

                expr->node_index = original_node_index;
                delete_node_atomically(expr);
            } else {
                delete_node_atomically(expr);
            }
        } else if (current_node->type == NODE_TYPE_END_OF_ARGUMENT) {
            /*
            took inspirtaiton from the fx-97sgx where deleting the very last EOA instead deleted the character before it
            but deleting any other EOA deletes the whole function, leaving the arguments there
            but because im lazy and because it's going to take forever to figure out the logic for placing the cursor, the cursor will now just be returned to the
            start of the function and the function will be deleted
            */
            int parent_function_node_index = -1;
            bool is_last_argument = false;
            int depth = 0;

            for (size_t i = expr->node_index; i-- > 0;) {
                Node* node = &expr->node_buffer[i];

                if (node->type == NODE_TYPE_END_OF_ARGUMENT) {
                    depth++;
                } else if (node->type == NODE_TYPE_FUNCTION && is_special_display_function(node->node.func_identifier)) {
                    size_t arity = get_function_arity(get_inbuilt_function_from_enum(node->node.func_identifier));
                    depth -= (int)arity;

                    if (depth <= 0) {
                        parent_function_node_index = (int)i;

                        if (depth == 0) {
                            is_last_argument = true;
                        }

                        break;
                    }
                }
            }

            if (parent_function_node_index == -1) {
                fprintf(stderr, "Error: Could not find parent function node for end of argument node at index %zu\n", expr->node_index);
                return;
            }

            if (is_last_argument) {
                backspace_node(expr);
            } else {
                expr->node_index = (size_t)parent_function_node_index;
                delete_node(expr);
            }
        } else {
            delete_node_atomically(expr);
        }
    } else {
        char right[MAX_NUMBER_STRING_LENGTH];
        size_t current_node_length = strlen(current_node->node.num_str);
        size_t right_length = current_node_length - expr->char_index - 1;
        memcpy(right, current_node->node.num_str + expr->char_index + 1, right_length);
        right[right_length] = '\0';
        char left[MAX_NUMBER_STRING_LENGTH];
        memcpy(left, current_node->node.num_str, expr->char_index);
        left[expr->char_index] = '\0';

        char new_num_str[MAX_NUMBER_STRING_LENGTH];
        strcpy(new_num_str, left);
        strcat(new_num_str, right);
        strcpy(current_node->node.num_str, new_num_str);

        if (right_length == 0) {
            shift_cursor(expr, CURSOR_DIRECTION_RIGHT);
        }
    }
}

void backspace_node(TokenExpression* expr) {
    if (shift_cursor(expr, CURSOR_DIRECTION_LEFT)) {
        delete_node(expr);
    }
}

size_t to_tokens(TokenExpression* expr, SyntaxToken* tokens, size_t max_tokens) {
    size_t token_count = 0;
    size_t arity_stack[32]; // stack to keep track of function arities
    size_t arity_stack_size = 0;

    for (size_t i = 0; i < expr->size && token_count < max_tokens; i++) {
        Node* node = &expr->node_buffer[i];

        switch (node->type) {
            case NODE_TYPE_NUMBER: {
                tokens[token_count++] = (SyntaxToken){ .kind = SYNTAX_KIND_NUMBER, .value.number = strtod(node->node.num_str, NULL) };
                break;
            }
            case NODE_TYPE_ATOM: {
                tokens[token_count++] = node->node.token;
                break;
            }
            case NODE_TYPE_FUNCTION: {
                tokens[token_count++] = (SyntaxToken){ .kind = SYNTAX_KIND_VARIABLE, .value.identifier = *node->node.func_identifier };
                tokens[token_count++] = (SyntaxToken){ .kind = SYNTAX_KIND_OPEN_PARENTHESIS };

                if (is_special_display_function(node->node.func_identifier)) {
                    size_t arity = get_function_arity(get_inbuilt_function_from_enum(node->node.func_identifier));
                    arity_stack[arity_stack_size++] = arity;
                }
                break;
            }
            case NODE_TYPE_END_OF_ARGUMENT: {
                if (arity_stack_size == 0) {
                    fprintf(stderr, "Error: Mismatched end of argument node without corresponding function\n");
                    return token_count;
                }

                arity_stack[arity_stack_size - 1]--;

                if (arity_stack[arity_stack_size - 1] > 0) {
                    tokens[token_count++] = (SyntaxToken){ .kind = SYNTAX_KIND_COMMA };
                } else {
                    tokens[token_count++] = (SyntaxToken){ .kind = SYNTAX_KIND_CLOSE_PARENTHESIS };
                    arity_stack_size--;
                }
            }
        }
    }

    return token_count;
}

void reset_token_expression(TokenExpression* expr) {
    expr->node_index = 0;
    expr->char_index = __SIZE_MAX__;
    expr->size = 0;

    for (size_t i = 0; i < MAX_TOKEN_EXPRESSION_BUFFER_SIZE; i++) {
        expr->node_buffer[i].type = NODE_TYPE_EMPTY;
    }
}

// at the start of a number or at an atomic/function node
static bool is_between_nodes(TokenExpression* expr) {
    return expr->char_index == __SIZE_MAX__ || expr->char_index == 0;
}

static void shift_tokens_right(TokenExpression* expr, size_t index, size_t places) {
    if (expr->size + places > MAX_TOKEN_EXPRESSION_BUFFER_SIZE) {
        fprintf(stderr, "Error: Cannot shift tokens right, buffer overflow\n");
        return;
    }

    size_t i = expr->size;
    while (i-- > index) {
        expr->node_buffer[i + places] = expr->node_buffer[i];
    }

    // Clear the newly created empty nodes at the specified index
    for (size_t j = index; j < index + places; j++) {
        expr->node_buffer[j].type = NODE_TYPE_EMPTY;
    }
}

static void shift_tokens_left(TokenExpression* expr, size_t index, size_t places) {
    if (index < places) {
        fprintf(stderr, "Error: Cannot shift tokens left, index out of bounds\n");
        return;
    }

    for (size_t i = index; i < expr->size; i++) {
        expr->node_buffer[i - places] = expr->node_buffer[i];
    }

    // Clear the now-unused nodes at the end of the buffer
    for (size_t i = expr->size - places; i < expr->size; i++) {
        expr->node_buffer[i].type = NODE_TYPE_EMPTY;
    }
}

// shift everything to the right and insert the node at the current index
static void insert_node_atomically(TokenExpression* expr, Node* node) {
    if (expr->size >= MAX_TOKEN_EXPRESSION_BUFFER_SIZE) {
        fprintf(stderr, "Error: Cannot insert node, buffer overflow\n");
        return;
    }

    shift_tokens_right(expr, expr->node_index, 1);
    expr->node_buffer[expr->node_index] = *node;
    expr->size++;
    expr->char_index = __SIZE_MAX__; // reset char_index to indicate we're between nodes
}

static void insert_atomic_node_within_number(TokenExpression* expr, Node* current_node, Node* new_node) {
    size_t node_index = expr->node_index;
    size_t char_index = expr->char_index;

    char* current_num_str = current_node->node.num_str;
    size_t current_length = strlen(current_num_str);

    char right_num_str[MAX_NUMBER_STRING_LENGTH];
    memcpy(right_num_str, current_num_str + char_index, current_length - char_index);
    right_num_str[current_length - char_index] = '\0';

    current_num_str[char_index] = '\0'; // truncate the current number string

    Node right_number_node;
    right_number_node.type = NODE_TYPE_NUMBER;
    memcpy(right_number_node.node.num_str, right_num_str, strlen(right_num_str) + 1);

    shift_tokens_right(expr, expr->node_index + 1, 2); // shift all nodes two positions to the right
    expr->node_buffer[expr->node_index + 1] = *new_node; // insert the new node
    expr->node_buffer[expr->node_index + 2] = right_number_node;
    expr->size += 2;
    expr->node_index++; // move the cursor to the new node
}

static void delete_node_atomically(TokenExpression* expr) {
    if (expr->size == 0 || expr->node_index >= expr->size) {
        fprintf(stderr, "Error: Cannot delete node, buffer underflow or invalid index\n");
        return;
    }

    shift_tokens_left(expr, expr->node_index + 1, 1);
    expr->node_buffer[expr->size - 1].type = NODE_TYPE_EMPTY; // clear the last node
    expr->size--;
    
    Node* new_current_node = &expr->node_buffer[expr->node_index];
    Node* previous_node = (expr->node_index > 0) ? &expr->node_buffer[expr->node_index - 1] : NULL;

    if (new_current_node->type == NODE_TYPE_NUMBER) {
        if (previous_node && previous_node->type == NODE_TYPE_NUMBER) {
            merge_number_strings(previous_node->node.num_str, new_current_node->node.num_str, STRING_MERGE_APPEND);
            shift_tokens_left(expr, expr->node_index, 1);
            expr->size--;
            expr->node_index--; // move the cursor to the previous node
            expr->char_index = strlen(previous_node->node.num_str); // set char_index to the end of the merged number
        } else {
            expr->char_index = 0;
        }
    } else {
        expr->char_index = __SIZE_MAX__; // reset char_index to indicate we're between nodes
    }
}

static bool can_merge_number_strings(const char* dst, const char* src) {
    return (strlen(dst) + strlen(src)) < MAX_NUMBER_STRING_LENGTH;
}

// just does a check and merges the string nothing else
static void merge_number_strings(char* dst, const char* src, StringMergeDirection direction) {
    if (can_merge_number_strings(dst, src)) {
        if (direction == STRING_MERGE_PREPEND) {
            memmove(dst + strlen(src), dst, strlen(dst) + 1);
            memcpy(dst, src, strlen(src));
        } else {
            strcat(dst, src);
        }
    } else {
        fprintf(stderr, "Error: Cannot merge number strings, resulting string would exceed maximum length\n");
    }
}

void output_token_expression(const TokenExpression* expr) {
    printf("Token Expression (size: %zu):\n", expr->size);
    for (size_t i = 0; i < expr->size; i++) {
        const Node* node = &expr->node_buffer[i];
        printf("Node %zu: Type: %d, ", i, node->type);
        switch (node->type) {
            case NODE_TYPE_NUMBER:
                printf("Number: %s\n", node->node.num_str);
                break;
            case NODE_TYPE_ATOM:
                printf("Atom: %d\n", node->node.token.kind);
                break;
            case NODE_TYPE_FUNCTION:
                printf("Function: %s\n", node->node.func_identifier);
                break;
            case NODE_TYPE_END_OF_ARGUMENT:
                printf("End of Argument\n");
                break;
            default:
                printf("Empty or Unknown Node\n");
                break;
        }
    }
}