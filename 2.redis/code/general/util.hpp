#pragma once
#include <vector>
#include <string>
#include <iostream>
template <typename T>

inline void printContainer(const T &container)
{
    for (const auto &e : container)
    {
        std::cout << e << std::endl;
    }
}