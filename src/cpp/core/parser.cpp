#include "parser.h"
#include "syntax_kind.h"
#include "syntax_token.h"
#include "expressions.h"
#include "lexer.h"
#include <stdint.h>
#include <memory>
#include <vector>
#include <unordered_set>
#include <iostream>

uint8_t getBindingPower(SyntaxToken token) {
    switch (token.kind) {
        case SyntaxKind::NumberToken:
        case SyntaxKind::VariableToken:
        case SyntaxKind::CloseParenthesisToken:
        case SyntaxKind::CommaToken:
            return 0;
        case SyntaxKind::PlusToken:
        case SyntaxKind::MinusToken:
            return 10;
        case SyntaxKind::MultiplyToken:
        case SyntaxKind::DivideToken:
            return 20;
        case SyntaxKind::UnaryToken:
            return 30;
        case SyntaxKind::FactorialToken:
        case SyntaxKind::PercentageToken:
            return 40;
        default:
            std::cout << "Error: Unexpected token " << static_cast<int>(token.kind) << " in getBindingPower" << std::endl;
            return 0; // Placeholder
    }
}

std::unordered_set<SyntaxKind> IMPLICIT_LEFT_TOKENS = {
    SyntaxKind::NumberToken,
    SyntaxKind::VariableToken,
    SyntaxKind::CloseParenthesisToken,
    SyntaxKind::FactorialToken,
    SyntaxKind::PercentageToken
};

std::unordered_set<SyntaxKind> IMPLICIT_RIGHT_TOKENS = {
    SyntaxKind::NumberToken,
    SyntaxKind::VariableToken,
    SyntaxKind::OpenParenthesisToken
};

Parser::Parser(std::string input_string) : Parser(Lexer(input_string).tokenize()) {

}

Parser::Parser(std::array<SyntaxToken, 256> tokens) : tokens(tokens) {
    position = 0;

    for (size_t i = 0; i < tokens.size(); ++i) {
        if (tokens[i].kind == SyntaxKind::EndOfFileToken) {
            break;
        }

        // std::cout << "Token " << i << ": " << static_cast<int>(tokens[i].kind) << " (" << tokens[i].value << ")\n";
    }
}

SyntaxToken Parser::peek() const {
    if (position < tokens.size()) {
        return tokens[position];
    }
    return SyntaxToken{ SyntaxKind::EndOfFileToken, std::monostate{} };
}

SyntaxToken Parser::consume() {
    SyntaxToken token = peek();
    if (token.kind != SyntaxKind::EndOfFileToken) {
        position++;
    }
    return token;
}

std::unique_ptr<Expression> Parser::parse() {
    std::unique_ptr<Expression> tree = parseExpression(0);

    if (peek().kind != SyntaxKind::EndOfFileToken) {
        // Handle error: unexpected token after parsing complete expression
    }

    return tree;
}

std::unique_ptr<Expression> Parser::parseExpression(uint8_t rbp = 0) {
    SyntaxToken token = consume();
    std::unique_ptr<Expression> left = nud(token);

    while (true) {
        SyntaxToken token = peek();
        if (token.kind == SyntaxKind::EndOfFileToken) break;

        // check for implicit multiplication by checking if the previous token allows it
        // and the next token is one that can be implicitly multiplied
        // honestly if you just check for implicit tokens on the right it should be ok
        // but this way we can avoid some edge cases (i will think about this more later)
        bool isImplicit = (
            position > 0 &&
            IMPLICIT_LEFT_TOKENS.count(tokens[position - 1].kind) > 0 &&
            IMPLICIT_RIGHT_TOKENS.count(token.kind) > 0
        );

        if (isImplicit)
            token = SyntaxToken{ SyntaxKind::MultiplyToken, std::monostate{} }; // Virtual token

        if (getBindingPower(token) <= rbp)
            break;

        if (!isImplicit)
            token = consume();

        left = led(std::move(left), token);
    }

    return left;
}

std::unique_ptr<Expression> Parser::led(std::unique_ptr<Expression> left, SyntaxToken token) {
    switch (token.kind) {
        case SyntaxKind::PlusToken:
        case SyntaxKind::MinusToken:
        case SyntaxKind::MultiplyToken:
        case SyntaxKind::DivideToken: {
            std::unique_ptr<Expression> right = parseExpression(getBindingPower(token));
            return std::make_unique<BinaryExpression>(std::move(left), token, std::move(right));
        }
        case SyntaxKind::FactorialToken:
        case SyntaxKind::PercentageToken: {
            return std::make_unique<PostfixExpression>(std::move(left), token);
        }
        default:
            // Handle error: unexpected token in led
            return nullptr;
    }
}

std::unique_ptr<Expression> Parser::nud(SyntaxToken token) {
    switch (token.kind) {
        case SyntaxKind::NumberToken:
            return std::make_unique<NumberExpression>(std::get<double>(token.value));
        case SyntaxKind::VariableToken: {
            // check function call
            if (peek().kind == SyntaxKind::OpenParenthesisToken) {
                consume(); // consume '('
                std::vector<std::unique_ptr<Expression>> arguments;
                if (peek().kind != SyntaxKind::CloseParenthesisToken) {
                    while (true) {
                        arguments.push_back(parseExpression(0));
                        if (peek().kind != SyntaxKind::CommaToken)
                            break;

                        consume(); // consume ','
                    }
                }

                if (peek().kind != SyntaxKind::CloseParenthesisToken) {
                    // Handle error: expected ')'
                }
                consume(); // consume ')'
                return std::make_unique<CallExpression>(std::get<FixedString16>(token.value), std::move(arguments));
            }

            return std::make_unique<VariableExpression>(std::get<FixedString16>(token.value));
        }
        case SyntaxKind::PlusToken:
        case SyntaxKind::MinusToken: {
            std::unique_ptr<Expression> operand = parseExpression(getBindingPower(SyntaxToken{ SyntaxKind::UnaryToken, std::monostate{} }));
            return std::make_unique<UnaryExpression>(token, std::move(operand));
        }
        case SyntaxKind::OpenParenthesisToken: {
            std::unique_ptr<Expression> expression = parseExpression(0);
            if (peek().kind != SyntaxKind::CloseParenthesisToken) {
                // Handle error: expected ')'
            }
            consume(); // consume ')'
            return expression;
        }
        default:
            std::cout << "Error: Unexpected token " << static_cast<int>(token.kind) << " in nud" << std::endl;
            return nullptr;
    }
}

void Parser::printAST(const Expression* expr, std::string prefix, bool is_last) {
    if (!expr) return;

    const char* connector = is_last ? "└── " : "├── ";

    if (const NumberExpression* numberExpr = dynamic_cast<const NumberExpression*>(expr)) {
        std::cout << prefix << connector << numberExpr->value << '\n';
    } else if (const UnaryExpression* unaryExpr = dynamic_cast<const UnaryExpression*>(expr)) {
        std::cout << prefix << connector << static_cast<int>(unaryExpr->op.kind) << '\n';
        std::string new_prefix = prefix + (is_last ? "    " : "│   ");
        printAST(unaryExpr->operand.get(), new_prefix, true);
    } else if (const BinaryExpression* binaryExpr = dynamic_cast<const BinaryExpression*>(expr)) {
        std::cout << prefix << connector << static_cast<int>(binaryExpr->op.kind) << '\n';
        std::string new_prefix = prefix + (is_last ? "    " : "│   ");
        printAST(binaryExpr->left.get(), new_prefix, false);
        printAST(binaryExpr->right.get(), new_prefix, true);
    } else if (const CallExpression* callExpr = dynamic_cast<const CallExpression*>(expr)) {
        std::cout << prefix << connector << callExpr->functionName.c_str() << '\n';
        std::string new_prefix = prefix + (is_last ? "    " : "│   ");
        for (size_t i = 0; i < callExpr->arguments.size(); ++i) {
            printAST(callExpr->arguments[i].get(), new_prefix, i + 1 == callExpr->arguments.size());
        }
    } else if (const VariableExpression* varExpr = dynamic_cast<const VariableExpression*>(expr)) {
        std::cout << prefix << connector << varExpr->name.c_str() << '\n';
    } else if (const PostfixExpression* postfixExpr = dynamic_cast<const PostfixExpression*>(expr)) {
        std::cout << prefix << connector << static_cast<int>(postfixExpr->op.kind) << '\n';
        std::string new_prefix = prefix + (is_last ? "    " : "│   ");
        printAST(postfixExpr->operand.get(), new_prefix, true);
    } else {
        std::cout << prefix << connector << "Unknown node type\n";
    }
}