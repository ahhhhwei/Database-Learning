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

template <typename T>
inline void printContainerOptional(const T &container)
{
    for (const auto &e : container)
    {
        if (e)
            std::cout << e.value() << std::endl;
        else
            std::cout << "element is not valued" << std::endl;
    }
}