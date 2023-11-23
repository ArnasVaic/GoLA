#pragma once

#include <bit>
#include <array>
#include <vector>
#include <cstdint>
#include <iostream>
#include <unordered_map>
#include <unordered_set>

class VectorU64Hash {
public:
    size_t operator()(const std::vector<uint64_t>& vector) const {
        std::hash<uint64_t> hasher;
        size_t seed = 0;
        for (const auto& i : vector) {
            seed ^= hasher(i) + 0x9e3779b9 + (seed<<6) + (seed>>2);
        }
        return seed;
    }
};

template<std::size_t TSize>
class Board
{
    static_assert(TSize <= 8, "Side length must be less or equal to 8");

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

    constexpr static std::size_t Size = TSize;

    constexpr static std::size_t Total = TSize * TSize;


    constexpr Board();

    constexpr Board(uint64_t state);


    constexpr void set(uint64_t state);

    constexpr uint64_t get() const;

    constexpr std::size_t get_frame() const;

    constexpr void next_gen();

    std::vector<uint64_t> find_cycle();
    std::unordered_set<std::vector<uint64_t>, VectorU64Hash> find_unique_non_zero_cycles();

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
        for (std::size_t row = 0; row < TSize; ++row)
        {
            std::array<std::size_t, 3> y = { (TSize + row - 1) % TSize, row, (row + 1) % TSize };

            for (std::size_t col = 0; col < TSize; ++col)
            {
                std::array<std::size_t, 3> x = { (TSize + col - 1) % TSize, col, (col + 1) % TSize };

                std::size_t index = to_index<TSize>(row, col);

                for(std::size_t i = 0; i < 3; ++i)
                {
                    for(std::size_t j = 0; j < 3; ++j)
                    {
                        if(i == 1 && j == 1)
                            continue;

                        set_bit<TSize>(y[i], x[j], table[index]);
                    }
                }
            }
        }
        return table;
    }() };
    
};

template<std::size_t TSize>
constexpr Board<TSize>::Board() 
    : m_state(0), m_frame(0), m_visited()
{

}

template<std::size_t TSize>
constexpr Board<TSize>::Board(uint64_t state)
{
    set(state);
}

template<std::size_t TSize>
constexpr void Board<TSize>::set(uint64_t state)
{
    m_state = state;
    m_frame = 0;
    m_visited.clear();
}

template<std::size_t TSize>
constexpr uint64_t Board<TSize>::get() const
{
    return m_state;
}

template<std::size_t TSize>
constexpr std::size_t Board<TSize>::get_frame() const
{
    return m_frame;
}

template<std::size_t TSize>
constexpr void Board<TSize>::next_gen()
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

template<std::size_t TSize>
std::vector<uint64_t> Board<TSize>::find_cycle()
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

template<std::size_t TSize>
std::unordered_set<std::vector<uint64_t>, VectorU64Hash> Board<TSize>::find_unique_non_zero_cycles()
{
    std::unordered_set<std::vector<uint64_t>, VectorU64Hash> cycles;
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
            if (are_cycles_equivalent(current, other) && other != current) {
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

template<std::size_t TSize>
constexpr bool Board<TSize>::are_cycles_equivalent(
    const std::vector<uint64_t> &lhs, 
    const std::vector<uint64_t> &rhs)
{
    for (const auto &elem : rhs)
    {
        if(is_equivalent(lhs[0], elem))
            return true;
    }
    return false;
}

template<std::size_t TSize>
std::ostream& operator<<(std::ostream& os, const Board<TSize>& board)
{
    uint64_t state = board.get();

    for(int i = 0; i < board.Total; ++i)
    {
        char c = (state & (1 << i)) == 0 ? '.' : '#';
        os << c << ' ';

        if((1 + i) % TSize == 0)
            os << '\n';
    }

    return os;
}

template<std::size_t TSize>
std::ostream& operator<<(std::ostream& os, const std::vector<Board<TSize>>& cycle)
{
    os << "Cycle:\n";
    for(const auto& frame : cycle) {
        os << frame << '\n';
    }
    return os;
}

template<std::size_t TSize>
constexpr std::size_t to_index(std::size_t row, std::size_t col)
{
    return col + row * TSize;
}

template<std::size_t TSize>
constexpr void set_bit(
    std::size_t row, 
    std::size_t col, 
    uint64_t &input)
{
    input |= (1 << to_index<TSize>(row, col));
}

template<std::size_t TSize>
constexpr bool Board<TSize>::is_equivalent(uint64_t a, uint64_t b)
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

template<std::size_t TSize>
template<bool horz, bool vert>
constexpr uint64_t Board<TSize>::flip(uint64_t state)
{
    uint64_t result = 0;
    for(std::size_t row = 0; row < TSize; ++row)
    {
        for(std::size_t col = 0; col < TSize; ++col)
        {
            std::size_t old_index = to_index<TSize>(row, col);
            bool alive = state & (1 << old_index);

            size_t new_row = row;
            if (horz)
                new_row = TSize - row - 1;

            size_t new_col = col;
            if (vert)
                new_col = TSize - col - 1;

            std::size_t new_index = to_index<TSize>(new_row, new_col);
            result |= (alive << new_index);
        }
    }
    return result;
}

template<std::size_t TSize>
constexpr bool Board<TSize>::is_translated(uint64_t a, uint64_t b)
{
    for(size_t row = 0; row < TSize; ++row)
    {
        for(size_t col = 0; col < TSize; ++col)
        {
            if(translate(row, col, b) == a)
                return true;
        }
    }
    return false;
}

template<std::size_t TSize>
constexpr uint64_t Board<TSize>::translate(
    std::size_t row_offset, 
    std::size_t col_offset, 
    uint64_t state)
{
    uint64_t result = 0;

    for(std::size_t row = 0; row < TSize; ++row)
    {
        for(std::size_t col = 0; col < TSize; ++col)
        {
            std::size_t old_index = to_index<TSize>(row, col);
            bool alive = state & (1 << old_index);
            std::size_t new_index = to_index<TSize>(
                (row + row_offset) % TSize,
                (col + col_offset) % TSize);
            result |= (alive << new_index);
        }
    }

    return result;
}