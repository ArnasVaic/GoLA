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
    std::unordered_map<std::uint64_t, std::size_t> m_visited;

public:

    constexpr static std::size_t Width = TWidth;

    constexpr static std::size_t Height = THeight;

    constexpr static std::size_t Total = TWidth * THeight;


    constexpr Board();

    constexpr Board(uint64_t state);


    constexpr void set(uint64_t state);

    constexpr uint64_t get() const;

    constexpr std::size_t get_frame() const;

    constexpr void next_gen();

    std::vector<uint64_t> find_cycle();
    std::unordered_set<std::vector<uint64_t>> find_unique_cycles();

    constexpr static bool is_equivalent(uint64_t a, uint64_t b);

    template<bool horz, bool vert>
    constexpr static uint64_t flip(uint64_t state);

    constexpr static bool is_translated(uint64_t a, uint64_t b);

    constexpr static uint64_t translate(std::size_t row_offset, std::size_t col_offset, uint64_t state);

    constexpr bool are_cycles_equivalent(
        const std::vector<uint64_t> &lhs, 
        const std::vector<uint64_t> &rhs);

private:

    constexpr static std::array<uint64_t, Total> neighbour_mask_lookup = { []() constexpr 
    {
        std::array<uint64_t, Total> table {};
        for (std::size_t row = 0; row < Height; ++row)
        {
            std::array<std::size_t, 3> y = { (Height + row - 1) % Height, row, (row + 1) % Height };

            for (std::size_t col = 0; col < Width; ++col)
            {
                std::array<std::size_t, 3> x = { (Width + col - 1) % Width, col, (col + 1) % Width };

                std::size_t index = to_index<Width>(row, col);

                for(std::size_t i = 0; i < 3; ++i)
                {
                    for(std::size_t j = 0; j < 3; ++j)
                    {
                        if(i == 1 && j == 1)
                            continue;

                        set_bit<Width>(y[i], x[j], table[index]);
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
constexpr Board<Width, Height>::Board(uint64_t state)
{
    set(state);
}

template<std::size_t Width, std::size_t Height>
constexpr void Board<Width, Height>::set(uint64_t state)
{
    m_state = state;
    m_frame = 0;
    m_visited.clear();
}

template<std::size_t Width, std::size_t Height>
constexpr uint64_t Board<Width, Height>::get() const
{
    return m_state;
}

template<std::size_t Width, std::size_t Height>
constexpr std::size_t Board<Width, Height>::get_frame() const
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
std::vector<uint64_t> Board<Width, Height>::find_cycle()
{
    for(;;)
    {
        next_gen();
        
        if(m_visited.contains(m_state))
        {
            size_t start = m_visited[m_state];
            std::vector<uint64_t> cycle;

            for (const auto& [state, frame] : m_visited) {
                if(frame >= start)
                {
                    cycle.push_back(state);
                }
            }

            return cycle;
        }
    
        m_visited[m_state] = m_frame++;
    }
}

template<std::size_t TWidth, std::size_t THeight>
std::unordered_set<std::vector<uint64_t>> Board<TWidth, THeight>::find_unique_cycles()
{
    std::unordered_set<std::vector<uint64_t>> cycles;
    uint64_t s = 0;
    for(uint64_t i = 0; i < (1 << Total); ++i)
    {
        set(i);
        std::vector<uint64_t> cycle = find_cycle();
        if(cycle[0] != 0 && !cycles.contains(cycle))
            cycles.insert(cycle);
    }

    // Remove equivalent cycles
    for (auto it = cycles.begin(); it != cycles.end();) {
        std::vector<uint64_t> current = *it;
        bool predicateSatisfied = false;

        // Check if there exists another element b such that P(a, b) is satisfied
        for (std::vector<uint64_t> other : cycles) {
            if (Board<w,h>::is_equivalent(current, other) && other != current) {
                predicateSatisfied = true;
                break;
            }
        }

        // If the predicate is satisfied, erase the current element
        if (predicateSatisfied) {
            it = cycles.erase(it);
        } else {
            ++it;
        }
    }

    return cycles;
}

bool are_cycles_equivalent(const std::vector<uint64_t> &lhs, const std::vector<uint64_t> &rhs)
{
    for (const auto &elem : rhs)
    {
        if(lhs[0])
    }
}

template<std::size_t Width, std::size_t Height>
std::ostream& operator<<(std::ostream& os, const Board<Width, Height>& board)
{
    //os << "frame: " << board.get_frame() << '\n';

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

template<std::size_t Width>
constexpr std::size_t to_index(std::size_t row, std::size_t col)
{
    return col + row * Width;
}

template<std::size_t Width>
constexpr void set_bit(
    std::size_t row, 
    std::size_t col, 
    uint64_t &input)
{
    input |= (1 << to_index<Width>(row, col));
}

template<std::size_t Width, std::size_t Height>
constexpr bool Board<Width, Height>::is_equivalent(uint64_t a, uint64_t b)
{
    uint64_t h = flip<true, false>(b);
    uint64_t v = flip<false, true>(b);
    uint64_t hv = flip<true, true>(b);
    return 
        is_translated(a, b) ||
        is_translated(a, h) || 
        is_translated(a, v) ||
        is_translated(a, hv);
}

template<std::size_t Width, std::size_t Height>
template<bool horz, bool vert>
constexpr uint64_t Board<Width, Height>::flip(uint64_t state)
{
    uint64_t result = 0;
    for(std::size_t row = 0; row < Height; ++row)
    {
        for(std::size_t col = 0; col < Width; ++col)
        {
            std::size_t old_index = to_index<Width>(row, col);
            bool alive = state & (1 << old_index);

            size_t new_row = row;
            if (horz)
                new_row = Height - row - 1;

            size_t new_col = col;
            if (vert)
                new_col = Width - col - 1;

            std::size_t new_index = to_index<Width>(new_row, new_col);
            result |= (alive << new_index);
        }
    }
    return result;
}

template<std::size_t Width, std::size_t Height>
constexpr bool Board<Width, Height>::is_translated(uint64_t a, uint64_t b)
{
    for(size_t row = 0; row < Height; ++row)
    {
        for(size_t col = 0; col < Width; ++col)
        {
            if(translate(row, col, b) == a)
                return true;
        }
    }
    return false;
}

template<std::size_t Width, std::size_t Height>
constexpr uint64_t Board<Width, Height>::translate(
    std::size_t row_offset, 
    std::size_t col_offset, 
    uint64_t state)
{
    uint64_t result = 0;

    for(std::size_t row = 0; row < Height; ++row)
    {
        for(std::size_t col = 0; col < Width; ++col)
        {
            std::size_t old_index = to_index<Width>(row, col);
            bool alive = state & (1 << old_index);
            std::size_t new_index = to_index<Width>(
                (row + row_offset) % Height,
                (col + col_offset) % Width);
            result |= (alive << new_index);
        }
    }

    return result;
}