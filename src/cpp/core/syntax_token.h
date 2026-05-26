#pragma once
#include "syntax_kind.h"
#include <string>
#include <variant>
#include "fixed_string_16.h"

// just in case future me asks: i have to use a variant because token can be a double for numbers
// or a string identifier for variables and functions. so unfortunately i have to create my own string
// handler to not use the heap

using TokenValue = std::variant<std::monostate, double, FixedString16>;

struct SyntaxToken
{
    SyntaxKind kind;
    TokenValue value;
};