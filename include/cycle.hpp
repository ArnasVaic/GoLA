#pragma once

#include <set>
#include <vector>
#include <cstdint>
#include <frame.hpp>

/// @brief 
/// @tparam N size of the board
template <size_t N>
class Cycle
{

private:

    std::set<Frame<N>> m_frames;

public:

    explicit constexpr Cycle(const std::vector<Frame<N>> &frames);

    [[nodiscard]]
    constexpr const std::set<Frame<N>>& frames() const;

    constexpr static std::set<Frame<N>> normalize(const std::vector<Frame<N>> &frames);

    struct Equal
    {
        [[nodiscard]]
        constexpr bool operator()(const Cycle<N>& lhs, const Cycle<N>& rhs) const;
    };

    struct Hash
    {
        [[nodiscard]]
        constexpr size_t operator()(const Cycle<N>& cycle) const;
    };
};

template<size_t Ts>
std::ostream& operator<<(std::ostream& os, const Cycle<Ts>& cycle);

#include <cycle.tpp>