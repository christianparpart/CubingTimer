// SPDX-License-Identifier: Apache-2.0
#include <catch2/catch_test_macros.hpp>

#include <random>

#include <CubingCore/Scrambler.hpp>

using namespace CubingCore;

namespace
{
/// Builds a seeded RNG for deterministic tests.
RandomFn seededRng(std::uint32_t seed)
{
    auto engine = std::make_shared<std::mt19937>(seed);
    return [engine]() {
        return (*engine)();
    };
}
} // namespace

TEST_CASE("Scrambler produces moves of the requested length", "[scrambler]")
{
    Scrambler s2(Spec2x2, seededRng(1));
    REQUIRE(static_cast<int>(s2.generate().size()) == Spec2x2.length);

    Scrambler s3(Spec3x3, seededRng(2));
    REQUIRE(static_cast<int>(s3.generate().size()) == Spec3x3.length);

    Scrambler s4(Spec4x4, seededRng(3));
    REQUIRE(static_cast<int>(s4.generate().size()) == Spec4x4.length);
}

TEST_CASE("Scrambler avoids repeating the same face consecutively", "[scrambler]")
{
    Scrambler s(Spec3x3, seededRng(42));
    auto const moves = s.generate();
    REQUIRE(moves.size() > 1);
    for (std::size_t i = 1; i < moves.size(); ++i)
        REQUIRE(moves[i].face != moves[i - 1].face);
}

TEST_CASE("Scrambler avoids three consecutive moves on the same axis", "[scrambler]")
{
    Scrambler s(Spec3x3, seededRng(7));
    auto const moves = s.generate();
    for (std::size_t i = 2; i < moves.size(); ++i)
    {
        auto const a0 = axisOf(moves[i - 2].face);
        auto const a1 = axisOf(moves[i - 1].face);
        auto const a2 = axisOf(moves[i].face);
        REQUIRE_FALSE((a0 == a1 && a1 == a2));
    }
}

TEST_CASE("Scrambler is deterministic for a fixed seed", "[scrambler]")
{
    Scrambler s1(Spec3x3, seededRng(1234));
    Scrambler s2(Spec3x3, seededRng(1234));
    REQUIRE(s1.generate() == s2.generate());
}

TEST_CASE("2x2 scrambler only uses U/R/F faces (fix-a-corner convention)", "[scrambler]")
{
    Scrambler s(Spec2x2, seededRng(11));
    auto const moves = s.generate();
    for (auto const& m: moves)
    {
        REQUIRE((m.face == Face::U || m.face == Face::R || m.face == Face::F));
        REQUIRE_FALSE(m.wide);
    }
}

TEST_CASE("4x4 scrambler emits some wide turns", "[scrambler]")
{
    Scrambler s(Spec4x4, seededRng(99));
    auto const moves = s.generate();
    bool anyWide = false;
    for (auto const& m: moves)
        if (m.wide)
        {
            anyWide = true;
            break;
        }
    REQUIRE(anyWide);
}

TEST_CASE("Move formatting matches WCA notation", "[scrambler]")
{
    REQUIRE(Scrambler::formatMove({ .face = Face::R, .amount = Amount::Cw, .wide = false }) == "R");
    REQUIRE(Scrambler::formatMove({ .face = Face::U, .amount = Amount::Half, .wide = false }) == "U2");
    REQUIRE(Scrambler::formatMove({ .face = Face::L, .amount = Amount::Ccw, .wide = false }) == "L'");
    REQUIRE(Scrambler::formatMove({ .face = Face::R, .amount = Amount::Cw, .wide = true }) == "Rw");
    REQUIRE(Scrambler::formatMove({ .face = Face::F, .amount = Amount::Ccw, .wide = true }) == "Fw'");
}

TEST_CASE("Scramble string joins moves with single spaces", "[scrambler]")
{
    std::vector<Move> moves {
        { .face = Face::R, .amount = Amount::Cw, .wide = false },
        { .face = Face::U, .amount = Amount::Ccw, .wide = false },
        { .face = Face::F, .amount = Amount::Half, .wide = false },
    };
    REQUIRE(Scrambler::format(moves) == "R U' F2");
}
