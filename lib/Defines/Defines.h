#pragma once

#include <stdint.h>
// all the defines will be stored here
constexpr int BUFSIZE = 600;

template<typename T, typename V>
struct Pair
{
    Pair(T f, V s) : first(f), second(s) { }
    T first;
    V second;
};
