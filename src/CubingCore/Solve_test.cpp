// SPDX-License-Identifier: Apache-2.0
#include <catch2/catch_test_macros.hpp>

#include <CubingCore/Solve.hpp>

using namespace CubingCore;
using namespace std::chrono_literals;

TEST_CASE("effectiveTime reflects penalty", "[solve]")
{
    Solve s;
    s.rawTime = 12'340ms;

    s.penalty = Penalty::Ok;
    REQUIRE(s.effectiveTime() == std::optional { 12'340ms });

    s.penalty = Penalty::PlusTwo;
    REQUIRE(s.effectiveTime() == std::optional { 14'340ms });

    s.penalty = Penalty::Dnf;
    REQUIRE_FALSE(s.effectiveTime().has_value());
}

TEST_CASE("effectiveTime preserves rawTime across penalty toggles", "[solve]")
{
    Solve s;
    s.rawTime = 9'500ms;
    s.penalty = Penalty::PlusTwo;
    REQUIRE(s.rawTime == 9'500ms); // raw is the source of truth, never mutated by penalty
    s.penalty = Penalty::Ok;
    REQUIRE(s.effectiveTime() == std::optional { 9'500ms });
}
