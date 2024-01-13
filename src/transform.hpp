#pragma once
#include <cstdint>

struct Transform
{
    constexpr Transform()
    : row_offset(0), col_offset(0), index(0)
    { }

    constexpr Transform(size_t row_offset, size_t col_offset, size_t index)
    : row_offset(row_offset), col_offset(col_offset), index(index)
    { }

    /// @brief row offset.
    size_t row_offset;

    /// @brief column offset.
    size_t col_offset;

    /// @brief D4 Cayley graph node index, must be in range 0 to 7.
    /// The traversal method applies these transforms in sequence: I, V, R, H, R, V, R, H.
    /// Example: transform with index 4 would represent a rotation of 180 degrees.
    /// I - identity transform,
    /// R - clockwise rotation of 90 degrees,
    /// V - flip about vertical axis,
    /// H - flip about horizontal axis
    size_t index;
};