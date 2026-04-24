from emulator.emulator import Emulator
from emulator.token_expression import NumberNode, AtomicNode, FunctionNode
from core.lexer import SyntaxToken, SyntaxKind
from core.functions.functions import FunctionNames

from time import time

if __name__ == "__main__":
    emulator = Emulator()
    exp = emulator.token_expression

    exp.insert_node(FunctionNode(FunctionNames.POW))
    exp.insert_node(NumberNode("2"))
    exp.shift_cursor(1)
    exp.insert_node(FunctionNode(FunctionNames.POW))
    exp.insert_node(NumberNode("3"))
    exp.shift_cursor(1)
    exp.insert_node(NumberNode("4"))
    exp.shift_cursor(-1)
    exp.shift_cursor(-1)
    exp.shift_cursor(-1)
    exp.shift_cursor(-1)
    exp.shift_cursor(-1)
    exp.shift_cursor(-1)
    exp.shift_cursor(-1)
    exp.backspace_node()

    print(emulator.token_expression.node_buffer[:emulator.token_expression.size])
    print("Current node", emulator.token_expression.node_buffer[emulator.token_expression.node_index])
    print("Node index", emulator.token_expression.node_index)
    print("Character index", emulator.token_expression.char_index)
    # print(emulator.token_expression.node_buffer[emulator.token_expression.node_index].num_str)

    # emulator.run()