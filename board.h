#pragma once

#include <bit>
#include <array>
#include <cstdint>
#include <iostream>
#include <unordered_map>
#include <unordered_set>

template<std::size_t TWidth, std::size_t THeight>
class Board
{
    static_assert(TWidth <= 8, "Width must be less or equal to 8");
    static_assert(THeight <= 8, "Height must be less or equal to 8");

private:

    constexpr static bool alive_lookup[2][8] =
    {
        { false, false, false, true, false, false, false, false },
        { false, false, true , true, false, false, false, false }
    };

    uint64_t m_state;
    std::size_t m_frame;
    std::unordered_map<std::size_t, std::size_t> m_visited;

public:

    constexpr static std::size_t Width = TWidth;

    constexpr static std::size_t Height = THeight;

    constexpr static std::size_t Total = TWidth * THeight;

    constexpr Board();

    constexpr void set(uint64_t state);

    constexpr uint64_t get();

    constexpr std::size_t get_frame();

    constexpr void next_gen();

    std::size_t find_cycle(uint64_t &period_1_state);

    void find_cycles(
        std::unordered_set<uint64_t> &stable,
        std::unordered_map<std::size_t, std::size_t> &frequencies
    );

private:

    constexpr static void set_bit(std::size_t row, std::size_t col, uint64_t &input);

    constexpr static std::size_t to_index(std::size_t row, std::size_t col);

    constexpr static std::array<uint64_t, Total> neighbour_mask_lookup = { []() constexpr 
    {
        std::array<uint64_t, Total> table {};
        for (std::size_t row = 0; row < Height; ++row)
        {
            std::array<std::size_t, 3> y = { (Height + row - 1) % Height, row, (row + 1) % Height };

            for (std::size_t col = 0; col < Width; ++col)
            {
                std::array<std::size_t, 3> x = { (Width + col - 1) % Width, col, (col + 1) % Width };

                std::size_t index = to_index(row, col);

                for(std::size_t i = 0; i < 3; ++i)
                {
                    for(std::size_t j = 0; j < 3; ++j)
                    {
                        if(i == 1 && j == 1)
                            continue;

                        set_bit(y[i], x[j], table[index]);
                    }
                }
            }
        }
        return table;
    }() };
};

template<std::size_t Width, std::size_t Height>
constexpr Board<Width, Height>::Board() 
    : m_state(0), m_frame(0), m_visited()
{

}

template<std::size_t Width, std::size_t Height>
constexpr void Board<Width, Height>::set(uint64_t state)
{
    m_state = state;
    m_frame = 0;
    m_visited.clear();
}

template<std::size_t Width, std::size_t Height>
constexpr uint64_t Board<Width, Height>::get()
{
    return m_state;
}

template<std::size_t Width, std::size_t Height>
constexpr std::size_t Board<Width, Height>::get_frame()
{
    return m_frame;
}

template<std::size_t Width, std::size_t Height>
constexpr void Board<Width, Height>::next_gen()
{
    uint64_t next = 0;
    for(size_t i = 0; i < Total; ++i)
    {
        uint64_t mask = neighbour_mask_lookup[i];
        size_t n = std::popcount(mask & m_state);
        bool alive = (m_state & (1 << i)) != 0;
        next |= alive_lookup[alive][n] << i;
    }
    m_state = next;
}

template<std::size_t Width, std::size_t Height>
std::size_t Board<Width, Height>::find_cycle(uint64_t &period_1_state)
{
    for(;;)
    {
        next_gen();
        
        if(m_visited.count(m_state) > 0)
        {
            period_1_state = m_state;
            return m_frame - m_visited[m_state];
        }
    
        m_visited[m_state] = m_frame++;
    }
}

template<std::size_t Width, std::size_t Height>
void Board<Width, Height>::find_cycles(
    std::unordered_set<uint64_t> &stable,
    std::unordered_map<std::size_t, std::size_t> &frequencies)
{
    uint64_t s = 0;
    for(uint64_t i = 0; i < (1 << Total); ++i)
    {
        set(i);
        size_t period = find_cycle(s);

        if(period == 1 && s != 0 && stable.count(s) == 0)
            stable.insert(s);

        if(frequencies.count(period) > 0)
            ++frequencies[period];
        else
            frequencies[period] = 1;
    }
}

template<std::size_t Width, std::size_t Height>
std::ostream& operator<<(std::ostream& os, const Board<Width, Height>& board)
{
    os << "frame: " << board.get_frame() << '\n';

    uint64_t state = board.get();

    for(int i = 0; i < board.Total; ++i)
    {
        char c = (state & (1 << i)) == 0 ? '.' : '#';
        os << c << ' ';

        if((1 + i) % Width == 0)
            os << '\n';
    }

    return os;
}

template<std::size_t Width, std::size_t Height>
constexpr std::size_t Board<Width, Height>::to_index(
    std::size_t row, 
    std::size_t col)
{
    return col + row * Width;
}

template<std::size_t Width, std::size_t Height>
constexpr void Board<Width, Height>::set_bit(
    std::size_t row, 
    std::size_t col, 
    uint64_t &input)
{
    input |= (1 << to_index(row, col));
}