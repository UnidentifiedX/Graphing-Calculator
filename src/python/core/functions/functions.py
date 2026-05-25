from enum import Enum, auto

from core.functions.abs import abs
from core.functions.sqrt import sqrt
from core.functions.exp import exp
from core.functions.ln import ln
from core.functions.log import log
from core.functions.pow import pow
from core.functions.nroot import nroot
from core.functions.sin import sin
from core.functions.cos import cos
from core.functions.tan import tan

class FunctionType(Enum):
    ABS = auto()
    SQRT = auto()
    EXP = auto()
    LN = auto()
    LOG = auto()
    POW = auto()
    NROOT = auto()
    SIN = auto()
    COS = auto()
    TAN = auto()

FUNCTION_INFO = {
    FunctionType.ABS: {"name": "abs", "arity": 1, "func": abs},
    FunctionType.SQRT: {"name": "sqrt", "arity": 1, "func": sqrt},
    FunctionType.EXP: {"name": "exp", "arity": 1, "func": exp},
    FunctionType.LN: {"name": "ln", "arity": 1, "func": ln},
    FunctionType.LOG: {"name": "log", "arity": 2, "func": log},
    FunctionType.POW: {"name": "pow", "arity": 2, "func": pow},
    FunctionType.NROOT: {"name": "nroot", "arity": 2, "func": nroot},
    FunctionType.SIN: {"name": "sin", "arity": 1, "func": sin},
    FunctionType.COS: {"name": "cos", "arity": 1, "func": cos},
    FunctionType.TAN: {"name": "tan", "arity": 1, "func": tan},
}

class FunctionNames(Enum):
    ABS = "abs"
    SQRT = "sqrt"
    EXP = "exp"
    LN = "ln"
    LOG = "log"
    POW = "pow"
    NROOT = "nroot"
    SIN = "sin"
    COS = "cos"
    TAN = "tan"

BUILTIN_FUNCTION_STRING_MAP = {
    FunctionNames.ABS.value: FunctionType.ABS,
    FunctionNames.SQRT.value: FunctionType.SQRT,
    FunctionNames.EXP.value: FunctionType.EXP,
    FunctionNames.LN.value: FunctionType.LN,
    FunctionNames.LOG.value: FunctionType.LOG,
    FunctionNames.POW.value: FunctionType.POW,
    FunctionNames.NROOT.value: FunctionType.NROOT,
    FunctionNames.SIN.value: FunctionType.SIN,
    FunctionNames.COS.value: FunctionType.COS,
    FunctionNames.TAN.value: FunctionType.TAN,
}