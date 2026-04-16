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