from enum import Enum, auto
from core.lexer import SyntaxToken, SyntaxKind
from core.functions.functions import FunctionNames, BUILTIN_FUNCTION_STRING_MAP, FUNCTION_INFO

"""
Logic for the entire token expression editor now that i wrote working spaghetti but it's time for a refactor

The main idea is that, from now on, ANY operation on the buffer operates on the node to the right of the cursor, for both the node and number buffer. 
What this means is that there is no such thing as a cursor at the end of a number node; 
the cursor instead belongs to the next node, which is None if the cursor is at the end of the entire expression.

From now on there will only be one check: is_between_nodes. This is a more confusion invariant, because this encompasses being at the start of a
number OR being at an atomic/function node. If we want to check if the current node is a number node just check the type

This goes for deletion as well. It only deletes the current node, so if we want to delete the previous node, shift left then delete.
This means we need to hard-code some special rules though but it makes the logic much simpler

We also ENFORCE that by the end of all insertion operations, the cursor is ALWAYS one position before the the new intended cursor position
So it's kinda like we've only ever been working on the node before the new cursor position
"""
SPECIAL_DISPLAY_FUNCTIONS = {
    FunctionNames.SQRT,
    FunctionNames.EXP,
    FunctionNames.POW,
    FunctionNames.NROOT
}

class NodeType(Enum):
    NUMBER = auto()
    ATOM = auto()
    FUNCTION = auto()
    ENDOFARGUMENT = auto()

class Node:
    def __init__(self, type: NodeType):
        self.type = type

class NumberNode(Node):
    def __init__(self, num_str: str): # when translating to cpp avoid using std::string for this, just use a char array and a length variable
        super().__init__(NodeType.NUMBER)
        self.num_str = num_str

class AtomicNode(Node):
    def __init__(self, token: SyntaxToken):
        super().__init__(NodeType.ATOM)
        self.token = token

class FunctionNode(Node):
    def __init__(self, identifier: FunctionNames):
        super().__init__(NodeType.FUNCTION)
        self.identifier = identifier

class EndOfArgumentNode(Node):
    def __init__(self):
        super().__init__(NodeType.ENDOFARGUMENT)

class TokenExpression:
    def __init__(self):
        self.number_buffer = ""
        self.node_index: int = 0
        self.char_index: int = -1 # -1 if cursor is not referring to a number node
        self.node_buffer: list[Node | None] = [None] * 256
        self.size: int = 0 # number of nodes in the expression, not counting None nodes

    def shift_cursor(self, direction):
        """
        Direction: -1 for left, +1 for right
        """
        current_node = self.node_buffer[self.node_index]
        is_between_nodes = self._is_between_nodes()

        if direction == 1:
            if current_node is None: # at the end of the expression, can't move right
                return
            if is_between_nodes and self.node_index == len(self.node_buffer) - 1: # somehow reached the end of the buffer
                print("Somehow reached the end of the buffer, can't move right")
                return
            if is_between_nodes and current_node.type != NodeType.NUMBER:
                next_node = self.node_buffer[self.node_index + 1]
                if next_node and next_node.type == NodeType.NUMBER:
                    self.node_index += 1
                    self.char_index = 0
                else:
                    self.node_index += 1
                    self.char_index = -1
            elif current_node.type == NodeType.NUMBER:
                self.char_index += 1
                if self.char_index >= len(current_node.num_str) - 1:
                    self.char_index = -1
                    self.node_index += 1
            else:
                raise ValueError("Invalid node type")
        elif direction == -1:
            if self.node_index == 0 and is_between_nodes: # at the beginning of the entire expression, can't move left
                return
            if is_between_nodes:
                previous_node = self.node_buffer[self.node_index - 1]
                if previous_node.type == NodeType.NUMBER:
                    self.node_index -= 1
                    self.char_index = len(previous_node.num_str) - 1
                else:
                    self.node_index -= 1
            elif current_node.type == NodeType.NUMBER:
                self.char_index -= 1
    
    def insert_node(self, node: Node):
        if self.size >= len(self.node_buffer):
            raise IndexError("Node buffer overflow")
        
        current_node = self.node_buffer[self.node_index]

        if current_node is None:
            if node.type == NodeType.FUNCTION and node.identifier in SPECIAL_DISPLAY_FUNCTIONS:
                arity = FUNCTION_INFO[BUILTIN_FUNCTION_STRING_MAP[node.identifier.value]]["arity"]
                self.node_buffer[self.node_index] = node
                for i in range(0, arity):
                    self.node_buffer[self.node_index + 1 + i] = EndOfArgumentNode()
                self.size += (arity + 1)
            else:
                self.node_buffer[self.node_index] = node
                self.size += 1
            
            self.shift_cursor(1) # shift cursor to the right of the newly inserted node
            return
        
        is_between_nodes = self._is_between_nodes()

        match node.type:
            case NodeType.NUMBER:
                if is_between_nodes:
                    previous_node = self.node_buffer[self.node_index - 1] if self.node_index > 0 else None
                    if previous_node and previous_node.type == NodeType.NUMBER: # check if previous node is a number if it is just append
                        previous_node.num_str += node.num_str
                        return # cursor stays in the same position
                    elif current_node.type == NodeType.NUMBER: # check if current node is a number if it is just prepend
                        current_node.num_str = node.num_str + current_node.num_str
                    else:
                        self._insert_node_atomically(node)
                        self.char_index = 0
                else:
                    num_str = current_node.num_str
                    new_num_str = num_str[:self.char_index] + node.num_str + num_str[self.char_index:]
                    current_node.num_str = new_num_str
            case NodeType.ATOM:
                if is_between_nodes:
                    self._insert_node_atomically(node)
                else:
                    self._insert_atomic_node_within_number(current_node, node)
            case NodeType.FUNCTION:
                if is_between_nodes:
                    if node.identifier in SPECIAL_DISPLAY_FUNCTIONS:
                        arity = FUNCTION_INFO[BUILTIN_FUNCTION_STRING_MAP[node.identifier.value]]["arity"]

                        self._shift_tokens_right(self.node_index, arity + 1)
                        self.node_buffer[self.node_index] = node
                        for i in range(0, arity):
                            self.node_buffer[self.node_index + 1 + i] = EndOfArgumentNode()
                        self.size += (arity + 1)
                    else:
                        self._insert_node_atomically(node)
                else:
                    if node.identifier in SPECIAL_DISPLAY_FUNCTIONS:
                        arity = FUNCTION_INFO[BUILTIN_FUNCTION_STRING_MAP[node.identifier.value]]["arity"]
                        num_str = current_node.num_str
                        left_num_str = num_str[:self.char_index]
                        right_num_str = num_str[self.char_index:]
                        current_node.num_str = left_num_str # current number node is the left part of the split number
                        right_number_node = NumberNode(right_num_str)

                        self.node_index += 1 # move node index to the right of the current node
                        self._shift_tokens_right(self.node_index, arity + 2) # arity + function node + right number node
                        self.node_buffer[self.node_index] = node
                        for i in range(0, arity):
                            self.node_buffer[self.node_index + 1 + i] = EndOfArgumentNode()
                        self.node_buffer[self.node_index + arity + 1] = right_number_node
                        self.size += (arity + 2)
                    else:
                        self._insert_atomic_node_within_number(current_node, node)
            case _:
                raise ValueError("Invalid node type")
            
        self.shift_cursor(1) # shift cursor to the right to reset all positional invariants

    def delete_node(self): # fix having to join two numbers together when trying to delete
        current_node = self.node_buffer[self.node_index]

        if self.size == 0:
            return # nothing to delete
        if current_node is None:
            return # nothing to delete
        
        is_between_nodes = self._is_between_nodes()
        if is_between_nodes:
            if current_node.type == NodeType.NUMBER and len(current_node.num_str) != 1:
                current_node.num_str = current_node.num_str[1:] # delete the first character of the number
            elif current_node.type == NodeType.FUNCTION:
                if current_node.identifier in SPECIAL_DISPLAY_FUNCTIONS:
                    original_node_index = self.node_index
                    depth_counter = 0
                    i = self.node_index + 1
                    while i < self.size:
                        node = self.node_buffer[i]
                        if node.type == NodeType.FUNCTION and node.identifier in SPECIAL_DISPLAY_FUNCTIONS:
                            depth_counter += FUNCTION_INFO[BUILTIN_FUNCTION_STRING_MAP[node.identifier.value]]["arity"]
                        elif node.type == NodeType.ENDOFARGUMENT:
                            if depth_counter == 0:
                                self.node_index = i
                                self._delete_node_atomically()
                                continue
                            # basically once everything is shifted we can't update the node index just continue from here
                            depth_counter -= 1                                
                        i += 1
                    if depth_counter != 0:
                        raise ValueError("Invalid state: mismatched function and end of argument nodes")
                    self.node_index = original_node_index
                    self._delete_node_atomically()
                else:
                    self._delete_node_atomically()
            else:
                self._delete_node_atomically()
        else:
            current_node.num_str = current_node.num_str[:self.char_index] + current_node.num_str[self.char_index + 1:]

    def backspace_node(self):
        self.shift_cursor(-1)
        self.delete_node()

    def _is_between_nodes(self):
        return self.char_index == -1 or self.char_index == 0
    
    def _shift_tokens_right(self, index: int, places: int):
        for i in range(self.size - 1, index - 1, -1):
            self.node_buffer[i + places] = self.node_buffer[i]

    def _shift_tokens_left(self, index: int, places: int):
        for i in range(index, self.size):
            self.node_buffer[i - places] = self.node_buffer[i]

    def _insert_node_atomically(self, node): # shift everything to the right and insert the node at the current index
        self._shift_tokens_right(self.node_index, 1)
        self.node_buffer[self.node_index] = node
        self.size += 1
        self.char_index = -1 # ensure that the cursor is between nodes after an atomic insertion

    def _insert_atomic_node_within_number(self, current_node, node):
        num_str = current_node.num_str
        left_num_str = num_str[:self.char_index]
        right_num_str = num_str[self.char_index:]
        current_node.num_str = left_num_str # current number node is the left part of the split number
        right_number_node = NumberNode(right_num_str)

        self.node_index += 1 # move node index to the right of the current node
        self._shift_tokens_right(self.node_index, 2) # shift all nodes two positions to the right
        self.node_buffer[self.node_index] = node
        self.node_buffer[self.node_index + 1] = right_number_node
        self.size += 2

    def _delete_node_atomically(self):
        self._shift_tokens_left(self.node_index + 1, 1)
        self.size -= 1
        new_current_node = self.node_buffer[self.node_index]
        previous_node = self.node_buffer[self.node_index - 1] if self.node_index > 0 else None
        if new_current_node and new_current_node.type == NodeType.NUMBER: # reset indexes
            if previous_node and previous_node.type == NodeType.NUMBER:
                previous_node_length = len(previous_node.num_str)
                new_current_node.num_str = previous_node.num_str + new_current_node.num_str
                self._shift_tokens_left(self.node_index, 1)
                self.size -= 1
                self.node_index -= 1
                self.char_index = previous_node_length
            else:
                self.char_index = 0
        else:
            self.char_index = -1