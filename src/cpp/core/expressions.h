#pragma once
#include <string>
#include <vector>
#include <memory>
#include "syntax_token.h"
#include "fixed_string_16.h"

class Evaluator;

struct Expression {
    virtual ~Expression() = default; 
    virtual double accept(class Evaluator& visitor) = 0;
};

struct NumberExpression : public Expression {
    double value;
    NumberExpression(double value);
    double accept(Evaluator& visitor) override;
};

struct VariableExpression : public Expression {
    FixedString16 name;
    VariableExpression(FixedString16 name);
    double accept(Evaluator& visitor) override;
};

struct UnaryExpression : public Expression {
    SyntaxToken op;
    std::unique_ptr<Expression> operand;
    UnaryExpression(SyntaxToken op, std::unique_ptr<Expression> operand);
    double accept(Evaluator& visitor) override;
};

struct PostfixExpression : public Expression {
    SyntaxToken op;
    std::unique_ptr<Expression> operand;
    PostfixExpression(std::unique_ptr<Expression> operand, SyntaxToken op);
    double accept(Evaluator& visitor) override;
};

struct BinaryExpression : public Expression {
    SyntaxToken op;
    std::unique_ptr<Expression> left;
    std::unique_ptr<Expression> right;
    BinaryExpression(std::unique_ptr<Expression> left, SyntaxToken op, std::unique_ptr<Expression> right);
    double accept(Evaluator& visitor) override;
};

struct CallExpression : public Expression {
    FixedString16 functionName;
    std::vector<std::unique_ptr<Expression>> arguments;
    CallExpression(FixedString16 functionName, std::vector<std::unique_ptr<Expression>> arguments);
    double accept(Evaluator& visitor) override;
};