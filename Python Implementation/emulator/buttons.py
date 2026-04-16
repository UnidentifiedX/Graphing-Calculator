import pygame
from core.symbols import SyntaxKind
from core.lexer import SyntaxToken

class Button:
    def __init__(self, label):
        self.label: str = label
        self.rect: pygame.Rect = None

    def accept(self, visitor):
        raise NotImplementedError("Subclasses must implement this method")

class UIStateButton(Button):
    def __init__(self, label):
        super().__init__(label)

    def accept(self, visitor):
        return visitor.visit_ui_state_button(self)
    
class TokenButton(Button):
    def __init__(self, label, token):
        super().__init__(label)
        self.token: SyntaxToken = token

    def accept(self, visitor):
        return visitor.visit_token_button(self)

class NumberButton(Button):
    def __init__(self, label):
        super().__init__(label)

    def accept(self, visitor):
        return visitor.visit_number_button(self)
    
class ControlButton(Button):
    def __init__(self, label):
        super().__init__(label)

    def accept(self, visitor):
        return visitor.visit_control_button(self)
    
class ButtonMatrix:
    def __init__(self, button_matrix: list[list[Button]]):
        self.button_matrix = button_matrix
        self.font = pygame.font.SysFont(None, 24)
    
    def render(self, screen):
        button_height = 40
        button_width = 50
        margin = 10

        for r, row in enumerate(self.button_matrix):
            for c, button in enumerate(row):
                x = c * (button_width + margin) + margin
                y = r * (button_height + margin) + margin

                button.rect = pygame.Rect(x, y, button_width, button_height)
                pygame.draw.rect(screen, (200, 200, 200), button.rect)

                text_surface = self.font.render(button.label, True, (0, 0, 0))
                text_rect = text_surface.get_rect(center=button.rect.center)

                screen.blit(text_surface, text_rect)

    def handle_click(self, pos, visitor):
        for row in self.button_matrix:
            for button in row:
                if button.rect and button.rect.collidepoint(pos):
                    button.accept(visitor)