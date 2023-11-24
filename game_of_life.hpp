#pragma once

#include <unordered_map>
#include <cycle.hpp>
#include <frame.hpp>

template<size_t Ts>
class GameOfLife
{

private:

    constexpr static bool alive_lookup[2][9] =
    {
        { false, false, false, true, false, false, false, false, false },
        { false, false, true , true, false, false, false, false, false }
    };

    Frame<Ts> m_frame;
    size_t m_generation;

    std::unordered_map<Frame<Ts>, size_t> m_visited;

public:

    constexpr GameOfLife(Frame<Ts> frame)
    : m_frame(frame)
    {

    }

    [[nodiscard]] constexpr Frame<Ts> current_frame() const
    {
        return m_frame;
    }

    [[nodiscard]] constexpr std::size_t generation() const
    {
        return m_generation;
    }

    constexpr void next_gen();

    std::vector<uint64_t> find_cycle();
    std::unordered_set<std::vector<uint64_t>, VectorU64Hash, VectorU64Equal<TSize>> find_unique_non_zero_cycles();

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

        // printf("%zu(%i) ", n, alive);
        // if((i + 1) % TSize == 0)
        //     std::cout << '\n';
    }
    //std::cout << '\n';
    m_state = next;

    //std::cout << *this << '\n';

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
std::unordered_set<std::vector<uint64_t>, VectorU64Hash, VectorU64Equal<TSize>> Board<TSize>::find_unique_non_zero_cycles()
{
    std::unordered_set<std::vector<uint64_t>, VectorU64Hash, VectorU64Equal<TSize>> cycles;
    uint64_t s = 0;

    for(uint64_t i = 0; i < (1 << Total); ++i)
    {
        set(i);
        std::vector<uint64_t> cycle = find_cycle();
        if(cycle[0] != 0 && !cycles.contains(cycle))
            cycles.insert(cycle);
    }

    // Remove equivalent cycles
    // for (auto it = cycles.begin(); it != cycles.end();) {
    //     std::vector<uint64_t> current = *it;
    //     bool predicateSatisfied = false;

    //     // Check if there exists another element b such that P(a, b) is satisfied
    //     for (std::vector<uint64_t> other : cycles) {
    //         if (are_cycles_equivalent(current, other) && other != current) {
    //             predicateSatisfied = true;
    //             break;
    //         }
    //     }

    //     // If the predicate is satisfied, erase the current element
    //     if (predicateSatisfied) {
    //         it = cycles.erase(it);
    //     } else {
    //         ++it;
    //     }
    // }

    return cycles;
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