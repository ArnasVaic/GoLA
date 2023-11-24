#pragma once

#include <bit>
#include <cstdint>
#include <array>
#include <limits>
#include <ostream>

class Transform
{

public:

    constexpr Transform() 
    : row_offset(0)
    , col_offset(0)
    , turn_count(0)
    , flip(false)
    {

    }

    std::size_t row_offset;
    std::size_t col_offset;
    std::size_t turn_count;
    bool flip;
};

/// @brief
/// @tparam Ts width and height of the frame 
template <std::size_t Ts>
requires(Ts <= 8)
class Frame 
{

private:

    uint64_t m_state;

public:

    constexpr static std::size_t CellCount = Ts * Ts;

public:

    constexpr Frame(uint64_t state) 
    {
        m_state = state;
    }

    [[nodiscard]]
    constexpr static std::size_t to_index(std::size_t row, std::size_t col)
    {
        return col + row * Ts;
    }

    [[nodiscard]]
    constexpr uint64_t get() const
    {
        return m_state;
    }

    [[nodiscard]]
    constexpr bool get(std::size_t index) const
    {
        return m_state & (1 << index);
    }

    [[nodiscard]]
    constexpr bool get(std::size_t row, std::size_t col) const
    {
        return m_state & (1 << to_index(row, col));
    }

    constexpr void set(std::size_t index)
    {
        m_state |= 1 << index;
    }

    constexpr void set(std::size_t index, bool value)
    {
        m_state |= value << index;
    }

    constexpr void set(std::size_t row, std::size_t col, bool value)
    {
        m_state |= value << to_index(row, col);
    }

    constexpr void set(std::size_t row, std::size_t col)
    {
        m_state |= 1 << to_index(row, col);
    }

    [[nodiscard]]
    constexpr std::size_t neighbour_cnt(std::size_t index) const
    {
        return std::popcount(neighbour_mask_lookup[index]);
    }

    [[nodiscard]]
    constexpr bool operator < (const Frame<Ts>& rhs) const 
    {
        return m_state < rhs.get();
    }

    [[nodiscard]]
    constexpr bool operator > (const Frame<Ts>& rhs) const 
    {
        return m_state > rhs.get();
    }

    [[nodiscard]]
    constexpr bool operator == (const Frame<Ts>& rhs) const 
    {
        return m_state == rhs.get();
    }


    /// @brief Calculates equivalent frame with minimal numerical value. Saves the information about the minizing transformation.
    /// @param row_offset Row offset
    /// @param col_offset Column offset
    /// @param trasnform_index Index of the D4 dyhedral group 
    /// transformation when traversing from original by alternating 
    /// horizontal/vertical turns and clockwise rotations.
    /// @return 
    [[nodiscard]] constexpr Frame<Ts> normalized(
        std::size_t& min_row_offset,
        std::size_t& min_col_offset,
        std::size_t& trasnform_index) const 
    {
        // I'm not sure how to meaningfully refactor this code.
        // The point is to traverse each node of the Cayley graph
        // of a dyhedral group D4 to find the equivalent grid with 
        // minimal numerical value.
        // This can be done by alternating flips (horizontal and vertical) and turns.

        min_row_offset = 0;
        min_col_offset = 0;
        trasnform_index = 0;

        Frame<Ts> min_state(m_state);
        for(std::size_t row_offset = 0; row_offset < Ts; ++row_offset)
        {
            for(std::size_t col_offset = 0; col_offset < Ts; ++col_offset)
            {
                // Identity
                Frame<Ts> s = this->translated(row_offset, col_offset);
                if(s < min_state)
                {
                    min_state = s;
                    min_row_offset = row_offset;
                    min_col_offset = col_offset;
                    trasnform_index = 0;
                }

                // Flip
                s = s.flipped<true>();
                if(s < min_state)
                {
                    min_state = s;
                    min_row_offset = row_offset;
                    min_col_offset = col_offset;
                    trasnform_index = 1;
                }

                // Turn 90 + Flipped
                s = s.turned<true>();
                if(s < min_state)
                {
                    min_state = s;
                    min_row_offset = row_offset;
                    min_col_offset = col_offset;
                    trasnform_index = 2;
                }

                // Turn 90
                s = s.flipped<false>();
                if(s < min_state)
                {
                    min_state = s;
                    min_row_offset = row_offset;
                    min_col_offset = col_offset;
                    trasnform_index = 3;
                }

                // Turn 180
                s = s.turned<true>();
                if(s < min_state)
                {
                    min_state = s;
                    min_row_offset = row_offset;
                    min_col_offset = col_offset;
                    trasnform_index = 4;
                }

                // Turn 180 + Flip
                s = s.flipped<true>();
                if(s < min_state)
                {
                    min_state = s;
                    min_row_offset = row_offset;
                    min_col_offset = col_offset;
                    trasnform_index = 5;
                }

                // Turn 270 + Flip
                s = s.turned<true>();
                if(s < min_state)
                {
                    min_state = s;
                    min_row_offset = row_offset;
                    min_col_offset = col_offset;
                    trasnform_index = 6;
                }

                // Turn 270
                s = s.flipped<false>();
                if(s < min_state)
                {
                    min_state = s;
                    min_row_offset = row_offset;
                    min_col_offset = col_offset;
                    trasnform_index = 7;
                }
            }    
        }
        return min_state;
    }

    [[nodiscard]]
    constexpr Frame<Ts> translated(std::size_t row_offset, std::size_t col_offset) const
    {
        uint64_t result = 0;

        for(std::size_t row = 0; row < Ts; ++row)
        {
            for(std::size_t col = 0; col < Ts; ++col)
            {
                std::size_t new_row = (row + row_offset) % Ts;
                std::size_t new_col = (col + col_offset) % Ts;
                result |= get(row, col) << to_index(new_row, new_col);
            }
        }

        return result;
    }

    /// @brief Flip frame around the horizontal or vertical axis.
    /// @tparam Th indicator whether to flip around the horizontal (true) or the vertical (false) axis.
    /// @return Flipped frame.
    template<bool Th>
    [[nodiscard]] constexpr Frame<Ts> flipped() const
    {
        uint64_t result = 0;
        for(std::size_t row = 0; row < Ts; ++row)
        {
            for(std::size_t col = 0; col < Ts; ++col)
            {
                bool alive;

                if constexpr (Th)
                {
                    alive = get(Ts - row - 1, col);
                }
                else
                {
                    alive = get(row, Ts - col - 1);
                }

                result |= alive << to_index(row, col);
            }
        }
        return result;
    }

    /// @brief Turn frame clockwise or counterclockwise
    /// @tparam Tcw indicator whether to turn clockwise (true) or counterclockwise (false)
    /// @return Rotated frame
    template<bool Tcw>
    [[nodiscard]] constexpr Frame<Ts> turned() const
    {
        uint64_t result = 0;
        for(std::size_t row = 0; row < Ts; ++row)
        {
            for(std::size_t col = 0; col < Ts; ++col)
            {

                bool alive;

                if constexpr (Tcw)
                {
                    alive = get(Ts - col - 1, row);
                }
                else
                {
                    alive = get(col, Ts - row - 1);
                }

                result |= alive << to_index(row, col);
            }
        }
        return result;
    }

    [[nodiscard]] constexpr Frame<Ts> transformed(
        std::size_t row_offset, 
        std::size_t col_offset, 
        std::size_t transform_index) const
    {
        Frame<Ts> result = translated(row_offset, col_offset);

        for(size_t i = 1; i < transform_index; ++i)
        {
            if (0 == (i - 1) % 4)
                result = flipped<false>();

            else if (0 == (i + 1) % 4)
                result = flipped<true>();

            else if(0 == i % 2)
                result = turned<true>();
        }

        return result;
    }

private:

    [[nodiscard]]
    constexpr static uint64_t get_neighbour_mask(std::size_t cell_row, std::size_t cell_col)
    {
        uint64_t mask = 0;
        for(std::size_t i = -1; i <= 1; ++i)
        {
            std::size_t row = (Ts + cell_row + i) % Ts;

            for(std::size_t j = -1; j <= 1; ++j)
            {
                if(i == 0 && j == 0)
                    continue;

                std::size_t col = (Ts + cell_col + j) % Ts;

                mask |= 1 << to_index(row, col);
            }
        }
        return mask;
    }

    constexpr static std::array<uint64_t, CellCount> neighbour_mask_lookup = { []() constexpr 
    {
        std::array<uint64_t, CellCount> table {};
        for (std::size_t row = 0; row < Ts; ++row)
        {
            for (std::size_t col = 0; col < Ts; ++col)
            {
                auto index = to_index(row, col);
                table[index] = get_neighbour_mask(row, col);
            }
        }
        return table;
    }() };

};

template<std::size_t Ts>
std::ostream& operator<<(std::ostream& os, const Frame<Ts>& frame)
{
    for(int i = 0; i < frame.CellCount; ++i)
    {
        os << (frame.get(i) ? '#' : '.') << ' ';

        if((1 + i) % Ts == 0)
            os << '\n';
    }
    return os;
}