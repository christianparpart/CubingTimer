// SPDX-License-Identifier: Apache-2.0
#include <CubingCore/Stats.h>

#include <algorithm>
#include <numeric>
#include <ranges>

namespace CubingCore::stats
{

namespace
{
    /// Computes ceil(n / divisor) for positive integers.
    /// @param n        numerator.
    /// @param divisor  positive divisor.
    /// @return         smallest k such that k * divisor >= n.
    constexpr std::size_t ceilDiv(std::size_t n, std::size_t divisor) noexcept
    {
        return (n + divisor - 1) / divisor;
    }
} // namespace

std::expected<Milliseconds, StatsError> best(std::span<Solve const> solves)
{
    std::optional<Milliseconds> bestSeen;
    for (auto const& s: solves)
    {
        auto const t = s.effectiveTime();
        if (!t)
            continue;
        if (!bestSeen || *t < *bestSeen)
            bestSeen = *t;
    }
    if (!bestSeen)
        return std::unexpected(StatsError::NotEnoughSolves);
    return *bestSeen;
}

std::expected<AverageResult, StatsError> meanOfLast(std::span<Solve const> solves, std::size_t n)
{
    if (solves.size() < n)
        return std::unexpected(StatsError::NotEnoughSolves);

    auto const tail = solves.last(n);

    Milliseconds sum { 0 };
    for (auto const& s: tail)
    {
        auto const t = s.effectiveTime();
        if (!t)
            return AverageResult { .value = std::nullopt };
        sum += *t;
    }
    return AverageResult { .value = sum / static_cast<Milliseconds::rep>(n) };
}

std::expected<AverageResult, StatsError> averageOfLast(std::span<Solve const> solves, std::size_t n)
{
    if (solves.size() < n)
        return std::unexpected(StatsError::NotEnoughSolves);

    auto const tail = solves.last(n);
    auto const trim = ceilDiv(n, 20U); // drop ceil(n/20) from each end

    // Collect effective times in a form that's order-comparable while keeping DNFs sortable to "worst".
    // We represent a solve's contribution as (isDnf, time). Sorting ascending puts non-DNFs first.
    struct Entry
    {
        bool dnf;
        Milliseconds value; // unused if dnf
    };
    std::vector<Entry> entries;
    entries.reserve(n);
    std::size_t dnfCount = 0;
    for (auto const& s: tail)
    {
        auto const t = s.effectiveTime();
        if (t)
            entries.push_back({ .dnf = false, .value = *t });
        else
        {
            entries.push_back({ .dnf = true, .value = Milliseconds { 0 } });
            ++dnfCount;
        }
    }

    // More DNFs than the "worst-trim" budget → DNF average.
    if (dnfCount > trim)
        return AverageResult { .value = std::nullopt };

    std::ranges::sort(entries, [](Entry const& a, Entry const& b) {
        if (a.dnf != b.dnf)
            return !a.dnf; // non-DNFs come first
        return a.value < b.value;
    });

    // Drop first `trim` (best) and last `trim` (worst, which includes any DNFs).
    auto const remaining = std::span<Entry const>(entries).subspan(trim, entries.size() - (2 * trim));

    Milliseconds sum { 0 };
    for (auto const& e: remaining)
    {
        // The trim budget ensured no DNFs survived to the middle slice.
        sum += e.value;
    }
    auto const count = static_cast<Milliseconds::rep>(remaining.size());
    return AverageResult { .value = sum / count };
}

std::expected<AverageResult, StatsError> bestAverageOf(std::span<Solve const> solves, std::size_t n)
{
    if (solves.size() < n)
        return std::unexpected(StatsError::NotEnoughSolves);

    Milliseconds bestSeen { 0 };
    bool sawValid = false;
    for (std::size_t end = n; end <= solves.size(); ++end)
    {
        auto const window = solves.subspan(end - n, n);
        auto const avg = averageOfLast(window, n);
        if (!avg || avg->isDnf() || !avg->value.has_value())
            continue;
        auto const candidate = *avg->value;
        if (!sawValid || candidate < bestSeen)
        {
            bestSeen = candidate;
            sawValid = true;
        }
    }
    if (!sawValid)
        return AverageResult { .value = std::nullopt };
    return AverageResult { .value = bestSeen };
}

std::expected<AverageResult, StatsError> bestMeanOf(std::span<Solve const> solves, std::size_t n)
{
    if (solves.size() < n)
        return std::unexpected(StatsError::NotEnoughSolves);

    Milliseconds bestSeen { 0 };
    bool sawValid = false;
    for (std::size_t end = n; end <= solves.size(); ++end)
    {
        auto const window = solves.subspan(end - n, n);
        auto const m = meanOfLast(window, n);
        if (!m || m->isDnf() || !m->value.has_value())
            continue;
        auto const candidate = *m->value;
        if (!sawValid || candidate < bestSeen)
        {
            bestSeen = candidate;
            sawValid = true;
        }
    }
    if (!sawValid)
        return AverageResult { .value = std::nullopt };
    return AverageResult { .value = bestSeen };
}

std::vector<Solve> filterLastDays(std::span<Solve const> solves, Timestamp now, int days)
{
    auto const cutoff = now - std::chrono::hours(24 * days);
    std::vector<Solve> result;
    result.reserve(solves.size());
    for (auto const& s: solves)
        if (s.timestamp >= cutoff)
            result.push_back(s);
    return result;
}

} // namespace CubingCore::stats
