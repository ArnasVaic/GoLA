#pragma once

#include <bit>
#include <array>
#include <limits>
#include <vector>
#include <iostream>
#include <ostream>
#include <cstdint>

#include <transform.hpp>

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

    constexpr Frame()
    : m_state(0)
    {

    }

    constexpr Frame(uint64_t state) 
    : m_state(state)
    {

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

    constexpr void toggle(std::size_t index)
    {
        m_state ^= 1 << index;
    }

    [[nodiscard]]
    constexpr std::size_t neighbour_cnt(std::size_t index) const
    {
        return std::popcount(m_state & neighbour_mask_lookup[index]);
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

    struct Hash
    {
        size_t operator()(const Frame<Ts>& frame) const 
        {
            return std::hash<uint64_t>()(frame.get());
        }
    };

    /// @brief Calculates equivalent frame with minimal numerical value.
    /// @param transform Transform that normalizes this frame.
    /// @return
    [[nodiscard]] constexpr Frame<Ts> normalized(Transform &transform) const 
    {
        // I'm not sure how to meaningfully refactor this code.
        // The point is to traverse each node of the Cayley graph
        // of a dyhedral group D4 to find the equivalent grid with 
        // minimal numerical value.
        // This can be done by alternating flips (horizontal and vertical) and turns.

        transform = Transform();
        Frame<Ts> min_state(m_state);
        
        Transform t;

        for(size_t row_offset = 0; row_offset < Ts; ++row_offset)
        {
            t.row_offset = row_offset;

            for(size_t col_offset = 0; col_offset < Ts; ++col_offset)
            {
                t.col_offset = col_offset;

                for(size_t i = 0; i < 8; ++i)
                {
                    t.index = i;
                    Frame<Ts> tr = transformed(t);

                    //std::cout << "(s,r,c,i)=(" << tr.get() << ", " << t.row_offset << ", " << t.col_offset << ", " << t.index << ")\n" << tr << '\n'; 

                    if(tr < min_state)
                    {
                        min_state = tr;
                        transform = t;
                    }
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
        const Transform &transform) const
    {
        Frame<Ts> result = translated(
            transform.row_offset, 
            transform.col_offset);

        //std::cout << "I\n" << result << '\n';

        // TODO: refactor
        for(size_t i = 1; i <= transform.index; ++i)
        {
            if (0 == (i - 1) % 4)
            {
                result = result.flipped<false>();
                //std::cout << "V\n" << result << '\n';
            }   

            else if (0 == (i + 1) % 4)
            {
                result = result.flipped<true>();
                //std::cout << "H\n" << result << '\n';
            }   

            else if(0 == i % 2)
            {
                result = result.turned<true>();
                //std::cout << "R\n" << result << '\n';
            }
        }
        
        return result;
    }

private:

    [[nodiscard]]
    constexpr static uint64_t get_neighbour_mask(std::size_t cell_row, std::size_t cell_col)
    {
        uint64_t mask = 0;
        for(int i = -1; i < 2; ++i)
        {
            std::size_t row = (Ts + cell_row + i) % Ts;

            for(int j = -1; j < 2; ++j)
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