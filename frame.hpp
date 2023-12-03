#pragma once

#include <bit>
#include <array>
#include <limits>
#include <vector>
#include <iostream>
#include <ostream>
#include <cstdint>
#include <transform.hpp>
#include <unordered_map>
#include <functional>

/// @brief
/// @tparam Ts width and height of the frame 
template <size_t Ts>
requires(Ts <= 8)
class Frame {

private:

    uint64_t m_state;

public:

    constexpr static size_t CellCount = Ts * Ts;

public:

    constexpr Frame()
        : m_state(0)
    { }

    explicit constexpr Frame(uint64_t state)
        : m_state(state)
    { }

    [[nodiscard]] constexpr static size_t to_index(size_t row, size_t col) {
        return col + row * Ts;
    }

    [[nodiscard]] constexpr uint64_t get() const {
        return m_state;
    }

    [[nodiscard]] constexpr bool get(size_t index) const {
        return m_state & (1ull << index);
    }

    [[nodiscard]] constexpr bool get(size_t row, size_t col) const {
        return m_state & (1ull << index_lookup[row][col]);
    }

    constexpr void set(size_t index) {
        m_state |= 1ull << index;
    }

    constexpr void set(size_t index, bool value) {
        m_state |= value << index;
    }

    constexpr void set(size_t row, size_t col, bool value) {
        m_state |= value << index_lookup[row][col];
    }

    constexpr void set(size_t row, size_t col) {
        m_state |= 1ull << index_lookup[row][col];
    }

    constexpr void toggle(size_t index) {
        m_state ^= 1ull << index;
    }

    [[nodiscard]] constexpr size_t neighbour_cnt(size_t index) const {
        return std::popcount(m_state & neighbour_mask_lookup[index]);
    }

    [[nodiscard]] constexpr auto operator == (Frame<Ts> const& other) const
    {
        return m_state == other.get();
    }

    [[nodiscard]] constexpr auto operator > (Frame<Ts> const& other) const
    {
        return m_state > other.get();
    }

    [[nodiscard]] constexpr auto operator < (Frame<Ts> const& other) const
    {
        return m_state < other.get();
    }

    [[nodiscard]] constexpr auto operator != (Frame<Ts> const& other) const
    {
        return m_state != other.get();
    }

    struct Hash {
        [[nodiscard]] size_t operator()(const Frame<Ts> &frame) const {
            return std::hash<uint64_t>()(frame.get());
        }
    };

    struct Equal
    {
        [[nodiscard]] constexpr bool operator()(const Frame<Ts>& lhs, const Frame<Ts>& rhs) const
        {
            return lhs.get() == rhs.get();
        }
    };

    /// @brief Retrieve normalized frame.
    /// @param min_transform Transform that normalizes this frame.
    /// @return Equivalent frame with minimal numerical value.
    [[nodiscard]] constexpr Frame<Ts> normalized(Transform &min_transform) const {
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

    [[nodiscard]] constexpr Frame<Ts> normalized() const {
        Frame<Ts> min_state(m_state);
        for (size_t row_offset = 0; row_offset < Ts; ++row_offset) {
            for (size_t col_offset = 0; col_offset < Ts; ++col_offset) {

                const Frame<Ts> translated = this->translated(row_offset, col_offset);

                Frame<Ts> transformed = translated;
                if (transformed < min_state)
                    min_state = translated;

                transformed = translated.transformed<1>();

                if (transformed < min_state)
                    min_state = transformed;

                transformed = translated.transformed<2>();

                if (transformed < min_state)
                    min_state = transformed;

                transformed = translated.transformed<3>();

                if (transformed < min_state)
                    min_state = transformed;

                transformed = translated.transformed<4>();

                if (transformed < min_state)
                    min_state = transformed;

                transformed = translated.transformed<5>();

                if (transformed < min_state)
                    min_state = transformed;

                transformed = translated.transformed<6>();

                if (transformed < min_state)
                    min_state = transformed;

                transformed = translated.transformed<7>();

                if (transformed < min_state)
                    min_state = transformed;
            }
        }
        return min_state;
    }

    [[nodiscard]] constexpr Frame<Ts> translated(size_t row_offset, size_t col_offset) const {
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

    /// @brief Flip frame.
    /// @tparam Horizontal indicator whether to flip around the horizontal axis.
    /// @tparam Vertical indicator whether to flip around the vertical axis.
    /// @return Flipped frame.
    template<bool Horizontal, bool Vertical>
    [[nodiscard]] constexpr Frame<Ts> flipped() const {
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

    /// @brief Transposes the frame.
    /// @tparam Anti Indicator whether to transpose about anti-diagonal (true) or the main diagonal (false).
    /// @return Transposed frame.
    template<bool Anti>
    [[nodiscard]] constexpr Frame<Ts> transposed() const {
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

    /// @brief Rotate frame
    /// @tparam Tcw Indicator whether to rotate clockwise (true) or counterclockwise (false)
    /// @return Rotated frame
    template<bool Tcw>
    [[nodiscard]] constexpr Frame<Ts> rotated() const {
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

    /// @brief Transform frame.
    /// @tparam Tid Index of the D4 group Cayley table node.
    /// Legend:
    /// I - identity transform,
    /// R - clockwise rotation of 90 degrees,
    /// V - flip about vertical axis,
    /// H - flip about horizontal axis
    ///
    /// Nodes are indexed like the following:
    /// 0 - I
    /// 1 - V
    /// 2 - VR
    /// 3 - R
    /// 4 - RR
    /// 5 - H
    /// 6 - HR
    /// 7 - RRR
    /// @return Transformed frame.
    template<size_t Tid>
    requires(Tid < 8)
    [[nodiscard]] constexpr Frame<Ts> transformed() const {
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

    /// @brief Transform frame.
    /// @param transform_index Index of the D4 group Cayley table node.
    /// Legend:
    /// I - identity transform,
    /// R - clockwise rotation of 90 degrees,
    /// V - flip about vertical axis,
    /// H - flip about horizontal axis
    ///
    /// Nodes are indexed like the following:
    /// 0 - I
    /// 1 - V
    /// 2 - VR
    /// 3 - R
    /// 4 - RR
    /// 5 - H
    /// 6 - HR
    /// 7 - RRR
    /// @return Transformed frame.
    [[nodiscard]] constexpr Frame<Ts> transformed(size_t transform_index) const {
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

private:

    [[nodiscard]]
    constexpr static uint64_t get_neighbour_mask(size_t cell_row, size_t cell_col) {
        uint64_t mask = 0;
        for (int i = -1; i < 2; ++i) {
            size_t row = (Ts + cell_row + i) % Ts;

            for (int j = -1; j < 2; ++j) {
                if (i == 0 && j == 0)
                    continue;

                size_t col = (Ts + cell_col + j) % Ts;

                mask |= 1ull << index_lookup[row][col];
            }
        }
        return mask;
    }

    constexpr static std::array<uint64_t, CellCount> neighbour_mask_lookup = {[]() constexpr {
        std::array<uint64_t, CellCount> table{};
        for (size_t row = 0; row < Ts; ++row) {
            for (size_t col = 0; col < Ts; ++col) {
                auto index = to_index(row, col);
                table[index] = get_neighbour_mask(row, col);
            }
        }
        return table;
    }()};

    constexpr static std::array<std::array<size_t, Ts>, Ts> index_lookup = {[]() constexpr {
        std::array<std::array<size_t, Ts>, Ts> table{};

        for (size_t row = 0; row < Ts; ++row) {
            for (size_t col = 0; col < Ts; ++col) {
                table[row][col] = to_index(row, col);
            }
        }

        return table;
    }()};
};

template<size_t Ts>
std::ostream& operator<<(std::ostream& os, const Frame<Ts>& frame)
{
    for(size_t i = 0; i < frame.CellCount; ++i)
    {
        os << (frame.get(i) ? '#' : '.') << ' ';

        if((1 + i) % Ts == 0)
            os << '\n';
    }
    return os;
}