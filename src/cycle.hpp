#pragma once

#include <set>
#include <vector>
#include <cstdint>
#include <frame.hpp>

/// @brief 
/// @tparam Ts size of the board 
template <size_t Ts>
class Cycle
{

private:

    std::set<Frame<Ts>> m_frames;

public:

    explicit constexpr Cycle(const std::vector<Frame<Ts>> &frames)
    {
        m_frames = normalize(frames);
    }

    [[nodiscard]] constexpr const std::set<Frame<Ts>>& frames() const
    {
        return m_frames;
    }

    constexpr static std::set<Frame<Ts>> normalize(const std::vector<Frame<Ts>> &frames)
    {
        if(frames.empty())
            return {};

        Transform min_transform, temp_transform;

        Frame<Ts> min_frame(frames[0]), normalized(0);

        for (const auto& frame : frames)
        {
            normalized = frame.normalized(temp_transform);
            if(normalized < min_frame)
            {
                min_frame = normalized;
                min_transform = temp_transform;
            }
        }

        std::set<Frame<Ts>> frame_set;
        for (const auto& frame : frames)
        {
            Frame<Ts> transformed = frame
                .translated(min_transform.row_offset, min_transform.col_offset)
                .transformed(min_transform.index);

            frame_set.insert(transformed);
        }
        
        return frame_set;
    }

    struct Equal
    {
        [[nodiscard]] constexpr bool operator()(const Cycle<Ts>& lhs, const Cycle<Ts>& rhs) const
        {
            return lhs.frames() == rhs.frames();
        }
    };

    struct Hash
    {
        [[nodiscard]] constexpr size_t operator()(const Cycle<Ts>& cycle) const
        {

            std::hash<uint64_t> hasher;
            size_t seed = 0;

            for (const auto& i : cycle.frames()) 
            {
                seed ^= hasher(i.get()) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            }

            return seed;
        }
    };
};

template<size_t Ts>
std::ostream& operator<<(std::ostream& os, const Cycle<Ts>& cycle)
{
    for(size_t row = 0; row < Ts; ++row)
    {
        for(const auto& frame : cycle.frames())
        {
            for(size_t col = 0; col < Ts; ++col)
            {
                char ch = frame.get(row, col) ? '#' : '.';
                os << ch << ' ';
            }
            os << "  ";
        }
        os << '\n';
    }

    return os;
}