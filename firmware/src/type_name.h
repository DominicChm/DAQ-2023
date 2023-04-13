#pragma once

// https://stackoverflow.com/a/59522794/16238567
constexpr unsigned int hash(const char *s, int off = 0)
{
    return !s[off] ? 5381 : (hash(s, off + 1) * 33) ^ s[off];
}
template <typename T>
constexpr const char *type_name()
{
#ifdef _MSC_VER
    return __FUNCSIG__;
#else
    return __PRETTY_FUNCTION__;
#endif
}

/**
 * Constexpr that evaluates to a unique numeric ID for a given type.
 */
template <typename T>
constexpr unsigned int id()
{
    return hash(type_name<T>());
}

template <typename T>
constexpr unsigned int id(T &arg)
{
    return hash(type_name<T>());
}
