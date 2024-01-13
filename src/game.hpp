#pragma once

#include <unordered_map>
#include <cycle.hpp>
#include <frame.hpp>

template <size_t N>
using cycle_uset_t =  std::unordered_set<Cycle<N>, typename Cycle<N>::Hash, typename Cycle<N>::Equal>;

template <size_t N>
using frame_gen_map_t = std::unordered_map<Frame<N>, size_t, typename Frame<N>::Hash>;

template<size_t N>
class Game
{

private:

    Frame<N> m_current;
    Frame<N> m_frame;
    size_t m_generation;

public:

    constexpr Game();

    [[nodiscard]] constexpr Frame<N> next() const;
    constexpr void evolve();
    constexpr void reset(const Frame<N> &frame);

    [[nodiscard]]
    constexpr Cycle<N> find_cycle(
        frame_gen_map_t<N> &visited_frames,
        std::vector<Frame<N>> &cycle_frames);

    cycle_uset_t<N> find_cycles(size_t samples, size_t sample_length);

    [[nodiscard]]
    cycle_uset_t<N> search_perturbed(cycle_uset_t<N> cycles);

    [[nodiscard]]
    cycle_uset_t<N> search_perturbed(Cycle<N> const & cycle);

    [[nodiscard]]
    cycle_uset_t<N> search_orbit(Cycle<N> const& start);

    void search_orbit_recursive(
        Cycle<N> const& parent_cycle,
        cycle_uset_t<N> &visited,
        frame_gen_map_t<N> &visited_frames,
        std::vector<Frame<N>> &cycle_frames,
        Frame<N> &frame);

private:
    constexpr static bool alive_lut[2][9] =
    {
        { false, false, false, true, false, false, false, false, false },
        { false, false, true , true, false, false, false, false, false }
    };
};

#include <game.tpp>