// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <array>
#include <cstdint>
#include <span>
#include <string_view>

namespace CubingCore
{

/// Enumeration of supported cubing puzzles for v1.
enum class Puzzle : std::uint8_t
{
    TwoByTwo,    ///< 2x2x2 (Pocket cube)
    ThreeByThree,///< 3x3x3 (Rubik's cube)
    FourByFour,  ///< 4x4x4 (Rubik's revenge)
};

/// A face of the cube. Face values are grouped into axes so that a scrambler
/// can reject moves on the same axis as the immediately previous move (and
/// after two parallel moves, reject the third).
enum class Face : std::uint8_t
{
    U, D,  ///< axis 0
    L, R,  ///< axis 1
    F, B,  ///< axis 2
};

/// The rotational amount of a move. 90 CW, 180, 90 CCW.
enum class Amount : std::uint8_t
{
    Cw,    ///< 90° clockwise (no suffix in WCA notation)
    Half,  ///< 180° (suffix "2")
    Ccw,   ///< 90° counter-clockwise (suffix "'")
};

/// A single cube move.
struct Move
{
    Face face;
    Amount amount;
    bool wide;  ///< true for wide turns (Rw/Uw/...) — only meaningful on cubes ≥4x4

    [[nodiscard]] constexpr bool operator==(Move const&) const = default;
};

/// Returns the axis index (0,1,2) for a face. Faces sharing an axis are parallel.
[[nodiscard]] constexpr int axisOf(Face face) noexcept
{
    return static_cast<int>(face) / 2;
}

/// Data-driven specification of how a puzzle is scrambled.
/// Per project guidelines, all puzzle-specific knowledge is data, not branching.
struct PuzzleSpec
{
    Puzzle puzzle;
    std::string_view name;       ///< WCA-style name ("222", "333", "444")
    std::span<Face const> faces; ///< faces usable in scrambles (2x2 fixes one corner → 3 faces)
    bool allowWide;              ///< whether Rw/Uw etc. are allowed
    int length;                  ///< standard scramble length (move count)
};

namespace detail
{
    inline constexpr std::array faces2x2 = { Face::U, Face::R, Face::F };
    inline constexpr std::array faces3x3 = { Face::U, Face::D, Face::L, Face::R, Face::F, Face::B };
    inline constexpr std::array faces4x4 = { Face::U, Face::D, Face::L, Face::R, Face::F, Face::B };
} // namespace detail

inline constexpr PuzzleSpec Spec2x2 {
    .puzzle = Puzzle::TwoByTwo,
    .name = "222",
    .faces = detail::faces2x2,
    .allowWide = false,
    .length = 11,
};

inline constexpr PuzzleSpec Spec3x3 {
    .puzzle = Puzzle::ThreeByThree,
    .name = "333",
    .faces = detail::faces3x3,
    .allowWide = false,
    .length = 20,
};

inline constexpr PuzzleSpec Spec4x4 {
    .puzzle = Puzzle::FourByFour,
    .name = "444",
    .faces = detail::faces4x4,
    .allowWide = true,
    .length = 45,
};

/// Returns the canonical spec for a puzzle.
[[nodiscard]] constexpr PuzzleSpec const& specOf(Puzzle puzzle) noexcept
{
    switch (puzzle)
    {
        case Puzzle::TwoByTwo: return Spec2x2;
        case Puzzle::ThreeByThree: return Spec3x3;
        case Puzzle::FourByFour: return Spec4x4;
    }
    return Spec3x3;
}

/// Parses a WCA-style puzzle key ("222"/"333"/"444") back into Puzzle.
/// Returns ThreeByThree on unknown input — calling code should validate first
/// if it cares about that distinction.
[[nodiscard]] constexpr Puzzle puzzleFromKey(std::string_view key) noexcept
{
    if (key == "222") return Puzzle::TwoByTwo;
    if (key == "444") return Puzzle::FourByFour;
    return Puzzle::ThreeByThree;
}

} // namespace CubingCore
