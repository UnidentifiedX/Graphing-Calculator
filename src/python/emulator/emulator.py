import pygame
from emulator.buttons import ButtonMatrix, UIStateButton, TokenButton, NumberButton, ControlButton
from core.symbols import SyntaxKind
from core.evaluator import Evaluator
from core.lexer import SyntaxToken
from enum import Enum, auto
from emulator.token_expression import TokenExpression

BUTTON_MATRIX = [
    [NumberButton("7"), NumberButton("8"), NumberButton("9"), TokenButton("/", SyntaxToken(SyntaxKind.DIVIDE_TOKEN, "/"))],
    [NumberButton("4"), NumberButton("5"), NumberButton("6"), TokenButton("*", SyntaxToken(SyntaxKind.MULTIPLY_TOKEN, "*"))],
    [NumberButton("1"), NumberButton("2"), NumberButton("3"), TokenButton("-", SyntaxToken(SyntaxKind.MINUS_TOKEN, "-"))],
    [NumberButton("0"), NumberButton("."), ControlButton("="), TokenButton("+", SyntaxToken(SyntaxKind.PLUS_TOKEN, "+"))],
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

    def visit_token_button(self, button):
        # what happens in the middle of a number node
        # what happens in the middle of nodes
        pass

    def visit_number_button(self, button):
        # what happens in the middle of a number node
        # what happens in the middle of nodes
        pass

    def visit_control_button(self, button):
        pass


    # def push_token(self, token):
    #     if self.pointer < len(self.tokens):
    #         self.tokens[self.pointer] = token
    #         self.pointer += 1
    #     else:
    #         raise IndexError("Token buffer overflow")

    # def visit_ui_state_button(self, button):
    #     print(f"UI State Button '{button.label}' clicked")

    # def visit_token_button(self, button):
    #     self.flush_number_buffer()
    #     self.push_token(button.token)

    # def visit_number_button(self, button):
    #     self.number_buffer += button.label

    # def visit_control_button(self, button):
    #     match button.label:
    #         case "=":
    #             self.flush_number_buffer()
    #             evaluation_result = Evaluator.from_tokens(self.tokens[:self.pointer]).evaluate()
    #             print(f"Evaluation result: {evaluation_result}")
    #             self.reset()

    # def flush_number_buffer(self):
    #     self.push_token(self.parse_number())

    # def parse_number(self):
    #     if not self.number_buffer:
    #         raise ValueError("No number to parse")
    #     if self.number_buffer.count(".") > 1:
    #         raise ValueError("Invalid number format: more than one decimal point")
            
    #     number = SyntaxToken(SyntaxKind.NUMBER_TOKEN, float(self.number_buffer) if "." in self.number_buffer else int(self.number_buffer))
    #     self.number_buffer = ""
    #     return number
    
    # def reset(self):
    #     self.tokens = [None] * 256
    #     self.pointer = 0
    #     self.number_buffer = ""