from core.functions.ln import ln
from core.constants import INV_LN10

def log(x):
    if x <= 0:
        raise ValueError("Cannot compute logarithm of non-positive numbers")
    
    # Use change of base formula: ln(x) = log2(x) / log2(e)
    return ln(x) * INV_LN10