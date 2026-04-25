from core.constants import LN2

def ln(x): # TODO: use Cherbyshev approxmiation in the future
    if x <= 0:
        raise ValueError("Cannot compute logarithm of non-positive numbers")
    if x == 1:
        return 0
    if x == 2:
        return LN2
    
    # reduce range (this is done way easier in c++ with frexp but thats only done when porting to c++ so whatever)
    k = 0
    while x >= 2:
        x *= 0.5
        k += 1
    while x < 1:
        x *= 2
        k -= 1

    y = (x - 1) / (x + 1)
    ln_m = 2 * y * (1 + 0.3333333333333333 * y * y + 0.2 * y**4 + 0.14285714285714285 * y**6 + 0.1111111111111111 * y**8) # Taylor expansion for ln(x) around 1
    return ln_m + k * LN2

# sum chatgpt slop i might analyse later
# from constants import LN2
# import math

# # minimax-like truncated atanh series (good tradeoff)
# def _ln_kernel(m):
#     z = (m - 1) / (m + 1)
#     z2 = z * z

#     # Horner-style evaluation for stability
#     return 2 * z * (
#         1 +
#         z2 * (
#             1/3 +
#             z2 * (
#                 1/5 +
#                 z2 * (
#                     1/7 +
#                     z2 * (1/9)
#                 )
#             )
#         )
#     )


# def ln(x):
#     if x <= 0:
#         raise ValueError("ln(x) undefined for x <= 0")

#     # handle very small edge cases cleanly
#     if x == 1:
#         return 0.0

#     # OPTIONAL: fast path for close-to-1 values (improves precision)
#     if 0.75 < x < 1.25:
#         return _ln_kernel(x)

#     # --- range reduction ---
#     k = 0

#     # bring x into [1, 2)
#     while x >= 2.0:
#         x *= 0.5
#         k += 1

#     while x < 1.0:
#         x *= 2.0
#         k -= 1

#     # kernel
#     return k * LN2 + _ln_kernel(x)