#pragma once

template<size_t N>
constexpr Game<N>::Game()
: m_current(0)
, m_next(0)
, m_generation(0)
{

}

template<size_t N>
constexpr void Game<N>::reset(const Frame<N> &frame) {
    m_current = frame;
    m_next = 0;
    m_generation = 0;
}

template<size_t N>
constexpr void Game<N>::evolve() {
    m_next = 0;
    for(size_t i = 0; i < Frame<N>::cell_count; ++i)
    {
        const size_t n = m_current.neighbour_cnt(i);
        const bool alive = m_current.get(i);
        m_next.set(i, alive_lut[alive][n]);
    }
    m_current = m_next;
    ++m_generation;
}

template<size_t N>
constexpr Frame<N> Game<N>::frame() const {
    return m_current.state();
}

template<size_t N>
constexpr Cycle<N> Game<N>::find_cycle(
    frame_gen_map_t<N> &visited_frames,
    std::vector<Frame<N>> &cycle_frames)
{
    visited_frames.insert({ m_current, m_generation });

    for(;;)
    {
        evolve();
        if(visited_frames.contains(m_current))
        {
            size_t cycle_begin_generation = visited_frames[m_current];

            for (const auto& [frame, generation] : visited_frames) {
                if(generation >= cycle_begin_generation)
                {
                    cycle_frames.push_back(frame);
                }
            }
            visited_frames.clear();
            Cycle<N> cycle(cycle_frames);
            cycle_frames.clear();
            return cycle;
        }
        else
        {
            visited_frames[m_current] = m_generation;
        }
    }
}

template<size_t N>
cycle_uset_t<N> Game<N>::search_perturbed(cycle_uset_t<N> cycles) {
    // Reuse containers to avoid instantiation.
    // Used as a lookup to check if frame has been visited
    frame_gen_map_t<N> visited_frames;
    // Cycle frames are accumulated.
    std::vector<Frame<N>> cycle_frames;
    // Given cycles + cycles that were found by perturbing each frame from given cycles
    cycle_uset_t<N> total_cycles;

    // Copy existing cycles
    for (auto const& cycle : cycles)
        total_cycles.insert(cycle);

    for (auto const& cycle : cycles)
    {
        for(const auto& org_frame : cycle.frames())
        {
            for(size_t i = 0; i < Frame<N>::cell_count; ++i)
            {
                Frame<N> frame = org_frame;
                frame.toggle(i);
                reset(frame);
                auto perturbed_cycle = find_cycle(visited_frames, cycle_frames);
                total_cycles.insert(perturbed_cycle);
            }
        }
    }

    return total_cycles;
}

template<size_t N>
cycle_uset_t<N> Game<N>::find_cycles(size_t samples, size_t sample_length) {
    // Reuse containers to avoid instantiation.

    // Used as a lookup to check if frame has been visited
    frame_gen_map_t<N> visited_frames;

    // Cycle frames are accumulated.
    std::vector<Frame<N>> cycle_frames;

    // Resulting cycles
    cycle_uset_t<N> cycles;

    const size_t space_length = Frame<N>::States / samples - sample_length;

    // Sample evenly spaced intervals
    for(size_t sample_index = 0; sample_index < samples; ++sample_index)
    {
        const uint64_t start_state = sample_index * (space_length + sample_length);

        for(uint64_t state = start_state; state < start_state + sample_length; ++state)
        {
            reset(Frame<N>(state));
            const auto cycle = find_cycle(visited_frames, cycle_frames);
            cycles.insert(cycle);
        }
    }

    return cycles;
}

template<size_t N>
cycle_uset_t<N> Game<N>::search_perturbed(const Cycle<N> &cycle) {
    // Reuse containers to avoid instantiation.
    // Used as a lookup to check if frame has been visited
    frame_gen_map_t<N> visited_frames;
    // Cycle frames are accumulated.
    std::vector<Frame<N>> cycle_frames;
    // Cycles that were found by perturbing each frame from given cycles
    cycle_uset_t<N> cycles;

    for(const auto& org_frame : cycle.frames())
    {
        for(size_t i = 0; i < Frame<N>::cell_count; ++i)
        {
            Frame<N> frame = org_frame;
            frame.toggle(i);
            reset(frame);
            auto perturbed_cycle = find_cycle(visited_frames, cycle_frames);
            cycles.insert(perturbed_cycle);
        }
    }

    return cycles;
}

template<size_t N>
void Game<N>::search_orbit_recursive(
        Cycle<N> const& parent_cycle,
        cycle_uset_t<N> &visited,
        frame_gen_map_t<N> &visited_frames,
        std::vector<Frame<N>> &cycle_frames,
        Frame<N> &frame)
{
    for(const auto& org_frame : parent_cycle.frames())
    {
        for(size_t i = 0; i < Frame<N>::cell_count; ++i)
        {
            frame = org_frame;
            frame.toggle(i);
            reset(frame);
            auto cycle = find_cycle(visited_frames, cycle_frames);
            if(visited.contains(cycle))
                continue;
            visited.insert(cycle);
            search_orbit_recursive(cycle, visited, visited_frames, cycle_frames, frame);
        }
    }
}

template<size_t N>
cycle_uset_t<N> Game<N>::search_orbit(Cycle<N> const& start) {
    cycle_uset_t<N> cycles;
    frame_gen_map_t<N> visited_frames;
    std::vector<Frame<N>> cycle_frames;
    Frame<N> frame;

    search_orbit_recursive(start, cycles, visited_frames, cycle_frames, frame);
    return cycles;
}