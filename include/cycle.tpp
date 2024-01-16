#pragma once

template<size_t N>
constexpr size_t Cycle<N>::Hash::operator()(const Cycle<N> &cycle) const {

    std::hash<uint64_t> hash;
    size_t seed = 0;

    for (const auto& i : cycle.frames())
    {
        seed ^= hash(i.state()) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

    return seed;
}

template<size_t N>
constexpr bool Cycle<N>::Equal::operator()(const Cycle<N> &lhs, const Cycle<N> &rhs) const {
    return lhs.frames() == rhs.frames();
}

template<size_t N>
constexpr std::set<Frame<N>> Cycle<N>::normalize(const std::vector<Frame<N>> &frames) {
    if(frames.empty())
        return {};

    Transform min_transform, temp_transform;

    Frame<N> min_frame(frames[0]), normalized(0);

    for (const auto& frame : frames)
    {
        normalized = frame.normalized(temp_transform);
        if(normalized < min_frame)
        {
            min_frame = normalized;
            min_transform = temp_transform;
        }
    }

    std::set<Frame<N>> frame_set;
    for (const auto& frame : frames)
    {
        Frame<N> transformed = frame
                .translated(min_transform.row_offset, min_transform.col_offset)
                .transformed(min_transform.index);

        frame_set.insert(transformed);
    }

    return frame_set;
}

template<size_t N>
constexpr const std::set<Frame<N>> &Cycle<N>::frames() const {
    return m_frames;
}

template<size_t N>
constexpr Cycle<N>::Cycle(const std::vector<Frame<N>> &frames) {
    m_frames = normalize(frames);
}

template<size_t Ts>
std::ostream &operator<<(std::ostream &os, const Cycle<Ts> &cycle) {
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
