// SPDX-License-Identifier: Apache-2.0
#include <catch2/catch_test_macros.hpp>

#include <CubingCore/Stats.hpp>

using namespace CubingCore;
using namespace CubingCore::stats;
using namespace std::chrono_literals;

namespace
{
Solve mk(int rawMs, Penalty penalty = Penalty::Ok)
{
    Solve s;
    s.rawTime = Milliseconds { rawMs };
    s.penalty = penalty;
    return s;
}
} // namespace

TEST_CASE("best returns NotEnoughSolves on empty", "[stats]")
{
    std::vector<Solve> empty;
    REQUIRE_FALSE(best(empty).has_value());
}

TEST_CASE("best skips DNFs", "[stats]")
{
    std::vector<Solve> solves { mk(10'000), mk(0, Penalty::Dnf), mk(8'500), mk(12'000) };
    REQUIRE(best(solves).value() == 8'500ms);
}

TEST_CASE("best applies +2 penalty before comparing", "[stats]")
{
    // raw 9.0s with +2 → 11.0s; raw 10.0s OK is the actual winner.
    std::vector<Solve> solves { mk(9'000, Penalty::PlusTwo), mk(10'000) };
    REQUIRE(best(solves).value() == 10'000ms);
}

TEST_CASE("meanOfLast Mo3 — any DNF → DNF", "[stats]")
{
    std::vector<Solve> solves { mk(10'000), mk(0, Penalty::Dnf), mk(11'000) };
    auto const m = meanOfLast(solves, 3);
    REQUIRE(m.has_value());
    REQUIRE(m->isDnf());
}

TEST_CASE("meanOfLast Mo3 — all OK averages cleanly", "[stats]")
{
    std::vector<Solve> solves { mk(10'000), mk(11'000), mk(12'000) };
    auto const m = meanOfLast(solves, 3);
    REQUIRE(m.has_value());
    REQUIRE_FALSE(m->isDnf());
    REQUIRE(m->value == std::optional { 11'000ms });
}

TEST_CASE("meanOfLast returns NotEnoughSolves under window size", "[stats]")
{
    std::vector<Solve> solves { mk(10'000), mk(11'000) };
    REQUIRE_FALSE(meanOfLast(solves, 3).has_value());
}

TEST_CASE("averageOfLast Ao5 trims best+worst (no DNFs)", "[stats]")
{
    // Times: 10, 12, 8, 14, 11 — trim best (8) and worst (14), mean of {10, 12, 11} = 11
    std::vector<Solve> solves {
        mk(10'000), mk(12'000), mk(8'000), mk(14'000), mk(11'000),
    };
    auto const r = averageOfLast(solves, 5);
    REQUIRE(r.has_value());
    REQUIRE_FALSE(r->isDnf());
    REQUIRE(r->value == std::optional { 11'000ms });
}

TEST_CASE("averageOfLast Ao5 with one DNF — DNF counts as the worst (trimmed)", "[stats]")
{
    // Times: 10, 12, DNF, 8, 11 — trim best (8) and worst (DNF), mean of {10, 12, 11} = 11
    std::vector<Solve> solves {
        mk(10'000), mk(12'000), mk(0, Penalty::Dnf), mk(8'000), mk(11'000),
    };
    auto const r = averageOfLast(solves, 5);
    REQUIRE(r.has_value());
    REQUIRE_FALSE(r->isDnf());
    REQUIRE(r->value == std::optional { 11'000ms });
}

TEST_CASE("averageOfLast Ao5 with two DNFs → DNF", "[stats]")
{
    std::vector<Solve> solves {
        mk(10'000), mk(0, Penalty::Dnf), mk(0, Penalty::Dnf), mk(8'000), mk(11'000),
    };
    auto const r = averageOfLast(solves, 5);
    REQUIRE(r.has_value());
    REQUIRE(r->isDnf());
}

TEST_CASE("averageOfLast applies +2 before trimming", "[stats]")
{
    // raw 8.0 +2 → 10.0; comparing as effective times.
    // Effective: 10 (+2 from 8), 12, 9, 14, 11 — trim 9 and 14 → {10, 12, 11} = 11
    std::vector<Solve> solves {
        mk(8'000, Penalty::PlusTwo), mk(12'000), mk(9'000), mk(14'000), mk(11'000),
    };
    auto const r = averageOfLast(solves, 5);
    REQUIRE(r.has_value());
    REQUIRE_FALSE(r->isDnf());
    REQUIRE(r->value == std::optional { 11'000ms });
}

TEST_CASE("averageOfLast Ao12 trim is 1 per side", "[stats]")
{
    // 12 OK solves 10..21s → trim 10 and 21 → mean(11..20) = 15500ms
    std::vector<Solve> solves;
    solves.reserve(12);
    for (int i = 0; i < 12; ++i)
        solves.push_back(mk((10 + i) * 1'000));
    auto const r = averageOfLast(solves, 12);
    REQUIRE(r.has_value());
    REQUIRE_FALSE(r->isDnf());
    REQUIRE(r->value == std::optional { 15'500ms });
}

TEST_CASE("averageOfLast Ao100 trim is 5 per side", "[stats]")
{
    // 100 OK solves 1..100ms → trim {1..5} and {96..100} → mean(6..95) = (6+95)/2 = 50.5
    // integer ms: sum 6..95 = sum(1..95) - sum(1..5) = 4560 - 15 = 4545, /90 = 50 (truncates)
    std::vector<Solve> solves;
    solves.reserve(100);
    for (int i = 1; i <= 100; ++i)
        solves.push_back(mk(i));
    auto const r = averageOfLast(solves, 100);
    REQUIRE(r.has_value());
    REQUIRE_FALSE(r->isDnf());
    REQUIRE(r->value == std::optional { 50ms });
}

TEST_CASE("bestAverageOf finds the best window", "[stats]")
{
    // Build 6 solves; the second Ao5 window should be strictly better.
    std::vector<Solve> solves {
        mk(20'000), mk(20'000), mk(20'000), mk(20'000), mk(20'000), mk(5'000),
    };
    // first  window [0..4]: trim → {20,20,20} → 20s
    // second window [1..5]: trim min=5 and max=20, mean({20,20,20}) = 20s — wait, identical
    // Make it tilt: change last entry to 1ms? Already does — they tie because everything is 20s except the new 5s extreme
    // that gets trimmed. Adjust to make the second strictly better:
    solves = std::vector<Solve> {
        mk(30'000), mk(20'000), mk(20'000), mk(20'000), mk(20'000), mk(10'000),
    };
    auto const r = bestAverageOf(solves, 5);
    REQUIRE(r.has_value());
    REQUIRE_FALSE(r->isDnf());
    // window [1..5]: values 20,20,20,20,10 → trim 10 and 20, mean({20,20,20}) = 20s
    // window [0..4]: values 30,20,20,20,20 → trim 20 and 30, mean({20,20,20}) = 20s
    REQUIRE(r->value == std::optional { 20'000ms });
}

TEST_CASE("filterLastDays keeps recent solves only", "[stats]")
{
    auto const now = Timestamp { 1'000'000ms };
    std::vector<Solve> solves;
    Solve a;
    a.rawTime = 10'000ms;
    a.timestamp = now - std::chrono::hours(24 * 100); // 100 days old
    solves.push_back(a);
    a.timestamp = now - std::chrono::hours(24 * 10); // 10 days old
    solves.push_back(a);
    auto const filtered = filterLastDays(solves, now, 90);
    REQUIRE(filtered.size() == 1);
}
