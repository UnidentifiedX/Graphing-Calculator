#include "virtual_keypad.h"

Keystroke char_to_keystroke(char c) {
    switch (c) {
        case '0': return KEYSTROKE_0;
        case '1': return KEYSTROKE_1;
        case '2': return KEYSTROKE_2;
        case '3': return KEYSTROKE_3;
        case '4': return KEYSTROKE_4;
        case '5': return KEYSTROKE_5;
        case '6': return KEYSTROKE_6;
        case '7': return KEYSTROKE_7;
        case '8': return KEYSTROKE_8;
        case '9': return KEYSTROKE_9;
        case '+': return KEYSTROKE_PLUS;
        case '-': return KEYSTROKE_MINUS;
        case '*': return KEYSTROKE_MULTIPLY;
        case '/': return KEYSTROKE_DIVIDE;
        case '.': return KEYSTROKE_DECIMAL_POINT;
        case '^': return KEYSTROKE_POWER;
        case '(': return KEYSTROKE_OPEN_PARENTHESIS;
        case ')': return KEYSTROKE_CLOSE_PARENTHESIS;
        case '=': return KEYSTROKE_ENTER;
        case 'C': return KEYSTROKE_CLEAR;
        case 'B': return KEYSTROKE_BACKSPACE;
        case 'L': return KEYSTROKE_LEFT;
        case 'R': return KEYSTROKE_RIGHT;
        case 's': return KEYSTROKE_SIN;
        case 'c': return KEYSTROKE_COS;
        case 't': return KEYSTROKE_TAN;
        case 'l': return KEYSTROKE_LN;
        case 'g': return KEYSTROKE_LOG10;
        case 'p': return KEYSTROKE_PI;
        default:  return KEYSTROKE_INVALID; // Invalid keystroke
    }
}