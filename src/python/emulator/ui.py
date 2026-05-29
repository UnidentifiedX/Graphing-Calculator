from enum import Enum, auto
from emulator.token_expression import TokenExpression
from emulator.buttons import Button, ButtonAction
from emulator.token_expression import Node, AtomicNode, NumberNode, FunctionNode, NodeType
from core.lexer import SyntaxToken, SyntaxKind
from core.functions.functions import FunctionNames
from core.evaluator import Evaluator
import pygame
from pygame import Surface

def action_to_ui_token(action: ButtonAction) -> Node:
    match action:
        case ButtonAction.ZERO:
            return NumberNode("0")
        case ButtonAction.ONE:
            return NumberNode("1")
        case ButtonAction.TWO:
            return NumberNode("2")
        case ButtonAction.THREE:
            return NumberNode("3")
        case ButtonAction.FOUR:
            return NumberNode("4")
        case ButtonAction.FIVE:
            return NumberNode("5")
        case ButtonAction.SIX:
            return NumberNode("6")
        case ButtonAction.SEVEN:
            return NumberNode("7")
        case ButtonAction.EIGHT:
            return NumberNode("8")
        case ButtonAction.NINE:
            return NumberNode("9")
        case ButtonAction.DECIMAL_POINT:
            return NumberNode(".")
        case ButtonAction.PLUS:
            return AtomicNode(SyntaxToken(SyntaxKind.PLUS_TOKEN, None))
        case ButtonAction.MINUS:
            return AtomicNode(SyntaxToken(SyntaxKind.MINUS_TOKEN, None))
        case ButtonAction.MULTIPLY:
            return AtomicNode(SyntaxToken(SyntaxKind.MULTIPLY_TOKEN, None))
        case ButtonAction.DIVIDE:
            return AtomicNode(SyntaxToken(SyntaxKind.DIVIDE_TOKEN, None))
        case ButtonAction.OPEN_PAREN:
            return AtomicNode(SyntaxToken(SyntaxKind.OPEN_PAREN_TOKEN, None))
        case ButtonAction.CLOSE_PAREN:
            return AtomicNode(SyntaxToken(SyntaxKind.CLOSE_PAREN_TOKEN, None))
        case ButtonAction.SIN:
            return FunctionNode(FunctionNames.SIN)
        case ButtonAction.COS:
            return FunctionNode(FunctionNames.COS)
        case ButtonAction.TAN:
            return FunctionNode(FunctionNames.TAN)
        case ButtonAction.POWER:
            return FunctionNode(FunctionNames.POW)
        case ButtonAction.FRACTION:
            return FunctionNode(FunctionNames.FRAC)
        case _:
            raise ValueError(f"Unrecognized button action: {action}")

class UIPageType(Enum):
    MAIN = auto()
    SETTINGS = auto()
    GRAPH = auto()

class UIPage:
    def __init__(self):
        self.font = pygame.font.SysFont("Consolas", 24)

    def render(self, screen: Surface):
        raise NotImplementedError("Subclasses must implement this method")

    def handle_button_action(self, action: ButtonAction):
        raise NotImplementedError("Subclasses must implement this method")