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

    std::unordered_set<Cycle<Ts>, typename Cycle<Ts>::Hash, typename Cycle<Ts>::Equal> find_cycles(
        size_t samples,
        size_t sample_length)
    {
        // Reuse containers to avoid instantiation.

        // Used as a lookup to check if frame has been visited
        std::unordered_map<Frame<Ts>, size_t, typename Frame<Ts>::Hash> visited_frames;

        // Cycle frames are accumulated.
        std::vector<Frame<Ts>> cycle_frames;

        // Resulting cycles
        std::unordered_set<Cycle<Ts>, typename Cycle<Ts>::Hash, typename Cycle<Ts>::Equal> cycles;

        const size_t space_length = Frame<Ts>::States / samples - sample_length;

        // Sample evenly spaced intervals
        for(size_t sample_index = 0; sample_index < samples; ++sample_index)
        {
            const uint64_t start_state = space_length + sample_index * (space_length + sample_length);

            for(uint64_t state = start_state; state < start_state + sample_length; ++state)
            {
                set(Frame<Ts>(state));
                const auto cycle = find_cycle(visited_frames, cycle_frames);
                cycles.insert(cycle);
            }
        }

        return cycles;
    }

    std::unordered_set<Cycle<Ts>, typename Cycle<Ts>::Hash, typename Cycle<Ts>::Equal> search_perturbed(
        std::unordered_set<Cycle<Ts>, typename Cycle<Ts>::Hash, typename Cycle<Ts>::Equal> cycles)
    {
        // Reuse containers to avoid instantiation.
        // Used as a lookup to check if frame has been visited
        std::unordered_map<Frame<Ts>, size_t, typename Frame<Ts>::Hash> visited_frames;
        // Cycle frames are accumulated.
        std::vector<Frame<Ts>> cycle_frames;
        // Given cycles + cycles that were found by perturbing each frame from given cycles
        std::unordered_set<Cycle<Ts>, typename Cycle<Ts>::Hash, typename Cycle<Ts>::Equal> total_cycles;

        // Copy existing cycles
        for (auto const& cycle : cycles)
            total_cycles.insert(cycle);

        for (auto const& cycle : cycles)
        {
            for(const auto& org_frame : cycle.frames())
            {
                for(size_t i = 0; i < Frame<Ts>::CellCount; ++i)
                {
                    Frame<Ts> frame = org_frame;
                    frame.toggle(i);
                    set(frame);
                    auto perturbed_cycle = find_cycle(visited_frames, cycle_frames);
                    total_cycles.insert(perturbed_cycle);
                }
            }
        }

        return total_cycles;
    }
};