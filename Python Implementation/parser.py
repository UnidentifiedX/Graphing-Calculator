from lexer import Lexer, SyntaxToken, SyntaxKind
from expressions import NumberExpression, BinaryExpression, UnaryExpression, CallExpression, VariableExpression, PostfixExpression

BINDING_POWER = {
    SyntaxKind.NUMBER_TOKEN: 0,
    SyntaxKind.VARIABLE_TOKEN: 0,
    SyntaxKind.PLUS_TOKEN: 10,
    SyntaxKind.MINUS_TOKEN: 10,
    SyntaxKind.MULTIPLY_TOKEN: 20,
    SyntaxKind.DIVIDE_TOKEN: 20,
    SyntaxKind.UNARY_TOKEN: 30,
    SyntaxKind.FACTORIAL_TOKEN: 40,
    SyntaxKind.PERCENTAGE_TOKEN: 40,
}

IMPLICIT_LEFT_TOKENS = {
    SyntaxKind.NUMBER_TOKEN,
    SyntaxKind.VARIABLE_TOKEN,
    SyntaxKind.CLOSE_PAREN_TOKEN,
    SyntaxKind.FACTORIAL_TOKEN,
    SyntaxKind.PERCENTAGE_TOKEN,
}

IMPLICIT_RIGHT_TOKENS = {
    SyntaxKind.NUMBER_TOKEN,
    SyntaxKind.VARIABLE_TOKEN,
    SyntaxKind.OPEN_PAREN_TOKEN,
}

class Parser:
    def __init__(self, input_string):
        self.tokens: list[SyntaxToken] = Lexer(input_string).tokenize()
        self.position = 0
    
    def peek(self):
        if self.position < len(self.tokens):
            return self.tokens[self.position]
        return SyntaxToken(SyntaxKind.END_OF_FILE_TOKEN, None)

    def consume(self):
        token = self.peek()
        if token.kind != SyntaxKind.END_OF_FILE_TOKEN:
            self.position += 1
        return token
    
    def parse(self):
        tree = self.parse_expression()
        if self.peek().kind != SyntaxKind.END_OF_FILE_TOKEN:
            raise ValueError("Unexpected token at end of expression")
        return tree

    def parse_expression(self, rbp=0):
        token = self.consume()
        left = self.nud(token)

        while True:
            token = self.peek()
            if token.kind == SyntaxKind.END_OF_FILE_TOKEN:
                break
            
            # check for implicit multiplication by checking if the previous token allows it
            # and the next token is one that can be implicitly multiplied
            # honestly if you just check for implicit tokens on the right it should be ok
            # but this way we can avoid some edge cases (i will think about this more later)
            is_implicit = (self.position > 0 
                and self.tokens[self.position - 1].kind in IMPLICIT_LEFT_TOKENS 
                and token.kind in IMPLICIT_RIGHT_TOKENS)
            if (is_implicit):
                token = SyntaxToken(SyntaxKind.MULTIPLY_TOKEN, None) # virtual token

            if self.bp(token) <= rbp:
                break
            if (not is_implicit):
                token = self.consume()

            left = self.led(left, token)

        return left

    def led(self, left, token: SyntaxToken):
        match token.kind:
            case SyntaxKind.PLUS_TOKEN | SyntaxKind.MINUS_TOKEN | \
                 SyntaxKind.MULTIPLY_TOKEN | SyntaxKind.DIVIDE_TOKEN:
                right = self.parse_expression(self.bp(token))
                return BinaryExpression(left, token, right)
            case SyntaxKind.FACTORIAL_TOKEN | SyntaxKind.PERCENTAGE_TOKEN:
                return PostfixExpression(left, token)
            case _:
                raise ValueError(f"Unexpected token: {token.kind}")

    def nud(self, token: SyntaxToken):
        match token.kind:
            case SyntaxKind.NUMBER_TOKEN:
                return NumberExpression(token.value)
            case SyntaxKind.VARIABLE_TOKEN:
                # check function call
                if self.peek().kind == SyntaxKind.OPEN_PAREN_TOKEN:
                    args = []
                    self.consume() # Consume "("

                    if self.peek().kind != SyntaxKind.CLOSE_PAREN_TOKEN:
                        while True:
                            argument = self.parse_expression(0)
                            args.append(argument)
                            if self.peek().kind != SyntaxKind.COMMA_TOKEN:
                                break                  
                            self.consume() # Consume ","

                    if self.peek().kind != SyntaxKind.CLOSE_PAREN_TOKEN:
                        raise ValueError("Expected closing parenthesis")
                    
                    self.consume() # Consume ")"
                    return CallExpression(token.value, args)

                return VariableExpression(token.value)
            case SyntaxKind.PLUS_TOKEN | SyntaxKind.MINUS_TOKEN:
                operand = self.parse_expression(BINDING_POWER[SyntaxKind.UNARY_TOKEN])
                return UnaryExpression(token, operand)
            case SyntaxKind.OPEN_PAREN_TOKEN:
                expression = self.parse_expression(0)
                if self.peek().kind != SyntaxKind.CLOSE_PAREN_TOKEN:
                    raise ValueError("Expected closing parenthesis")
                self.consume() # Consume ")"
                return expression
            case _:
                raise ValueError(f"Unexpected token: {token.kind}")

    def bp(self, token: SyntaxToken):
        if token.kind == SyntaxKind.CLOSE_PAREN_TOKEN:
            return 0

        if token.kind in BINDING_POWER:
            return BINDING_POWER[token.kind]

        return 0
            
def print_ast(node, prefix="", is_last=True):
    connector = "└── " if is_last else "├── "

    if isinstance(node, NumberExpression):
        print(prefix + connector + str(node.value))
    elif isinstance(node, UnaryExpression):
        print(prefix + connector + node.operator.kind.name)
        new_prefix = prefix + ("    " if is_last else "│   ")
        print_ast(node.operand, new_prefix, True)
    elif isinstance(node, BinaryExpression):
        print(prefix + connector + node.operator.kind.name)
        new_prefix = prefix + ("    " if is_last else "│   ")
        print_ast(node.left, new_prefix, False)
        print_ast(node.right, new_prefix, True)
    elif isinstance(node, CallExpression):
        print(prefix + connector + node.identifier)
        new_prefix = prefix + ("    " if is_last else "│   ")
        for i, arg in enumerate(node.arguments):
            print_ast(arg, new_prefix, i == len(node.arguments) - 1)
    elif isinstance(node, VariableExpression):
        print(prefix + connector + node.name)
    elif isinstance(node, PostfixExpression):
        print(prefix + connector + node.operator.kind.name)
        new_prefix = prefix + ("    " if is_last else "│   ")
        print_ast(node.operand, new_prefix, True)
    else:
        print(prefix + connector + "Unknown node type")

if __name__ == "__main__":
    input_string = "-------9!"
    parser = Parser(input_string)
    ast = parser.parse()
    print(f"Parsed AST for expression: {input_string}")
    print_ast(ast)