// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include <CubingCore/Puzzle.hpp>

namespace CubingCore
{

/// A function that yields a uniformly distributed 32-bit value. Injecting the
/// RNG keeps the scrambler deterministic in tests.
using RandomFn = std::function<std::uint32_t()>;

/// Random-move scrambler. The same engine handles 2x2/3x3/4x4 — only the
/// PuzzleSpec varies.
///
/// Constraints enforced:
///   - no two consecutive moves on the same face,
///   - no three consecutive moves on the same axis (avoids redundant U-D-U etc.).
///
/// This is not a WCA-legal random-state scrambler (those are based on
/// Kociemba's two-phase algorithm); random-move is what csTimer uses by default
/// and is perfectly suitable for practice.
class Scrambler
{
  public:
    /// Constructs a scrambler for the given puzzle spec, drawing entropy from
    /// an injected RNG (the seam that keeps tests deterministic).
    /// @param spec puzzle specification (faces, wide-turn allowance, length).
    /// @param rng  function returning a uniformly distributed 32-bit value.
    Scrambler(PuzzleSpec const& spec, RandomFn rng);

    /// Generates one scramble as a vector of moves.
    /// @return scramble of `spec.length` moves obeying the adjacency rules.
    [[nodiscard]] std::vector<Move> generate() const;

    /// Generates one scramble and formats it as a WCA-notation string ("R U R' …").
    /// @return scramble formatted with single-space separators.
    [[nodiscard]] std::string generateString() const;

    /// Formats a sequence of moves as a space-separated WCA notation string.
    /// @param moves moves to format in order.
    /// @return moves joined by single spaces.
    [[nodiscard]] static std::string format(std::vector<Move> const& moves);

    /// Formats a single move (e.g. "R", "U'", "Rw2") in WCA notation.
    /// @param move move to format.
    /// @return formatted move string (1–3 characters).
    [[nodiscard]] static std::string formatMove(Move move);

  private:
    PuzzleSpec const* _spec;
    mutable RandomFn _rng;
};

} // namespace CubingCore
