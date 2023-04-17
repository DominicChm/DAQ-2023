#pragma once
#include <Arduino.h>
// https://stackoverflow.com/a/59522794/16238567
constexpr unsigned int hash(const char *s, int off = 0)
{
    return !s[off] ? 5381 : (hash(s, off + 1) * 33) ^ s[off];
}

constexpr unsigned int hash(uint8_t *s, size_t len, int off = 0)
{
    return off >= len ? 5381 : (hash(s, len, off + 1) * 33) ^ s[off];
}

template <typename T>
constexpr const char *t()
{
#ifdef _MSC_VER
    return __FUNCSIG__;
#else
    return __PRETTY_FUNCTION__;
#endif
}

/**
 * Generates a characteristic type name for the passed 
 * type using GCC's __PRETTY_FUNCTION__
 * This string should be parsable to get the actual type's name.
 */
template <typename T>
constexpr const char *characteristic_type_name()
{
    return t<T>();
}

/**
 * Copies a string to heap-allocated memory, and sets the passed pointer to said memory.
 * If heap memory already exists at str_dest, it is realloced.
 */
void str2heap(char **str_dest, const char *str)
{
    // Make size always at least 1.
    size_t s = str == nullptr ? 1 : strlen(str) + 1;

    // Set destination to new (or reallocated) memory
    *str_dest = (char *)realloc(*str_dest, s);

    // ensure a null-term in target string
    *str_dest[0] = '\0';

    // Don't copy string if it's null.
    if (str != nullptr)
        strlcpy(*str_dest, str, s);
}