#pragma once

#include <stddef.h>

typedef enum {
    INBUILT_FUNCTION_INVALID = -1,
    INBUILT_FUNCTION_SIN,
    INBUILT_FUNCTION_COS,
    INBUILT_FUNCTION_TAN,
    INBUILT_FUNCTION_LN,
    INBUILT_FUNCTION_LOG,
    INBUILT_FUNCTION_SQRT,
    INBUILT_FUNCTION_POW,
    INBUILT_FUNCTION_ABS,
    INBUILT_FUNCTION_NROOT,
    INBUILT_FUNCTION_EXP,
    INBUILT_FUNCTION_FRAC
} InbuiltFunction;

InbuiltFunction get_inbuilt_function_from_identifier(const char* name);
char* get_inbuilt_function_name(InbuiltFunction func);
size_t get_function_arity(InbuiltFunction func);