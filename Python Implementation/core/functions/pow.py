from .exp import exp
from .ln import ln

def pow(x, y):
    if y == 0:
        return 1.0
    if y == 1:
        return x
    if y == 2:
        return x * x
    if x == 0:
        if y < 0:
            raise ValueError("0 cannot be raised to a negative power")
        return 0
    if x == 1:
        return 1.0

    return exp(y * ln(x))