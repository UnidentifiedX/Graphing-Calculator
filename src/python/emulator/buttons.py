import pygame
from core.symbols import SyntaxKind
from core.lexer import SyntaxToken 
from emulator.token_expression import AtomicNode, NumberNode, FunctionNode
from core.functions.functions import FunctionNames

class Button:
    def __init__(self, label):
        self.label: str = label
        self.rect: pygame.Rect = None

    def accept(self, visitor):
        raise NotImplementedError("Subclasses must implement this method")
    
    def create_node(self):
        raise NotImplementedError("Subclasses must implement this method")

class UIStateButton(Button):
    def __init__(self, label):
        super().__init__(label)

    def accept(self, visitor):
        return visitor.visit_ui_state_button(self)
    
    def create_node(self):
        raise NotImplementedError("UIStateButton does not create a node")
    
class AtomicTokenButton(Button):
    def __init__(self, label: str, kind: SyntaxKind):
        super().__init__(label)
        self.kind = kind

    def accept(self, visitor):
        return visitor.visit_token_button(self)
    
    def create_node(self):
        return AtomicNode(SyntaxToken(self.kind, None))

class NumberTokenButton(Button):
    def __init__(self, label: str):
        super().__init__(label)

    def accept(self, visitor):
        return visitor.visit_token_button(self)

    def create_node(self):
        return NumberNode(self.label)

class FunctionTokenButton(Button):
    def __init__(self, label: str, function_name: FunctionNames):
        super().__init__(label)
        self.function_name = function_name

    def accept(self, visitor):
        return visitor.visit_token_button(self)

    def create_node(self):
        return FunctionNode(self.function_name)
    
class ControlButton(Button):
    def __init__(self, label):
        super().__init__(label)

    def accept(self, visitor):
        return visitor.visit_control_button(self)
    
    def create_node(self):
        raise NotImplementedError("ControlButton does not create a node")
    
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
                    # print(f"Clicked button: {button.label}")
                    button.accept(visitor)