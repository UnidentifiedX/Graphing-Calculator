#pragma once

typedef enum {
    INBUILT_CONSTANT_INVALID = -1,
    INBUILT_CONSTANT_PI,
    INBUILT_CONSTANT_E
} InbuiltConstant;

InbuiltConstant get_constant_from_name(const char* name);
double get_constant(InbuiltConstant constant);