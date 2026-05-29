from core.parser import Parser
from core.symbols import SyntaxKind
from core.lexer import SyntaxToken
from core.expressions import NumberExpression, VariableExpression, UnaryExpression, BinaryExpression, CallExpression, PostfixExpression

from core.functions.functions import FUNCTION_INFO, BUILTIN_FUNCTION_STRING_MAP

from core.operators.factorial import factorial

from core.constants import PI, E
import math

POSTFIX_OPERATORS = {
    SyntaxKind.FACTORIAL_TOKEN: factorial,
    SyntaxKind.PERCENTAGE_TOKEN: lambda x: x * 0.01,
}

CONSTANTS = {
    "pi": PI,
    "e": E,
}

class Evaluator:
    def __init__(self, ast):
        self.ast = ast

    @classmethod
    def from_string(cls, input_string):
        parser = Parser.from_string(input_string)
        ast = parser.parse()
        return cls(ast)
    
    @classmethod
    def from_tokens(cls, tokens):
        parser = Parser.from_tokens(tokens)
        ast = parser.parse()
        return cls(ast)

    def evaluate(self):
        return self.ast.accept(self)

    def visit_number(self, node: NumberExpression):
        return node.value

    def visit_variable(self, node: VariableExpression):
        if node.name in CONSTANTS:
            return CONSTANTS[node.name]
        raise ValueError(f"Undefined variable: {node.name}")

    def visit_unary(self, node: UnaryExpression):
        operand = node.operand.accept(self)

        match node.operator.kind:
            case SyntaxKind.PLUS_TOKEN:
                return +operand
            case SyntaxKind.MINUS_TOKEN:
                return -operand
            case _:
                raise ValueError(f"Unsupported unary operator: {node.operator.kind}")

    def visit_binary(self, node: BinaryExpression):
        left = node.left.accept(self)
        right = node.right.accept(self)

        match node.operator.kind:
            case SyntaxKind.PLUS_TOKEN:
                return left + right
            case SyntaxKind.MINUS_TOKEN:
                return left - right
            case SyntaxKind.MULTIPLY_TOKEN:
                return left * right
            case SyntaxKind.DIVIDE_TOKEN:
                return left / right
            case _:
                raise ValueError(f"Unsupported binary operator: {node.operator.kind}")

    def visit_postfix(self, node: PostfixExpression):
        operand = node.operand.accept(self)

        if node.operator.kind in POSTFIX_OPERATORS:
            operator_function = POSTFIX_OPERATORS[node.operator.kind]
            return operator_function(operand)
        
        raise ValueError(f"Unsupported postfix operator: {node.operator.kind}")

    def visit_call(self, node: CallExpression):
        arguments = [node.accept(self) for node in node.arguments]
        # use inbuilt functions now
        match node.identifier:
            case "sin":
                return math.sin(arguments[0])
            case "cos":
                return math.cos(arguments[0])
            case "tan":
                return math.tan(arguments[0])
            case "pow":
                return math.pow(arguments[0], arguments[1])
            case "ln":
                return math.log(arguments[0])
            case "log":
                return math.log(arguments[0], 10)
            case "sqrt":
                return math.sqrt(arguments[0])
            case "nroot":
                return math.pow(arguments[0], 1/arguments[1])
            case "abs":
                return abs(arguments[0])
            case "frac":
                return arguments[0] / arguments[1]
        
        raise ValueError(f"Undefined function: {node.identifier}")

if __name__ == "__main__":
    input_string = "pow(2, pow(3, 2)) + nroot(27, 3) - ln(e) + 5!"
    evaluator = Evaluator(input_string)
    result = evaluator.evaluate()
    print(f"Result of evaluating expression: {input_string} is {result}")