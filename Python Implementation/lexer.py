from enum import Enum, auto

# in the future i might just extend this enum so lexable and virtual tokens are separate
# removed all function tokens since they can be handled as variables and then resolved during evaluation
#     - this also means that user-defined functions can be supported without needing to modify the lexer or parser
class SyntaxKind(Enum):
    NUMBER_TOKEN = auto()
    VARIABLE_TOKEN = auto()
    PLUS_TOKEN = auto()
    MINUS_TOKEN = auto()
    UNARY_TOKEN = auto()
    MULTIPLY_TOKEN = auto()
    DIVIDE_TOKEN = auto()
    OPEN_PAREN_TOKEN = auto()
    CLOSE_PAREN_TOKEN = auto()
    COMMA_TOKEN = auto()
    FACTORIAL_TOKEN = auto()
    PERCENTAGE_TOKEN = auto()
    END_OF_FILE_TOKEN = auto()

class SyntaxToken:
    def __init__(self, kind, value):
        self.kind = kind
        self.value = value

class Lexer:
    def __init__(self, input_string):
        self.input_string = input_string
        self.position = 0

    def peek(self):
        if self.position < len(self.input_string):
            return self.input_string[self.position]
        return "\0"
    
    def next(self):
        if self.position < len(self.input_string):
            char = self.input_string[self.position]
            self.position += 1
            return char
        return "\0"
    
    def parse_number(self):
        number_string = ""
        has_decimal_point = False
        while True:
            current_char = self.peek()
            if current_char == "." and not has_decimal_point:
                has_decimal_point = True
                number_string += current_char
                self.next()
            elif current_char.isdigit():
                number_string += current_char
                self.next()
            else:
                break
        
        if number_string == ".":
            raise ValueError("Invalid number format: '.' is not a valid number")

        return float(number_string) if has_decimal_point else int(number_string)

    def parse_identifier(self):
        identifier_string = ""
        while True:
            current_char = self.peek()
            if current_char.isalnum() or current_char == "_":
                identifier_string += current_char
                self.next()
            else:
                break
        return identifier_string

    # NOTE: honestly this might not work for like x1 x2 y1 y2 representations but thats a problem for future me to solve
    def tokenize(self):
        tokens = []
        while True:
            current_char = self.peek()
            
            match current_char:
                case "\0":
                    tokens.append(SyntaxToken(SyntaxKind.END_OF_FILE_TOKEN, None))
                    break
                case "+":
                    tokens.append(SyntaxToken(SyntaxKind.PLUS_TOKEN, None))
                    self.next()
                case "-":
                    tokens.append(SyntaxToken(SyntaxKind.MINUS_TOKEN, None))
                    self.next()
                case "*":
                    tokens.append(SyntaxToken(SyntaxKind.MULTIPLY_TOKEN, None))
                    self.next()
                case "/":
                    tokens.append(SyntaxToken(SyntaxKind.DIVIDE_TOKEN, None))
                    self.next()
                case "(":
                    tokens.append(SyntaxToken(SyntaxKind.OPEN_PAREN_TOKEN, None))
                    self.next()
                case ")":
                    tokens.append(SyntaxToken(SyntaxKind.CLOSE_PAREN_TOKEN, None))
                    self.next()
                case "!":
                    tokens.append(SyntaxToken(SyntaxKind.FACTORIAL_TOKEN, None))
                    self.next()
                case "%":
                    tokens.append(SyntaxToken(SyntaxKind.PERCENTAGE_TOKEN, None))
                    self.next()
                case ",":
                    tokens.append(SyntaxToken(SyntaxKind.COMMA_TOKEN, None))
                    self.next()
                case current_char if current_char.isdigit():
                    number_value = self.parse_number()
                    tokens.append(SyntaxToken(SyntaxKind.NUMBER_TOKEN, number_value))
                case current_char if current_char.isalnum() or current_char == "_":
                    identifier_value = self.parse_identifier()
                    tokens.append(SyntaxToken(SyntaxKind.VARIABLE_TOKEN, identifier_value))
                case current_char if current_char.isspace():
                    self.next()  # Skip whitespace
                case _:
                    raise ValueError(f"Unexpected character: '{current_char}'")
        return tokens
    
if __name__ == "__main__":
    input_string = "5xsin(30)"
    lexer = Lexer(input_string)
    tokens = lexer.tokenize()
    print(f"Expression: {input_string}")
    for token in tokens:
        print(f"Kind: {token.kind.name}, Value: {token.value}")