#include "evaluator.h"
#include "lexer.h"
#include "parser.h"
#include "constants.h"
#include <unordered_map>
#include <string>
#include <functional>
#include <iostream>
#include <limits>
#include <cmath>

using MathFunction = std::function<double(const std::vector<double>&)>;
struct InbuiltFunction {
    size_t arity;
    MathFunction function;
};

namespace FunctionNames {
    constexpr const char* SIN = "sin";
    constexpr const char* COS = "cos";
    constexpr const char* TAN = "tan";
    constexpr const char* LN = "ln";
    constexpr const char* LOG = "log";
    constexpr const char* SQRT = "sqrt";
    constexpr const char* POW = "pow";
    constexpr const char* ABS = "abs";
    constexpr const char* NROOT = "nroot";
}

const std::unordered_map<std::string, InbuiltFunction> InbuiltFunctions = {
    {FunctionNames::SIN,   {1, [](const std::vector<double>& args) { return std::sin(args[0]); }}},
    {FunctionNames::COS,   {1, [](const std::vector<double>& args) { return std::cos(args[0]); }}},
    {FunctionNames::TAN,   {1, [](const std::vector<double>& args) { return std::tan(args[0]); }}},
    {FunctionNames::LN,    {1, [](const std::vector<double>& args) { return std::log(args[0]); }}},
    {FunctionNames::LOG,   {1, [](const std::vector<double>& args) { return std::log10(args[0]); }}},
    {FunctionNames::SQRT,  {1, [](const std::vector<double>& args) { return std::sqrt(args[0]); }}},
    {FunctionNames::POW,   {2, [](const std::vector<double>& args) { return std::pow(args[0], args[1]); }}},
    {FunctionNames::ABS,   {1, [](const std::vector<double>& args) { return std::abs(args[0]); }}},
    {FunctionNames::NROOT, {2, [](const std::vector<double>& args) { return std::pow(args[0], 1.0 / args[1]); }}} // FIX: this doesnt handle odd roots of negative numbers correctly so gotta fix
};

const std::unordered_map<std::string, double> CONSTANTS = {
    {"pi", PI},
    {"e", E}
};
    
Evaluator::Evaluator(std::string input_string) : Evaluator(Lexer(input_string).tokenize()) {}

Evaluator::Evaluator(std::array<SyntaxToken, 256> tokens) : ast(Parser(tokens).parse()) {
    // Parser::printAST(ast.get());
}

double Evaluator::evaluate() {
    return ast->accept(*this);
}

double Evaluator::visit(const NumberExpression& expr) {
    return expr.value;
}

double Evaluator::visit(const VariableExpression& expr) {
    auto it = CONSTANTS.find(expr.name.c_str());
    if (it != CONSTANTS.end()) {
        return it->second;
    } 

    std::cout << "Error: Undefined variable '" << expr.name.c_str() << "'" << std::endl;
    return 0; // Placeholder
}

double Evaluator::visit(const UnaryExpression& expr) {
    auto operand = expr.operand->accept(*this);
    switch (expr.op.kind) {
        case SyntaxKind::PlusToken:
            return operand;
        case SyntaxKind::MinusToken:
            return -operand;
        default:
            std::cout << "Error: Unexpected unary operator " << static_cast<int>(expr.op.kind) << std::endl;
            return 0; // Placeholder
    }
}

double Evaluator::visit(const PostfixExpression& expr) {
    auto operand = expr.operand->accept(*this);
    switch (expr.op.kind) {
        case SyntaxKind::FactorialToken: {
            int upper_bound = static_cast<int>(operand);
            if (operand < 0 || operand != upper_bound) {
                std::cout << "Error: Factorial is only defined for non-negative integers." << std::endl;
                return 0; // Placeholder
            }

            if (upper_bound > 170) {
                std::cout << "Error: Factorial result is too large to fit in a double." << std::endl;
                return std::numeric_limits<double>::infinity(); // Placeholder for overflow
            }

            double result = 1;
            for (int i = 1; i <= upper_bound; i++) {
                result *= i;
            }

            return result;
        }
        case SyntaxKind::PercentageToken:
            return operand / 100.0;
        default:
            std::cout << "Error: Unexpected postfix operator " << static_cast<int>(expr.op.kind) << std::endl;
            return 0; // Placeholder
    }
}

double Evaluator::visit(const BinaryExpression& expr) {
    auto left = expr.left->accept(*this);
    auto right = expr.right->accept(*this);
    switch (expr.op.kind) {
        case SyntaxKind::PlusToken:
            return left + right;
        case SyntaxKind::MinusToken:
            return left - right;
        case SyntaxKind::MultiplyToken:
            return left * right;
        case SyntaxKind::DivideToken:
            return left / right;
        default:
            // Handle error: unexpected binary operator
            std::cout << "Error: Unexpected binary operator " << static_cast<int>(expr.op.kind) << std::endl;
            return 0; // Placeholder
    }
}

double Evaluator::visit(const CallExpression& expr) {
    FixedString16 functionName = expr.functionName;
    auto it = InbuiltFunctions.find(functionName.c_str());

    if (it == InbuiltFunctions.end()) {
        std::cout << "Error: Undefined function '" << functionName.c_str() << "'" << std::endl;
        return 0; // Placeholder
    }  

    std::vector<double> argumentValues; // vector for now but change to array later
    argumentValues.reserve(expr.arguments.size());

    for (const auto& arg : expr.arguments) {
        argumentValues.push_back(arg->accept(*this));
    }

    if (argumentValues.size() != it->second.arity) {
        std::cout << "Error: Function '" << functionName.c_str() << "' expects " << it->second.arity << " arguments, but " << argumentValues.size() << " were given." << std::endl;
        return 0; // Placeholder
    }

    return it->second.function(argumentValues);
}