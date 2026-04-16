from core.constants import LN2, INV_LN2
import math

def exp(x): # TODO: use minimax approximation for better accuracy
    k = int(x * INV_LN2 + (0.5 if x >= 0 else -0.5))
    r = x - k * LN2
    exp_r = 1 + r * (1 + r * (0.5 + r * (1/6 + r * (1/24 + r * (1/120 + r * (1/720)))))) # Taylor expansion for exp(r)

    return math.ldexp(exp_r, k)