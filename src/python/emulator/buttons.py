import pygame
from core.symbols import SyntaxKind
from core.lexer import SyntaxToken 
from emulator.token_expression import AtomicNode, NumberNode, FunctionNode
from core.functions.functions import FunctionNames
from enum import Enum, auto

class ButtonAction(Enum):
    AC = auto()
    DEL = auto()
    BKSP = auto()
    EQUALS = auto()
    ZERO = auto()
    ONE = auto()
    TWO = auto()
    THREE = auto()
    FOUR = auto()
    FIVE = auto()
    SIX = auto()
    SEVEN = auto()
    EIGHT = auto()
    NINE = auto()
    DECIMAL_POINT = auto()
    PLUS = auto()
    MINUS = auto()
    MULTIPLY = auto()
    DIVIDE = auto()
    OPEN_PAREN = auto()
    CLOSE_PAREN = auto()
    SIN = auto()
    COS = auto()
    TAN = auto()
    POWER = auto()
    FRACTION = auto()
    UP = auto()
    DOWN = auto()
    LEFT = auto()
    RIGHT = auto()

BUTTON_LABEL_MAP = {
    ButtonAction.AC: "AC",
    ButtonAction.DEL: "DEL",
    ButtonAction.BKSP: "BKSP",
    ButtonAction.EQUALS: "=",
    ButtonAction.ZERO: "0",
    ButtonAction.ONE: "1",
    ButtonAction.TWO: "2",
    ButtonAction.THREE: "3",
    ButtonAction.FOUR: "4",
    ButtonAction.FIVE: "5",
    ButtonAction.SIX: "6",
    ButtonAction.SEVEN: "7",
    ButtonAction.EIGHT: "8",
    ButtonAction.NINE: "9",
    ButtonAction.DECIMAL_POINT: ".",
    ButtonAction.PLUS: "+",
    ButtonAction.MINUS: "-",
    ButtonAction.MULTIPLY: "*",
    ButtonAction.DIVIDE: "/",
    ButtonAction.OPEN_PAREN: "(",
    ButtonAction.CLOSE_PAREN: ")",
    ButtonAction.SIN: "sin",
    ButtonAction.COS: "cos",
    ButtonAction.TAN: "tan",
    ButtonAction.POWER: "^",
    ButtonAction.FRACTION: "frac",
    ButtonAction.UP: "UP",
    ButtonAction.DOWN: "DOWN",
    ButtonAction.LEFT: "LEFT",
    ButtonAction.RIGHT: "RIGHT",
}

class Button:
    def __init__(self, action: ButtonAction):
        self.action: ButtonAction = action
        self.rect: pygame.Rect = None
    
class ButtonMatrix:
    def __init__(self, button_matrix: list[list[Button]]):
        self.button_matrix = button_matrix
        self.font = pygame.font.SysFont(None, 24)
    
    def render(self, screen: pygame.Surface):
            # 1. Layout Boundaries
            screen_width, screen_height = screen.get_size()
            top_reserve = 200  # Leave the top 200px blank for the calculator display
            grid_height = screen_height - top_reserve  # The buttons get the remaining 400px

            num_rows = len(self.button_matrix)
            # some rows might have different number of columns, so we take the max
            num_cols = max(len(row) for row in self.button_matrix) if num_rows > 0 else 1
            
            margin = 8
            button_width = (screen_width - (margin * (num_cols + 1))) // num_cols
            button_height = (grid_height - (margin * (num_rows + 1))) // num_rows

            for r, row in enumerate(self.button_matrix):
                for c, button in enumerate(row):
                    x = c * (button_width + margin) + margin
                    # Y coordinate gets shifted down by top_reserve (200px)
                    y = top_reserve + (r * (button_height + margin) + margin)

                    button.rect = pygame.Rect(x, y, button_width, button_height)
                    
                    # Draw button background (soft gray)
                    pygame.draw.rect(screen, (220, 220, 220), button.rect, border_radius=4)
                    # Optional: Add a subtle border around buttons
                    pygame.draw.rect(screen, (160, 160, 160), button.rect, width=1, border_radius=4)

                    # Render and center text
                    text_surface = self.font.render(BUTTON_LABEL_MAP[button.action], True, (0, 0, 0))
                    text_rect = text_surface.get_rect(center=button.rect.center)

                    screen.blit(text_surface, text_rect)

    def handle_click(self, pos, visitor) -> Button:
        for row in self.button_matrix:
            for button in row:
                if button.rect and button.rect.collidepoint(pos):
                    return button
        return None