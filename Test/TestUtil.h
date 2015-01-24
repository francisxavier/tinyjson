#pragma once

#include <stdexcept>

template <class T>
void CheckEqual(const T &value1, const T &value2)
{
    if( value1 != value2 )
        throw std::runtime_error("CheckEqual failed");
}

template <class T>
void CheckNotEqual(const T &value1, const T &value2)
{
    if( value1 == value2 )
        throw std::runtime_error("CheckNotEqual failed");
}

inline void Check(const bool flag)
{
    if( !flag )
        throw std::runtime_error("Check failed");
}

template <class T>
void CheckNotNull(const T *const ptr)
{
    if( ptr == nullptr )
        throw std::runtime_error("CheckNotNull failed");
}
