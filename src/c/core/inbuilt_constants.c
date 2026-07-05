#include "inbuilt_constants.h"
#include <math.h>
#include <string.h>

InbuiltConstant get_constant_from_name(const char* name) {
    if (strcmp(name, "pi") == 0) return INBUILT_CONSTANT_PI;
    if (strcmp(name, "e") == 0) return INBUILT_CONSTANT_E;

    return INBUILT_CONSTANT_INVALID;
}

double get_constant(InbuiltConstant constant) {
    switch (constant) {
        case INBUILT_CONSTANT_PI:
            return M_PI;
        case INBUILT_CONSTANT_E:
            return M_E;
        default:
            return NAN;
    }
}