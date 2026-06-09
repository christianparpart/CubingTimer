// SPDX-License-Identifier: Apache-2.0
#include <CubingCore/CsvIo.h>

#include <catch2/catch_test_macros.hpp>

using namespace CubingCore;
using namespace CubingCore::csv;
using namespace std::chrono_literals;

TEST_CASE("CSV round-trip preserves every field", "[csv]")
{
    std::vector<Solve> in;

    Solve a;
    a.timestamp = Timestamp { 1'700'000'000'000ms };
    a.puzzle = Puzzle::ThreeByThree;
    a.rawTime = 12'345ms;
    a.penalty = Penalty::PlusTwo;
    a.scramble = "R U R' U' F2";
    a.comment = "felt good";
    a.inspection = 7'000ms;
    in.push_back(a);

    Solve b;
    b.timestamp = Timestamp { 1'700'000'050'000ms };
    b.puzzle = Puzzle::TwoByTwo;
    b.rawTime = 3'200ms;
    b.penalty = Penalty::Dnf;
    b.scramble = "R U' F";
    b.comment = "popped";
    in.push_back(b);

    auto const text = toCsv(in);
    auto const parsed = fromCsv(text);
    REQUIRE(parsed.has_value());
    REQUIRE(parsed->size() == 2);
    REQUIRE(parsed->at(0).rawTime == 12'345ms);
    REQUIRE(parsed->at(0).penalty == Penalty::PlusTwo);
    REQUIRE(parsed->at(0).scramble == "R U R' U' F2");
    REQUIRE(parsed->at(0).comment == "felt good");
    REQUIRE(parsed->at(0).inspection == 7'000ms);
    REQUIRE(parsed->at(0).puzzle == Puzzle::ThreeByThree);
    REQUIRE(parsed->at(1).penalty == Penalty::Dnf);
    REQUIRE_FALSE(parsed->at(1).inspection.has_value());
}

TEST_CASE("CSV quotes fields containing commas and quotes", "[csv]")
{
    std::vector<Solve> in;
    Solve a;
    a.rawTime = 1'000ms;
    a.scramble = "R U R'";
    a.comment = "has, comma and \"quote\"";
    a.puzzle = Puzzle::ThreeByThree;
    a.timestamp = Timestamp { 0ms };
    in.push_back(a);

    auto const text = toCsv(in);
    auto const parsed = fromCsv(text);
    REQUIRE(parsed.has_value());
    REQUIRE(parsed->at(0).comment == "has, comma and \"quote\"");
}

TEST_CASE("CSV rejects unknown puzzle keys", "[csv]")
{
    auto const r = fromCsv("timestamp_ms,puzzle,raw_time_ms,penalty,scramble,comment,inspection_ms\n0,777,1000,OK,,,\n");
    REQUIRE_FALSE(r.has_value());
    REQUIRE(r.error() == CsvError::UnknownPuzzle);
}
