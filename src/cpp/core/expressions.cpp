#include "expressions.h"
#include "evaluator.h"
#include <memory>

NumberExpression::NumberExpression(double value) : value(value) {}
VariableExpression::VariableExpression(FixedString16 name) : name(name) {}
UnaryExpression::UnaryExpression(SyntaxToken op, std::unique_ptr<Expression> operand) : op(op), operand(std::move(operand)) {}
PostfixExpression::PostfixExpression(std::unique_ptr<Expression> operand, SyntaxToken op) : operand(std::move(operand)), op(op) {}
BinaryExpression::BinaryExpression(std::unique_ptr<Expression> left, SyntaxToken op, std::unique_ptr<Expression> right) : left(std::move(left)), op(op), right(std::move(right)) {}
CallExpression::CallExpression(FixedString16 functionName, std::vector<std::unique_ptr<Expression>> arguments) : functionName(functionName), arguments(std::move(arguments)) {}

double NumberExpression::accept(Evaluator& visitor) {
    return visitor.visit(*this);
}

double VariableExpression::accept(Evaluator& visitor) {
    return visitor.visit(*this);
}

double UnaryExpression::accept(Evaluator& visitor) {
    return visitor.visit(*this);
}

double PostfixExpression::accept(Evaluator& visitor) {
    return visitor.visit(*this);
}

double BinaryExpression::accept(Evaluator& visitor) {
    return visitor.visit(*this);
}

double CallExpression::accept(Evaluator& visitor) {
    return visitor.visit(*this);
}