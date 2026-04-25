from core.functions.pow import pow

def nroot(x, n): # Might change to Newton-Raphson if this is too imprecise
    if n <= 0:
        raise ValueError("n must be a positive integer")
    if x < 0 and n % 2 == 0:
        raise ValueError("Cannot compute even root of a negative number")
    if x == 0:
        return 0.0
    if n == 1:
        return x

    return pow(x, 1.0 / n)