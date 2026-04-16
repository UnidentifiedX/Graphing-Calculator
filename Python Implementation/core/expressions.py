class Expression:
    def accept(self):
        raise NotImplementedError("Accept method not implemented in base Expression class")

class NumberExpression(Expression):
    def __init__(self, value):
        self.value = value

    def accept(self, visitor):
        return visitor.visit_number(self)

class VariableExpression(Expression):
    def __init__(self, name):
        self.name = name

    def accept(self, visitor):
        return visitor.visit_variable(self)

class UnaryExpression(Expression):
    def __init__(self, operator, operand):
        self.operator = operator
        self.operand = operand

    def accept(self, visitor):
        return visitor.visit_unary(self)

class PostfixExpression(Expression):
    def __init__(self, operand, operator):
        self.operand = operand
        self.operator = operator

    def accept(self, visitor):
        return visitor.visit_postfix(self)

class BinaryExpression(Expression):
    def __init__(self, left, operator, right):
        self.left = left
        self.operator = operator
        self.right = right

    def accept(self, visitor):
        return visitor.visit_binary(self)

class CallExpression(Expression):
    def __init__(self, identifier, arguments):
        self.identifier = identifier
        self.arguments = arguments

    def accept(self, visitor):
        return visitor.visit_call(self)