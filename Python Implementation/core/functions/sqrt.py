# Newton-Raphson
def sqrt(a):
    if a < 0:
        raise ValueError("Cannot compute square root of a negative number")
    
    x = a / 2.0
    for _ in range(10):
        x = 0.5 * (x + a / x)
    return x