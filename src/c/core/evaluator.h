#pragma once

#include "expression.h"
#include "calculator/evaluator/token_expression.h"

typedef struct {
    const char* name;
    double value;
} Variable;

typedef struct {
    Variable* variables;
    size_t count;
} VariableEnvironment;

double evaluate(const Expression* expressions, size_t index, const VariableEnvironment* env);
double evaluate_token_expression(TokenExpression* expr, const VariableEnvironment* env);