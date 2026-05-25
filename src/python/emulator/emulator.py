import pygame
from emulator.buttons import ButtonMatrix, UIStateButton, AtomicTokenButton, NumberTokenButton, ControlButton, FunctionTokenButton, Button
from core.symbols import SyntaxKind
from core.evaluator import Evaluator
from core.lexer import SyntaxToken
from enum import Enum, auto
from emulator.token_expression import TokenExpression, AtomicNode, NumberNode, FunctionNode, NodeType
from core.functions.functions import FunctionNames

BUTTON_MATRIX = [
    [FunctionTokenButton("sin", FunctionNames.SIN), FunctionTokenButton("cos", FunctionNames.COS), FunctionTokenButton("tan", FunctionNames.TAN), FunctionTokenButton("^", FunctionNames.POW)],
    [AtomicTokenButton("(", SyntaxKind.OPEN_PAREN_TOKEN), AtomicTokenButton(")", SyntaxKind.CLOSE_PAREN_TOKEN), ControlButton("AC"), ControlButton("DEL")],
    [NumberTokenButton("7"), NumberTokenButton("8"), NumberTokenButton("9"), AtomicTokenButton("/", SyntaxKind.DIVIDE_TOKEN)],
    [NumberTokenButton("4"), NumberTokenButton("5"), NumberTokenButton("6"), AtomicTokenButton("*", SyntaxKind.MULTIPLY_TOKEN)],
    [NumberTokenButton("1"), NumberTokenButton("2"), NumberTokenButton("3"), AtomicTokenButton("-", SyntaxKind.MINUS_TOKEN)],
    [NumberTokenButton("0"), NumberTokenButton("."), ControlButton("="), AtomicTokenButton("+", SyntaxKind.PLUS_TOKEN)],
]

class Emulator:
    def __init__(self):
        pygame.init()
        self.screen = pygame.display.set_mode((400, 300))
        self.running = True
        self.button_matrix = ButtonMatrix(BUTTON_MATRIX)
        self.token_expression = TokenExpression()

    def run(self):
        while self.running:
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    self.running = False
                if event.type == pygame.MOUSEBUTTONDOWN:
                    self.button_matrix.handle_click(event.pos, self)
            self.screen.fill((255, 255, 255))
            self.button_matrix.render(self.screen)
            pygame.display.flip()

    def visit_ui_state_button(self, button):
        pass

    def visit_token_button(self, button: Button):
        self.token_expression.insert_node(button.create_node())
        self.display_expression()

    def visit_control_button(self, button: ControlButton):
        match button.label:
            case "=":
                print(self.token_expression.node_buffer[:self.token_expression.size])
                tokens = self.token_expression.generate_token_list()
                print(tokens)
                evaluator = Evaluator.from_tokens(tokens)
                result = evaluator.evaluate()
                print("Result:", result)
                self.token_expression.clear_buffer()
            case "AC":
                self.token_expression.clear_buffer()
            case "DEL":
                self.token_expression.backspace_node()
                self.display_expression()

    def display_expression(self):
        # For now let's just print the expression to the console
        print("\033[1A\x1b[2K", end="")
        for node in self.token_expression.node_buffer[:self.token_expression.size]:
            if node.type == NodeType.ATOM:
                match node.token.kind:
                    case SyntaxKind.PLUS_TOKEN:
                        print("+", end="")
                    case SyntaxKind.MINUS_TOKEN:
                        print("-", end="")
                    case SyntaxKind.MULTIPLY_TOKEN:
                        print("*", end="")
                    case SyntaxKind.DIVIDE_TOKEN:
                        print("/", end="")
                    case SyntaxKind.OPEN_PAREN_TOKEN:
                        print("(", end="")
                    case SyntaxKind.CLOSE_PAREN_TOKEN:
                        print(")", end="")
                    case SyntaxKind.FACTORIAL_TOKEN:
                        print("!", end="")
                    case SyntaxKind.PERCENTAGE_TOKEN:
                        print("%", end="")
            elif node.type == NodeType.NUMBER:
                print(node.num_str, end="")
            elif node.type == NodeType.FUNCTION:
                print(node.identifier.value, end="")
        print()