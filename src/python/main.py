from emulator.emulator import Emulator
from core.symbols import SyntaxKind
from emulator.token_expression import NumberNode, AtomicNode, FunctionNode
from core.lexer import SyntaxToken, SyntaxKind
from core.functions.functions import FunctionNames
from core.evaluator import Evaluator

from time import time

if __name__ == "__main__":
    emulator = Emulator()

    emulator.run()