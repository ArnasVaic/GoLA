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

public:

    constexpr GameOfLife()
    : m_frame(0)
    , m_generation(0)
    {

    }

    constexpr GameOfLife(Frame<Ts> frame)
    : m_frame(frame)
    , m_generation(0)
    {

    }

    [[nodiscard]] constexpr Frame<Ts> next() const
    {
        Frame<Ts> next;
        for(size_t i = 0; i < Frame<Ts>::CellCount; ++i)
        {
            size_t n = m_frame.neighbour_cnt(i);
            bool alive = m_frame.get(i);

            // std:: cout << n;

            // if((i + 1) % Ts == 0)
            //     std::cout << '\n';

            next.set(i, alive_lookup[alive][n]);
        }

        //std::cout << '\n';

        return next;
    }

    constexpr void evolve()
    {
        m_frame = next();
        ++m_generation;
    }

    [[nodiscard]] constexpr Frame<Ts> frame() const
    {
        return m_frame;
    }

    [[nodiscard]] constexpr std::size_t generation() const
    {
        return m_generation;
    }

    constexpr void set(const Frame<Ts> &frame)
    {
        m_frame = frame;
        m_generation = 0;
    }
};