#include "parser.h"

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "syntax_token.h"
#include "syntax_kind.h"
#include "expression.h"

size_t parse_expression(Parser* parser, Expression* out, uint8_t rbp);
size_t nud(Parser* parser, Expression* out, SyntaxToken* token);
size_t led(Parser* parser, Expression* out, size_t left, SyntaxToken* token);

static const uint8_t UNARY_BP = 30;

static uint8_t get_binding_power(SyntaxToken* token) {
    switch (token->kind) {
        case SYNTAX_KIND_NUMBER:
        case SYNTAX_KIND_VARIABLE:
        case SYNTAX_KIND_CLOSE_PARENTHESIS:
        case SYNTAX_KIND_COMMA:
            return 0;
        case SYNTAX_KIND_PLUS:
        case SYNTAX_KIND_MINUS:
            return 10;
        case SYNTAX_KIND_MULTIPLY:
        case SYNTAX_KIND_DIVIDE:
            return 20;
        case SYNTAX_KIND_FACTORIAL:
        case SYNTAX_KIND_PERCENTAGE:
            return 40;
        default:
            fprintf(stderr, "Error: Unexpected token %d in getBindingPower\n", token->kind);
            return 0;
    }
}

static bool IMPLICIT_LEFT_TOKENS[SYNTAX_KIND_COUNT] = {
    [SYNTAX_KIND_NUMBER] = true,
    [SYNTAX_KIND_VARIABLE] = true,
    [SYNTAX_KIND_CLOSE_PARENTHESIS] = true,
    [SYNTAX_KIND_FACTORIAL] = true,
    [SYNTAX_KIND_PERCENTAGE] = true
};

static bool IMPLICIT_RIGHT_TOKENS[SYNTAX_KIND_COUNT] = {
    [SYNTAX_KIND_NUMBER] = true,
    [SYNTAX_KIND_VARIABLE] = true,
    [SYNTAX_KIND_OPEN_PARENTHESIS] = true
};

void init_parser(Parser* parser, SyntaxToken* tokens, size_t token_count) {
    parser->tokens = tokens;
    parser->token_count = token_count;
    parser->position = 0;
}

static SyntaxToken* peek(Parser* parser) {
    if (parser->position < parser->token_count) {
        return &parser->tokens[parser->position];
    } 

    static SyntaxToken eof_token = { .kind = SYNTAX_KIND_EOF };
    return &eof_token;
}

static SyntaxToken* consume(Parser* parser) {
    SyntaxToken* token = peek(parser);
    if (token->kind != SYNTAX_KIND_EOF) {
        parser->position++;
    }
    return token;
}

static size_t alloc_node(Parser* parser) {
    return parser->expression_count++;
}

size_t parse(Parser* parser, Expression* out, size_t max_expressions) {
    parser->expressions = out;
    parser->expression_count = 0;
    parser->max_expressions = max_expressions;

    size_t root = parse_expression(parser, out, 0);

    if (peek(parser)->kind != SYNTAX_KIND_EOF) {
        fprintf(stderr, "Error: Unexpected token after parsing complete expression: %d\n", peek(parser)->kind);
    }

    return root;
}

size_t parse_expression(Parser* parser, Expression* out, uint8_t rbp) {
    SyntaxToken* token = consume(parser);
    size_t left = nud(parser, out, token);

    while (1) {
        token = peek(parser);
        if (token->kind == SYNTAX_KIND_EOF) break;

        // check for implicit multiplication by checking if the previous token allows it
        // and the next token is one that can be implicitly multiplied
        // honestly if you just check for implicit tokens on the right it should be ok
        // but this way we can avoid some edge cases (i will think about this more later)
        bool is_implicit = parser->position > 0 &&
            IMPLICIT_LEFT_TOKENS[parser->tokens[parser->position - 1].kind] &&
            IMPLICIT_RIGHT_TOKENS[token->kind];

        if (is_implicit) {
            static const SyntaxToken implicit_multiply_token = { .kind = SYNTAX_KIND_MULTIPLY };
            token = (SyntaxToken*)&implicit_multiply_token;
        }
        if (get_binding_power(token) <= rbp) break;

        if (!is_implicit) token = consume(parser);

        left = led(parser, out, left, token);
    }

    return left;
}

size_t led(Parser* parser, Expression* out, size_t left, SyntaxToken* token) {
    switch (token->kind){
        case SYNTAX_KIND_PLUS:
        case SYNTAX_KIND_MINUS:
        case SYNTAX_KIND_MULTIPLY:
        case SYNTAX_KIND_DIVIDE: {
            size_t right = parse_expression(parser, out, get_binding_power(token));
            size_t node = alloc_node(parser);
            out[node].type = EXPRESSION_TYPE_BINARY;
            out[node].value.binary.operator = *token;
            out[node].value.binary.left_index = left;
            out[node].value.binary.right_index = right;
            return node;
        }
        case SYNTAX_KIND_FACTORIAL:
        case SYNTAX_KIND_PERCENTAGE: {
            size_t node = alloc_node(parser);
            out[node].type = EXPRESSION_TYPE_POSTFIX;
            out[node].value.postfix.operator = *token;
            out[node].value.postfix.operand_index = left;
            return node;
        }
        default: {
            fprintf(stderr, "Error: Unexpected token %d in led\n", token->kind);
            return left; // return the left node to avoid crashing, even though this is an error case
        }
    }
}

size_t nud(Parser* parser, Expression* out, SyntaxToken* token) {
    switch (token->kind) {
        case SYNTAX_KIND_NUMBER: {
            size_t node = alloc_node(parser);
            out[node].type = EXPRESSION_TYPE_NUMBER;
            out[node].value.number = token->value.number;
            return node;
        }
        case SYNTAX_KIND_VARIABLE: {
            // check function call
            if (peek(parser)->kind == SYNTAX_KIND_OPEN_PARENTHESIS) {
                consume(parser); // comsume '('
                size_t call_node = alloc_node(parser);
                out[call_node].type = EXPRESSION_TYPE_CALL;
                out[call_node].value.call.argument_count = 0;
                out[call_node].value.call.identifier = token->value.identifier;
                
                if (peek(parser)->kind != SYNTAX_KIND_CLOSE_PARENTHESIS) {
                    while (1) {
                        if (out[call_node].value.call.argument_count >= MAX_ARGUMENTS) {
                            fprintf(stderr, "Error: Too many arguments in function call, max is %zu\n", MAX_ARGUMENTS);
                            break;
                        }
                        
                        size_t argument_index = parse_expression(parser, out, 0);
                        out[call_node].value.call.argument_indices[out[call_node].value.call.argument_count++] = argument_index;

                        if (peek(parser)->kind != SYNTAX_KIND_COMMA) 
                            break;

                        consume(parser); // consume ','
                    }
                }
                                        
                if (peek(parser)->kind != SYNTAX_KIND_CLOSE_PARENTHESIS) {
                    fprintf(stderr, "Error: Expected ')' but got %d\n", peek(parser)->kind);
                }

                consume(parser); // consume ')'
                return call_node;
            }
            
            size_t node = alloc_node(parser);
            out[node].type = EXPRESSION_TYPE_VARIABLE;
            out[node].value.variable = token->value.identifier;
            return node;
        }
        case SYNTAX_KIND_PLUS:
        case SYNTAX_KIND_MINUS: {
            size_t node = alloc_node(parser);
            out[node].type = EXPRESSION_TYPE_UNARY;
            out[node].value.unary.operator = *token;
            out[node].value.unary.operand_index = parse_expression(parser, out, UNARY_BP);
            return node;
        }
        case SYNTAX_KIND_OPEN_PARENTHESIS: {
            size_t expression_index = parse_expression(parser, out, 0);
            if (peek(parser)->kind != SYNTAX_KIND_CLOSE_PARENTHESIS) {
                fprintf(stderr, "Error: Expected ')' but got %d\n", peek(parser)->kind);
            } else {
                consume(parser); // consume the close parenthesis
            }
            return expression_index;
        }
        default: {
            fprintf(stderr, "Error: Unexpected token %d in nud\n", token->kind);
            return 0; // return an empty node to avoid crashing, even though this is an error case
        }
    }
}

void print_AST(const Expression* exprs, size_t index, const char* prefix, uint8_t is_last) {
    const Expression* expr = &exprs[index];
    const char* connector = is_last ? "└── " : "├── ";
    char new_prefix[256];
    snprintf(new_prefix, sizeof(new_prefix), "%s%s", prefix, is_last ? "    " : "│   ");

    switch (expr->type) {
        case EXPRESSION_TYPE_NUMBER: {
            printf("%s%s%g\n", prefix, connector, expr->value.number);
            break;
        }
        case EXPRESSION_TYPE_VARIABLE: {
            printf("%s%s%s\n", prefix, connector, expr->value.variable);
            break;
        }
        case EXPRESSION_TYPE_UNARY: {
            printf("%s%s%d\n", prefix, connector, expr->value.unary.operator.kind);
            print_AST(exprs, expr->value.unary.operand_index, new_prefix, 1);
            break;
        }
        case EXPRESSION_TYPE_POSTFIX: {
            printf("%s%s%d\n", prefix, connector, expr->value.postfix.operator.kind);
            print_AST(exprs, expr->value.postfix.operand_index, new_prefix, 1);
            break;
        }
        case EXPRESSION_TYPE_BINARY: {
            printf("%s%s%d\n", prefix, connector, expr->value.binary.operator.kind);
            print_AST(exprs, expr->value.binary.left_index, new_prefix, 0);
            print_AST(exprs, expr->value.binary.right_index, new_prefix,1);
            break;
        }
        case EXPRESSION_TYPE_CALL: {
            printf("%s%s%s\n", prefix, connector, expr->value.call.identifier);
            for (size_t i = 0; i < expr->value.call.argument_count; i++) {
                print_AST(exprs, expr->value.call.argument_indices[i], new_prefix, i + 1 == expr->value.call.argument_count);
            }
            break;
        }
        default: {
            printf("%s%sUnknown node type\n", prefix, connector);
            break;
        }
    }
}