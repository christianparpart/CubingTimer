// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <CubingCore/Solve.h>

#include <chrono>
#include <expected>
#include <optional>
#include <span>

namespace CubingCore::stats
{

/// Why a statistic could not be computed.
enum class StatsError
{
    NotEnoughSolves, ///< fewer solves than the window size
    AllDnf,          ///< the relevant window is all-DNF / can't be averaged
};

/// A computed average: either a duration, or DNF (engaged when too many DNFs).
struct AverageResult
{
    std::optional<Milliseconds> value; ///< nullopt ⇒ DNF average

    [[nodiscard]] bool isDnf() const noexcept { return !value.has_value(); }
};

/// Best single (effective time, +2 applied) across the given solves.
/// DNFs are ignored.
/// @param solves solves to scan.
/// @return best non-DNF effective time, or `NotEnoughSolves` if all are DNF.
[[nodiscard]] std::expected<Milliseconds, StatsError> best(std::span<Solve const> solves);

/// Mean of the last `n` solves (typically `n=3`).
/// ANY DNF in the window collapses the result to DNF.
/// @param solves source solves (most recent at the tail).
/// @param n      window size.
/// @return arithmetic mean, DNF, or `NotEnoughSolves`.
[[nodiscard]] std::expected<AverageResult, StatsError> meanOfLast(std::span<Solve const> solves, std::size_t n);

/// WCA "Average of N": drop `ceil(n/20)` best and `ceil(n/20)` worst, mean
/// the rest. More than `ceil(n/20)` DNFs collapse the result to DNF.
/// For `n=5` and `n=12` the trim is 1 per side; for `n=100` it's 5 per side.
/// @param solves source solves (most recent at the tail).
/// @param n      window size.
/// @return trimmed mean, DNF, or `NotEnoughSolves`.
[[nodiscard]] std::expected<AverageResult, StatsError> averageOfLast(std::span<Solve const> solves,
                                                                      std::size_t n);

/// The best AoN over any window in the entire history (PB Ao5, etc.).
/// @param solves source solves.
/// @param n      window size.
/// @return best trimmed mean, or DNF/`NotEnoughSolves`.
[[nodiscard]] std::expected<AverageResult, StatsError> bestAverageOf(std::span<Solve const> solves,
                                                                      std::size_t n);

/// The best mean-of-n over any window in the entire history.
/// @param solves source solves.
/// @param n      window size.
/// @return best arithmetic mean, or DNF/`NotEnoughSolves`.
[[nodiscard]] std::expected<AverageResult, StatsError> bestMeanOf(std::span<Solve const> solves,
                                                                   std::size_t n);

/// Filters solves to a rolling window ending at `now`. Solves with
/// `timestamp >= now - days*86400s` are kept; input ordering is preserved.
/// @param solves source solves.
/// @param now    end of the window (exclusive upper bound is open).
/// @param days   length of the window in days.
/// @return solves whose timestamps fall in the window.
[[nodiscard]] std::vector<Solve> filterLastDays(std::span<Solve const> solves,
                                                Timestamp now,
                                                int days);

} // namespace CubingCore::stats
