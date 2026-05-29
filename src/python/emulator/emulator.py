import pygame
from emulator.buttons import ButtonMatrix, Button, ButtonAction
from emulator.token_expression import TokenExpression, AtomicNode, NumberNode, FunctionNode, NodeType
from core.functions.functions import FunctionNames
from emulator.ui import UIPage, UIPageType
from emulator.pages.main_calculator_page import MainCalculatorPage

BUTTON_MATRIX = [
    [Button(ButtonAction.LEFT), Button(ButtonAction.RIGHT), Button(ButtonAction.FRACTION)],
    [Button(ButtonAction.SIN), Button(ButtonAction.COS), Button(ButtonAction.TAN), Button(ButtonAction.POWER)],
    [Button(ButtonAction.OPEN_PAREN), Button(ButtonAction.CLOSE_PAREN), Button(ButtonAction.AC), Button(ButtonAction.BKSP)],
    [Button(ButtonAction.SEVEN), Button(ButtonAction.EIGHT), Button(ButtonAction.NINE), Button(ButtonAction.DIVIDE)],
    [Button(ButtonAction.FOUR), Button(ButtonAction.FIVE), Button(ButtonAction.SIX), Button(ButtonAction.MULTIPLY)],
    [Button(ButtonAction.ONE), Button(ButtonAction.TWO), Button(ButtonAction.THREE), Button(ButtonAction.MINUS)],
    [Button(ButtonAction.ZERO), Button(ButtonAction.DECIMAL_POINT), Button(ButtonAction.EQUALS), Button(ButtonAction.PLUS)],
]

class Emulator:
    def __init__(self):
        pygame.init()
        self.screen = pygame.display.set_mode((400, 600))
        self.running = True
        self.button_matrix = ButtonMatrix(BUTTON_MATRIX)
        self.token_expression = TokenExpression()
        self.current_page = UIPageType.MAIN
        self.pages = {
            UIPageType.MAIN: MainCalculatorPage(),
        }

    def run(self):
        while self.running:
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    self.running = False
                    
                if event.type == pygame.MOUSEBUTTONDOWN:
                    button = self.button_matrix.handle_click(event.pos, self)
                    if button:
                        self.pages[self.current_page].handle_button_action(button.action)

            self.pages[self.current_page].render(self.screen)
            self.button_matrix.render(self.screen)

            pygame.display.flip()