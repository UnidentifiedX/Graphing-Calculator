#pragma once
#include <array>
#include <string_view>
#include <algorithm>

/**
 * @brief Fixed-size string class that hold up to 15 characters (plus a null terminator)
 * 
 * Point is that there's no heap allocation
 * 
 * 15 because variables and functions shouldn't have that long a name will they
 */
struct FixedString16 {
    std::array<char, 16> buffer = {}; // maximum 15 characters + null terminator
    size_t length = 0;

    FixedString16() = default;
    FixedString16(std::string_view str) {
        length = std::min<size_t>(str.size(), buffer.size() - 1);
        std::copy_n(str.data(), length, buffer.data());
        buffer[length] = '\0'; // null terminator
    }
    // FixedString16(const FixedString16& other) {
    //     buffer = other.buffer;
    //     length = other.length;
    // }

    const char* c_str() const {
        return buffer.data();
    }
};