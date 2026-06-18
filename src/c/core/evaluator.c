#include "evaluator.h"
#include "expression.h"
#include <math.h>
#include <string.h>
#include <stdio.h>

typedef enum {
    INBUILT_FUNCTION_INVALID = -1,
    INBUILT_FUNCTION_SIN,
    INBUILT_FUNCTION_COS,
    INBUILT_FUNCTION_TAN,
    INBUILT_FUNCTION_LN,
    INBUILT_FUNCTION_LOG,
    INBUILT_FUNCTION_SQRT,
    INBUILT_FUNCTION_POW,
    INBUILT_FUNCTION_ABS,
    INBUILT_FUNCTION_NROOT
} InbuiltFunction;

static InbuiltFunction get_inbuilt_function(const char* name) {
    if (strcmp(name, "sin") == 0) return INBUILT_FUNCTION_SIN;
    if (strcmp(name, "cos") == 0) return INBUILT_FUNCTION_COS;
    if (strcmp(name, "tan") == 0) return INBUILT_FUNCTION_TAN;
    if (strcmp(name, "ln") == 0) return INBUILT_FUNCTION_LN;
    if (strcmp(name, "log") == 0) return INBUILT_FUNCTION_LOG;
    if (strcmp(name, "sqrt") == 0) return INBUILT_FUNCTION_SQRT;
    if (strcmp(name, "pow") == 0) return INBUILT_FUNCTION_POW;
    if (strcmp(name, "abs") == 0) return INBUILT_FUNCTION_ABS;
    if (strcmp(name, "nroot") == 0) return INBUILT_FUNCTION_NROOT;

    return -1; // Invalid function
}

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
        case INBUILT_FUNCTION_NROOT:
            if (args[1] == 0) {
                fprintf(stderr, "Error: Cannot take the 0th root\n");
                return NAN;
            }
            return pow(args[0], 1.0 / args[1]);
        default:
            fprintf(stderr, "Error: Unknown inbuilt function\n");
            return NAN;
    }
}

static double get_constant(const char* name) {
    if (strcmp(name, "pi") == 0) return M_PI;
    if (strcmp(name, "e") == 0) return M_E;

    return NAN;
}

double evaluate(const Expression* expressions, size_t index) {
    const Expression* expression = &expressions[index];

    switch (expression->type) {
        case EXPRESSION_TYPE_NUMBER:
            return expression->value.number;
        case EXPRESSION_TYPE_VARIABLE: {
            double constant_value = get_constant(expression->value.variable);
            if (!isnan(constant_value)) {
                return constant_value;
            } else {
                fprintf(stderr, "Error: Undefined variable '%s'\n", expression->value.variable);
                return NAN;
            }
        }
        case EXPRESSION_TYPE_UNARY: {
            double operand = evaluate(expressions, expression->value.unary.operand_index);
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
            double operand = evaluate(expressions, expression->value.postfix.operand_index);
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
            double left = evaluate(expressions, expression->value.binary.left_index);
            double right = evaluate(expressions, expression->value.binary.right_index);
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
            InbuiltFunction func = get_inbuilt_function(identifier);

            if (func == INBUILT_FUNCTION_INVALID) {
                fprintf(stderr, "Error: Undefined function '%s'\n", identifier);
                return NAN;
            }

            double args[MAX_ARGUMENTS];
            for (size_t i = 0; i < expression->value.call.argument_count; i++) {
                args[i] = evaluate(expressions, expression->value.call.argument_indices[i]);
            }
            return call_inbuilt_function(func, args, expression->value.call.argument_count);
        }
    }
}