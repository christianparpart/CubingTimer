// SPDX-License-Identifier: Apache-2.0
#include <CubingCore/Puzzle.h>

#include <catch2/catch_test_macros.hpp>

using namespace CubingCore;

TEST_CASE("axisOf groups parallel faces", "[puzzle]")
{
    REQUIRE(axisOf(Face::U) == axisOf(Face::D));
    REQUIRE(axisOf(Face::L) == axisOf(Face::R));
    REQUIRE(axisOf(Face::F) == axisOf(Face::B));

    REQUIRE(axisOf(Face::U) != axisOf(Face::R));
    REQUIRE(axisOf(Face::R) != axisOf(Face::F));
    REQUIRE(axisOf(Face::U) != axisOf(Face::F));
}

TEST_CASE("specOf returns the right spec per puzzle", "[puzzle]")
{
    REQUIRE(specOf(Puzzle::TwoByTwo).name == "222");
    REQUIRE(specOf(Puzzle::ThreeByThree).name == "333");
    REQUIRE(specOf(Puzzle::FourByFour).name == "444");

    REQUIRE_FALSE(specOf(Puzzle::TwoByTwo).allowWide);
    REQUIRE_FALSE(specOf(Puzzle::ThreeByThree).allowWide);
    REQUIRE(specOf(Puzzle::FourByFour).allowWide);

    REQUIRE(specOf(Puzzle::TwoByTwo).faces.size() == 3);
    REQUIRE(specOf(Puzzle::ThreeByThree).faces.size() == 6);
}

TEST_CASE("puzzleFromKey round-trips", "[puzzle]")
{
    REQUIRE(puzzleFromKey("222") == Puzzle::TwoByTwo);
    REQUIRE(puzzleFromKey("333") == Puzzle::ThreeByThree);
    REQUIRE(puzzleFromKey("444") == Puzzle::FourByFour);
}
