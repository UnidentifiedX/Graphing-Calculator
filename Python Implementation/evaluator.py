from parser import Parser
from expressions import NumberExpression, VariableExpression, UnaryExpression, BinaryExpression, CallExpression, PostfixExpression
from parser import SyntaxKind

from functions.pow import pow
from functions.abs import abs
from functions.sqrt import sqrt
from functions.exp import exp
from functions.ln import ln
from functions.log import log
from functions.nroot import nroot

from operators.factorial import factorial

from constants import PI, E

FUNCTIONS = {
    "abs": abs,
    "sqrt": sqrt,
    "exp": exp,
    "ln": ln,
    "log": log,
    "pow": pow,
    "nroot": nroot
}

POSTFIX_OPERATORS = {
    SyntaxKind.FACTORIAL_TOKEN: factorial,
    SyntaxKind.PERCENTAGE_TOKEN: lambda x: x * 0.01,
}

CONSTANTS = {
    "pi": PI,
    "e": E,
}

class Evaluator:
    def __init__(self, input_string):
        self.ast = Parser(input_string).parse()

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
        if node.identifier in FUNCTIONS:
            function = FUNCTIONS[node.identifier]
            arguments = [arg.accept(self) for arg in node.arguments]
            return function(*arguments)
        
        raise ValueError(f"Undefined function: {node.identifier}")

if __name__ == "__main__":
    input_string = "pow(2, pow(3, 2)) + nroot(27, 3) - ln(e) + 5!"
    evaluator = Evaluator(input_string)
    result = evaluator.evaluate()
    print(f"Result of evaluating expression: {input_string} is {result}")