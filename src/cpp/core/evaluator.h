#pragma once
#include <string>
#include <memory>
#include "syntax_token.h"
#include "expressions.h"

struct Evaluator {
    Evaluator(std::string input_string);
    Evaluator(std::array<SyntaxToken, 256> tokens);

    double evaluate();
    double visit(const NumberExpression& expr);
    double visit(const VariableExpression& expr);
    double visit(const UnaryExpression& expr);
    double visit(const PostfixExpression& expr);
    double visit(const BinaryExpression& expr);
    double visit(const CallExpression& expr);

private:
    std::unique_ptr<Expression> ast;
};