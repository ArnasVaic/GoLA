#pragma once

#include <set>
#include <cstdint>

#include <frame.hpp>

/// @brief 
/// @tparam Ts size of the board 
template <std::size_t Ts>
class Cycle
{

private:

    std::set<Frame<Ts>> m_frames;

public:

    constexpr Cycle(std::vector<Frame<Ts>> frames)
    {
        m_frames = normalize(frames);
    }

    constexpr static std::set<Frame<Ts>> normalize(std::vector<Frame<Ts>> frames)
    {
        if(frames.empty())
            return {};

        Transform min_transform, temp_transform;
        Frame<Ts> min_frame(std::numeric_limits<uint64_t>::max()), normalized(0);
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
            frame_set.insert(frame.transform(min_transform));

        return frame_set;
    }
};