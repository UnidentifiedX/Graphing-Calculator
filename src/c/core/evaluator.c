#include "evaluator.h"
#include "parser.h"
#include "expression.h"
#include <math.h>
#include <string.h>
#include <stdio.h>
#include "inbuilt_functions.h"
#include "inbuilt_constants.h"

double call_inbuilt_function(InbuiltFunction func, double* args, size_t argc) {
    switch (func) {
        case INBUILT_FUNCTION_SIN:
            return sin(args[0]);
        case INBUILT_FUNCTION_COS:
            return cos(args[0]);
        case INBUILT_FUNCTION_TAN:
            return tan(args[0]);
        case INBUILT_FUNCTION_LN:
            return log(args[0]);
        case INBUILT_FUNCTION_LOG:
            return log10(args[0]);
        case INBUILT_FUNCTION_SQRT:
            return sqrt(args[0]);
        case INBUILT_FUNCTION_POW:
            return pow(args[0], args[1]);
        case INBUILT_FUNCTION_ABS:
            return fabs(args[0]);
        case INBUILT_FUNCTION_NRT:
            if (args[0] == 0) {
                fprintf(stderr, "Error: Cannot take the 0th root\n");
                return NAN;
            }
            return pow(args[1], 1.0 / args[0]);
        case INBUILT_FUNCTION_FRAC:
            if (args[1] == 0) {
                fprintf(stderr, "Error: Division by zero in fraction\n");
                return NAN;
            }
            return args[0] / args[1];
        default:
            fprintf(stderr, "Error: Unknown inbuilt function\n");
            return NAN;
    }
}

double get_variable_value(const char* name, const VariableEnvironment* env) {
    for (size_t i = 0; i < env->count; i++) {
        if (strcmp(env->variables[i].name, name) == 0) {
            return env->variables[i].value;
        }
    }
    fprintf(stderr, "Error: Undefined variable '%s'\n", name);
    return NAN;
}

double evaluate(const Expression* expressions, size_t index, const VariableEnvironment* env) {
    const Expression* expression = &expressions[index];

    switch (expression->type) {
        case EXPRESSION_TYPE_NUMBER:
            return expression->value.number;
        case EXPRESSION_TYPE_VARIABLE: {
            InbuiltConstant constant = get_constant_from_name(expression->value.variable);
            double constant_value = get_constant(constant);
            
            if (!isnan(constant_value)) return constant_value; // inbuilt constant

            double variable_value = get_variable_value(expression->value.variable, env);
            printf("Variable value: %g\n", variable_value);
            if (!isnan(variable_value)) return variable_value; // user-defined variable
            
            fprintf(stderr, "Error: Undefined variable '%s'\n", expression->value.variable);
            return NAN;
        }
        case EXPRESSION_TYPE_UNARY: {
            double operand = evaluate(expressions, expression->value.unary.operand_index, env);
            switch (expression->value.unary.operator.kind) {
                case SYNTAX_KIND_PLUS:
                    return operand;
                case SYNTAX_KIND_MINUS:
                    return -operand;
                default:
                    fprintf(stderr, "Error: Unexpected unary operator %d\n", expression->value.unary.operator.kind);
                    return NAN;
            }
        }
        case EXPRESSION_TYPE_POSTFIX: {
            double operand = evaluate(expressions, expression->value.postfix.operand_index, env);
            switch (expression->value.postfix.operator.kind) {
                case SYNTAX_KIND_FACTORIAL: {
                    if (operand < 0 || floor(operand) != operand) {
                        fprintf(stderr, "Error: Factorial is only defined for non-negative integers, got %g\n", operand);
                        return NAN;
                    }
                    
                    if (operand > 170) { // 170! is the largest factorial that fits in a double
                        fprintf(stderr, "Error: Factorial result is too large to represent, got %g\n", operand);
                        return NAN;
                    }

                    double result = 1;
                    for (int i = 1; i <= (int)operand; i++) {
                        result *= i;
                    }

                    return result;
                }
                case SYNTAX_KIND_PERCENTAGE:
                    return operand / 100.0;
                default:
                    fprintf(stderr, "Error: Unexpected postfix operator %d\n", expression->value.postfix.operator.kind);
                    return NAN;
            }
        }
        case EXPRESSION_TYPE_BINARY: {
            double left = evaluate(expressions, expression->value.binary.left_index, env);
            double right = evaluate(expressions, expression->value.binary.right_index, env);
            switch (expression->value.binary.operator.kind) {
                case SYNTAX_KIND_PLUS:
                    return left + right;
                case SYNTAX_KIND_MINUS:
                    return left - right;
                case SYNTAX_KIND_MULTIPLY:
                    return left * right;
                case SYNTAX_KIND_DIVIDE:
                    if (right == 0) {
                        fprintf(stderr, "Error: Division by zero\n");
                        return NAN;
                    }
                    return left / right;
                default:
                    fprintf(stderr, "Error: Unexpected binary operator %d\n", expression->value.binary.operator.kind);
                    return NAN;
            }
        }
        case EXPRESSION_TYPE_CALL: {
            char* identifier = expression->value.call.identifier;
            InbuiltFunction func = get_inbuilt_function_from_identifier(identifier);

            if (func == INBUILT_FUNCTION_INVALID) {
                fprintf(stderr, "Error: Undefined function '%s'\n", identifier);
                return NAN;
            }

            double args[MAX_ARGUMENTS];
            for (size_t i = 0; i < expression->value.call.argument_count; i++) {
                args[i] = evaluate(expressions, expression->value.call.argument_indices[i], env);
            }
            return call_inbuilt_function(func, args, expression->value.call.argument_count);
        }
    }
}

double evaluate_token_expression(TokenExpression* expr, const VariableEnvironment* env) {
    Expression expressions[256];
    SyntaxToken tokens[256];
    size_t token_count = to_tokens(expr, tokens, 256);

    Parser parser;
    init_parser(&parser, tokens, token_count);
    size_t root_index = parse(&parser, expressions, 256);
    
    return evaluate(expressions, root_index, env);
}