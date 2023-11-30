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

    constexpr GameOfLife(const Frame<Ts> &frame)
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

    [[nodiscard]] constexpr size_t generation() const
    {
        return m_generation;
    }

    constexpr void set(const Frame<Ts> &frame)
    {
        m_frame = frame;
        m_generation = 0;
    }

    [[nodiscard]] constexpr Cycle<Ts> find_cycle(
            std::unordered_map<Frame<Ts>, size_t, typename Frame<Ts>::Hash> &visited_frames,
            std::vector<Frame<Ts>> &cycle_frames)
    {
        visited_frames.insert({ m_frame, m_generation });
    
        for(;;)
        {
            evolve();
            if(visited_frames.contains(m_frame))
            {
                size_t cycle_begin_generation = visited_frames[m_frame];

                for (const auto& [frame, generation] : visited_frames) {
                    if(generation >= cycle_begin_generation)
                    {
                        cycle_frames.push_back(frame);
                    }
                }
                visited_frames.clear();
                Cycle<Ts> cycle(cycle_frames);
                cycle_frames.clear();
                return cycle;
            }
            else
            {
                visited_frames[m_frame] = m_generation;
            }
        }
    }
};