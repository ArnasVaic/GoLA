#include "absl/hash/hash.h"

template <size_t N>
requires(N <= 11)
constexpr Frame<N>::Frame()
: m_state(0)
{ }

template <size_t N>
requires(N <= 11)
constexpr Frame<N>::Frame(absl::uint128 state)
: m_state(state)
{ }

template <size_t N>
requires(N <= 11)
constexpr absl::uint128 Frame<N>::get() const
{
    return m_state;
}

template <size_t N>
requires(N <= 11)
constexpr bool Frame<N>::get(size_t index) const
{
    const absl::uint128 mask = 1;
    auto masked = m_state & (mask << index);
    return masked > 0;
}

template <size_t N>
requires(N <= 11)
[[nodiscard]] constexpr bool Frame<N>::get(size_t row, size_t col) const
{
    const absl::uint128 mask = 1;
    const auto masked = m_state & (mask << index_lookup[row][col]);
    return masked > 0;
}

template <size_t N>
requires(N <= 11)
constexpr void Frame<N>::set(size_t index) {
    const absl::uint128 one = 1;
    m_state |= one << index;
}

template <size_t N>
requires(N <= 11)
constexpr void Frame<N>::set(size_t index, bool value) {
    const absl::uint128 bit = value;
    m_state |= bit << index;
}

template <size_t N>
requires(N <= 11)
constexpr void Frame<N>::set(size_t row, size_t col, bool value) {
    const absl::uint128 bit = value;
    m_state |= bit << index_lookup[row][col];
}

template <size_t N>
requires(N <= 11)
constexpr void Frame<N>::set(size_t row, size_t col) {
    const absl::uint128 one = 1;
    m_state |= one << index_lookup[row][col];
}

template <size_t N>
requires(N <= 11)
constexpr size_t Frame<N>::to_index(size_t row, size_t col)
{
    return col + row * N;
}

template <size_t N>
requires(N <= 11)
constexpr void Frame<N>::toggle(size_t index) {
    const absl::uint128 one = 1;
    m_state ^= one << index;
}

template <size_t N>
requires(N <= 11)
[[nodiscard]] constexpr size_t Frame<N>::neighbour_cnt(size_t index) const {
    const absl::uint128 near = m_state & neighbour_mask_lookup[index];
    const auto high = absl::Uint128High64(near);
    const auto low = absl::Uint128Low64(near);
    return std::popcount(high) + std::popcount(low);
}

template<size_t Ts>
requires(Ts <= 11)bool Frame<Ts>::operator==(const Frame<Ts> &other) const {
    return get() == other.get();
}

template<size_t Ts>
requires(Ts <= 11)constexpr auto Frame<Ts>::operator<(const Frame<Ts> &other) const {
    return get() < other.get();
}

template<size_t Ts>
requires(Ts <= 11)constexpr auto Frame<Ts>::operator>(const Frame<Ts> &other) const {
    return get() > other.get();
}

template<size_t Ts>
requires(Ts <= 11)size_t Frame<Ts>::Hash::operator()(const Frame<Ts> &frame) const {

    const absl::Hash<absl::uint128> hasher;
    return hasher(frame.get());
}

template<size_t Ts>
requires(Ts <= 11)constexpr Frame<Ts> Frame<Ts>::normalized(Transform &min_transform) const {
    min_transform = Transform();
    Frame<Ts> min_state(m_state);
    for (size_t row_offset = 0; row_offset < Ts; ++row_offset) {
        for (size_t col_offset = 0; col_offset < Ts; ++col_offset) {

            const Frame<Ts> translated = this->translated(row_offset, col_offset);

            Frame<Ts> transformed = translated;
            if (transformed < min_state) {
                min_state = translated;
                min_transform = {row_offset, col_offset, 0};
            }

            transformed = translated.transformed<1>();

            if (transformed < min_state) {
                min_state = transformed;
                min_transform = {row_offset, col_offset, 1};
            }

            transformed = translated.transformed<2>();

            if (transformed < min_state) {
                min_state = transformed;
                min_transform = {row_offset, col_offset, 2};
            }

            transformed = translated.transformed<3>();

            if (transformed < min_state) {
                min_state = transformed;
                min_transform = {row_offset, col_offset, 3};
            }

            transformed = translated.transformed<4>();

            if (transformed < min_state) {
                min_state = transformed;
                min_transform = {row_offset, col_offset, 4};
            }

            transformed = translated.transformed<5>();

            if (transformed < min_state) {
                min_state = transformed;
                min_transform = {row_offset, col_offset, 5};
            }

            transformed = translated.transformed<6>();

            if (transformed < min_state) {
                min_state = transformed;
                min_transform = {row_offset, col_offset, 6};
            }

            transformed = translated.transformed<7>();

            if (transformed < min_state) {
                min_state = transformed;
                min_transform = {row_offset, col_offset, 7};
            }
        }
    }
    return min_state;
}

template<size_t Ts>
requires(Ts <= 11)constexpr Frame<Ts> Frame<Ts>::translated(size_t row_offset, size_t col_offset) const {
    Frame<Ts> result;
    for (size_t row = 0; row < Ts; ++row) {
        const size_t new_row = (row + row_offset) % Ts;
        for (size_t col = 0; col < Ts; ++col) {
            const size_t new_col = (col + col_offset) % Ts;
            result.set(row, col, get(new_row, new_col));
        }
    }
    return result;
}

template<size_t Ts>
requires(Ts <= 11)
template<bool Horizontal, bool Vertical>
constexpr Frame<Ts> Frame<Ts>::flipped() const {
    if constexpr (!Horizontal && !Vertical)
        return get();

    Frame<Ts> result;
    bool cell_value;
    for (size_t row = 0; row < Ts; ++row) {
        for (size_t col = 0; col < Ts; ++col) {
            if (Horizontal && !Vertical)
                cell_value = get(Ts - 1 - row, col);

            else if (!Horizontal && Vertical)
                cell_value = get(row, Ts - col - 1);

            else // Both are true
                cell_value = get(Ts - 1 - row, Ts - col - 1);

            result.set(row, col, cell_value);
        }
    }
    return Frame<Ts>(result);
}

template<size_t Ts>
requires(Ts <= 11)
template<bool Anti>
constexpr Frame<Ts> Frame<Ts>::transposed() const {
    Frame<Ts> result;
    bool cell_value;
    for (size_t row = 0; row < Ts; ++row) {
        for (size_t col = 0; col < Ts; ++col) {
            if constexpr (Anti)
                cell_value = get(Ts - 1 - col, Ts - 1 - row);

            else
                cell_value = get(col, row);

            result.set(row, col, cell_value);
        }
    }
    return result;
}

template<size_t Ts>
requires(Ts <= 11)
template<size_t Tid>
requires(Tid < 8)
constexpr Frame<Ts> Frame<Ts>::transformed() const {
    if constexpr (0 == Tid) {
        return Frame<Ts>(get());
    } else if (1 == Tid) {
        return flipped<false, true>();
    } else if (2 == Tid) {
        return transposed<true>();
    } else if (3 == Tid) {
        return rotated<true>();
    } else if (4 == Tid) {
        return flipped<true, true>();
    } else if (5 == Tid) {
        return flipped<true, false>();
    } else if (6 == Tid) {
        return transposed<false>();
    } else if (7 == Tid) {
        return rotated<false>();
    }
}

template<size_t Ts>
requires(Ts <= 11)
template<bool Tcw>
constexpr Frame<Ts> Frame<Ts>::rotated() const {
    Frame<Ts> result;
    bool cell_value;
    for (size_t row = 0; row < Ts; ++row) {
        for (size_t col = 0; col < Ts; ++col) {
            if constexpr (Tcw)
                cell_value = get(Ts - 1 - col , row);

            else
                cell_value = get(col, Ts - 1 - row );

            result.set(row, col, cell_value);
        }
    }
    return result;
}

template<size_t Ts>
requires(Ts <= 11)constexpr Frame<Ts> Frame<Ts>::transformed(size_t transform_index) const {
    if (0 == transform_index) {
        return Frame<Ts>(get());
    } else if (1 == transform_index) {
        return flipped<false, true>();
    } else if (2 == transform_index) {
        return transposed<true>();
    } else if (3 == transform_index) {
        return rotated<true>();
    } else if (4 == transform_index) {
        return flipped<true, true>();
    } else if (5 == transform_index) {
        return flipped<true, false>();
    } else if (6 == transform_index) {
        return transposed<false>();
    } else if (7 == transform_index) {
        return rotated<false>();
    }
}

template<size_t N>
requires(N <= 11) constexpr absl::uint128 Frame<N>::get_neighbour_mask(size_t cell_row, size_t cell_col) {
    absl::uint128 mask = 0;
    for (int i = -1; i < 2; ++i) {
        const size_t row = (N + cell_row + i) % N;

        for (int j = -1; j < 2; ++j) {

            if (i == 0 && j == 0)
                continue;

            const size_t col = (N + cell_col + j) % N;
            absl::uint128 bitmask = 1;
            bitmask = bitmask << index_lookup[row][col];
            mask = mask | bitmask;
        }
    }
    return mask;
}

template<size_t N>
requires(N <= 11)
constexpr std::array<absl::uint128, Frame<N>::CellCount> Frame<N>::create_neighbour_mask_lut() {
    std::array<absl::uint128, CellCount> table{};
    for (size_t row = 0; row < N; ++row) {
        for (size_t col = 0; col < N; ++col) {
            auto index = to_index(row, col);
            table[index] = get_neighbour_mask(row, col);
        }
    }
    return table;
}

template<size_t Ts>
requires(Ts <= 11)constexpr std::array<std::array<size_t, Ts>, Ts> Frame<Ts>::create_index_lut() {
    std::array<std::array<size_t, Ts>, Ts> table{};

    for (size_t row = 0; row < Ts; ++row) {
        for (size_t col = 0; col < Ts; ++col) {
            table[row][col] = to_index(row, col);
        }
    }

    return table;
}

template<size_t N>
std::ostream &operator<<(std::ostream &os, const Frame<N> &frame) {
    for(size_t i = 0; i < frame.CellCount; ++i)
    {
        os << (frame.get(i) ? '#' : '.') << ' ';

        if((1 + i) % N == 0)
            os << '\n';
    }
    return os;
}