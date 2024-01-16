#pragma once
#include <ostream>
#include <optional>
#include <algorithm>
#include <transform.hpp>

/// @brief
/// @tparam N width and height of the frame
template <size_t N>
requires(N <= 8)
class Frame {

private:

    /// @brief internal state of the frame.
    uint64_t m_state;

public:

    constexpr static size_t cell_count = N * N;

    constexpr static size_t States = 1ull << cell_count;

public:

    constexpr Frame();
    constexpr Frame(uint64_t state);

    /// @brief Construct a frame from a string or fail is the format is incorrect.
    /// Cells should me denoted in a following way:
    /// Dead: `.`
    /// Alive: `#`
    /// @param str input string.
    /// @return optional frame.
    [[nodiscard]] constexpr static std::optional<Frame<N>> from_str(std::string_view const& str);

    [[nodiscard]] constexpr uint64_t state() const;
    [[nodiscard]] constexpr bool get(size_t index) const;
    [[nodiscard]] constexpr bool get(size_t row, size_t col) const;

    constexpr void toggle(size_t index);

    constexpr void set(size_t index);
    constexpr void set(size_t index, bool value);
    constexpr void set(size_t row, size_t col, bool value);
    constexpr void set(size_t row, size_t col);

    [[nodiscard]] constexpr static size_t to_index(size_t row, size_t col);

    [[nodiscard]] constexpr size_t neighbour_cnt(size_t index) const;

    [[nodiscard]] constexpr auto operator<=>(Frame<N> const& other) const;
    [[nodiscard]] bool operator==(Frame<N> const& other) const;

    struct Hash {
        size_t operator()(const Frame<N> &frame) const;
    };

    /// @brief Retrieve normalized frame.
    /// @param min_transform Transform that normalizes this frame.
    /// @return Equivalent frame with minimal numerical value.
    [[nodiscard]] constexpr Frame<N> normalized(Transform &min_transform) const;

    [[nodiscard]] constexpr Frame<N> translated(size_t row_offset, size_t col_offset) const;

    /// @brief Flip frame.
    /// @tparam Horizontal indicator whether to flip around the horizontal axis.
    /// @tparam Vertical indicator whether to flip around the vertical axis.
    /// @return Flipped frame.
    template<bool Horizontal, bool Vertical>
    [[nodiscard]] constexpr Frame<N> flipped() const;

    /// @brief Transposes the frame.
    /// @tparam Anti Indicator whether to transpose about anti-diagonal (true) or the main diagonal (false).
    /// @return Transposed frame.
    template<bool Anti>
    [[nodiscard]] constexpr Frame<N> transposed() const;

    /// @brief Rotate frame
    /// @tparam Tcw Indicator whether to rotate clockwise (true) or counterclockwise (false)
    /// @return Rotated frame
    template<bool Tcw>
    [[nodiscard]] constexpr Frame<N> rotated() const;

    /// @brief Transform frame.
    /// @tparam Tid Index of the D4 group Cayley table node.
    /// Legend:
    /// I - identity transform,
    /// R - clockwise rotation of 90 degrees,
    /// V - flip about vertical axis,
    /// H - flip about horizontal axis
    /// Nodes are indexed like the following [I, V, VR, R, RR, H, HR, RRR]
    /// @return Transformed frame.
    template<size_t Tid>
    requires(Tid < 8)
    [[nodiscard]] constexpr Frame<N> transformed() const;

    /// @brief Transform frame.
    /// @param transform_index Index of the D4 group Cayley table node.
    /// Legend:
    /// I - identity transform,
    /// R - clockwise rotation of 90 degrees,
    /// V - flip about vertical axis,
    /// H - flip about horizontal axis
    /// Nodes are indexed like the following [I, V, VR, R, RR, H, HR, RRR]
    /// @return Transformed frame.
    [[nodiscard]] constexpr Frame<N> transformed(size_t transform_index) const;

    [[nodiscard]] constexpr static uint64_t get_neighbour_mask(size_t cell_row, size_t cell_col);
    [[nodiscard]] constexpr static std::array<uint64_t, cell_count> create_neighbour_mask_lut();
    [[nodiscard]] constexpr static std::array<std::array<size_t, N>, N> create_index_lut();

    constexpr static std::array<uint64_t, cell_count> neighbour_mask_lut = create_neighbour_mask_lut();
    constexpr static std::array<std::array<size_t, N>, N> index_lut = create_index_lut();
};

template<size_t Ts>
std::ostream& operator<<(std::ostream& os, const Frame<Ts>& frame);

#include <frame.tpp>