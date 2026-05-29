from core.lexer import SyntaxKind
from core.evaluator import Evaluator
from emulator.ui import UIPage, action_to_ui_token
from emulator.buttons import ButtonAction
from emulator.token_expression import TokenExpression, NodeType
from pygame import Surface

class MainCalculatorPage(UIPage):
    def __init__(self):
        super().__init__()
        self.token_expression = TokenExpression()
        self.result = None

    def render(self, screen: Surface):
        screen.fill((255, 255, 255))
        expression_str = self._format_expression()
        expression_surface = self.font.render(expression_str, True, (0, 0, 0))
        screen.blit(expression_surface, (10, 10))

        if self.result is not None:
            result_surface = self.font.render(f"= {self.result}", True, (0, 0, 255))
            screen.blit(result_surface, (10, 50))

    def _format_expression(self):
        out = []
        cursor_node_position = self.token_expression.node_index
        cursor_char_position = self.token_expression.char_index

        # print(f"Cursor node index: {cursor_node_position}, char index: {cursor_char_position}") # debug

        for i, node in enumerate(self.token_expression.node_buffer[:self.token_expression.size]):
            if i == cursor_node_position and cursor_char_position == -1:
                out.append("|")  # Cursor is between nodes

            if node.type == NodeType.ATOM:
                match node.token.kind:
                    case SyntaxKind.PLUS_TOKEN: out.append("+")
                    case SyntaxKind.MINUS_TOKEN: out.append("-")
                    case SyntaxKind.MULTIPLY_TOKEN: out.append("*")
                    case SyntaxKind.DIVIDE_TOKEN: out.append("/")
                    case SyntaxKind.OPEN_PAREN_TOKEN: out.append("(")
                    case SyntaxKind.CLOSE_PAREN_TOKEN: out.append(")")
                    case SyntaxKind.FACTORIAL_TOKEN: out.append("!")
                    case SyntaxKind.PERCENTAGE_TOKEN: out.append("%")
            elif node.type == NodeType.NUMBER:
                if cursor_char_position != -1 and i == cursor_node_position: # Cursor is within this number node
                    num_str = node.num_str
                    cursor_pos = cursor_char_position
                    num_str_with_cursor = num_str[:cursor_pos] + "|" + num_str[cursor_pos:]
                    out.append(num_str_with_cursor)
                else:
                    out.append(node.num_str)
            elif node.type == NodeType.FUNCTION:
                out.append(node.identifier.value + "(")
            elif node.type == NodeType.ENDOFARGUMENT:
                out.append(",")

        if cursor_node_position == self.token_expression.size and cursor_char_position == -1:
            out.append("|")  # Cursor at the very end
        
        return "".join(out)

    def handle_button_action(self, action: ButtonAction):
        if action == ButtonAction.AC:
            self.token_expression.clear_buffer()
        elif action == ButtonAction.BKSP:
            self.token_expression.backspace_node()
        elif action == ButtonAction.EQUALS:
            if self.token_expression.size == 0:
                return

            tokens = self.token_expression.generate_token_list()
            evaluator = Evaluator.from_tokens(tokens)
            self.result = evaluator.evaluate()
            self.token_expression.clear_buffer()
        elif action == ButtonAction.LEFT:
            self.token_expression.shift_cursor(-1)
        elif action == ButtonAction.RIGHT:
            self.token_expression.shift_cursor(1)
        else:
            node = action_to_ui_token(action)
            self.token_expression.insert_node(node)