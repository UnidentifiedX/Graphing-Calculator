#include "inbuilt_functions.h"
#include <stddef.h>
#include <string.h>

InbuiltFunction get_inbuilt_function_from_enum(const char* name) {
    if (strcmp(name, "sin") == 0)   return INBUILT_FUNCTION_SIN;
    if (strcmp(name, "cos") == 0)   return INBUILT_FUNCTION_COS;
    if (strcmp(name, "tan") == 0)   return INBUILT_FUNCTION_TAN;
    if (strcmp(name, "ln") == 0)    return INBUILT_FUNCTION_LN;
    if (strcmp(name, "log") == 0)   return INBUILT_FUNCTION_LOG;
    if (strcmp(name, "sqrt") == 0)  return INBUILT_FUNCTION_SQRT;
    if (strcmp(name, "pow") == 0)   return INBUILT_FUNCTION_POW;
    if (strcmp(name, "abs") == 0)   return INBUILT_FUNCTION_ABS;
    if (strcmp(name, "nroot") == 0) return INBUILT_FUNCTION_NROOT;

    return INBUILT_FUNCTION_INVALID; // Invalid function
}

char* get_inbuilt_function_name(InbuiltFunction func) {
    switch (func) {
        case INBUILT_FUNCTION_SIN:   return "sin";
        case INBUILT_FUNCTION_COS:   return "cos";
        case INBUILT_FUNCTION_TAN:   return "tan";
        case INBUILT_FUNCTION_LN:    return "ln";
        case INBUILT_FUNCTION_LOG:   return "log";
        case INBUILT_FUNCTION_SQRT:  return "sqrt";
        case INBUILT_FUNCTION_POW:   return "pow";
        case INBUILT_FUNCTION_ABS:   return "abs";
        case INBUILT_FUNCTION_NROOT: return "nroot";
        default:                     return NULL; // Invalid function
    }
}

size_t get_function_arity(InbuiltFunction func) {
    switch (func) {
        case INBUILT_FUNCTION_SIN:
        case INBUILT_FUNCTION_COS:
        case INBUILT_FUNCTION_TAN:
        case INBUILT_FUNCTION_LN:
        case INBUILT_FUNCTION_LOG:
        case INBUILT_FUNCTION_SQRT:
        case INBUILT_FUNCTION_ABS:
        case INBUILT_FUNCTION_EXP:
            return 1;
        case INBUILT_FUNCTION_POW:
        case INBUILT_FUNCTION_NROOT:
        case INBUILT_FUNCTION_FRAC:
            return 2;
        default:
            return 0; // Invalid function or unknown arity
    }
}