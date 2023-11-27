#pragma once

#include <cstdint>

struct Transform
{
    constexpr Transform()
    : row_offset(0)
    , col_offset(0)
    , index(0)
    {
        
    }

    constexpr Transform(
        std::size_t row_offset,
        std::size_t col_offset,
        std::size_t index
    )
    : row_offset(row_offset)
    , col_offset(col_offset)
    , index(index)
    {
        
    }

    std::size_t row_offset;

    std::size_t col_offset;

    /// @brief index of node in a D4 Cayley graph if the traversal method is IVRHRVRH
    std::size_t index;
    
};