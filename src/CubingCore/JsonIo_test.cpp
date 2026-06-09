// SPDX-License-Identifier: Apache-2.0
#include <catch2/catch_test_macros.hpp>

#include <CubingCore/JsonIo.hpp>

using namespace CubingCore;
using namespace CubingCore::json;
using namespace std::chrono_literals;

TEST_CASE("JSON round-trip preserves every field", "[json]")
{
    std::vector<Solve> in;

    Solve a;
    a.timestamp = Timestamp { 1'700'000'000'000ms };
    a.puzzle = Puzzle::ThreeByThree;
    a.rawTime = 12'345ms;
    a.penalty = Penalty::Ok;
    a.scramble = "R U R' U' F2";
    a.comment = "line1\nline2";
    in.push_back(a);

    Solve b;
    b.timestamp = Timestamp { 1'700'000'050'000ms };
    b.puzzle = Puzzle::FourByFour;
    b.rawTime = 65'000ms;
    b.penalty = Penalty::Dnf;
    b.scramble = "Rw U Fw'";
    in.push_back(b);

    auto const text = toJson(in);
    auto const parsed = fromJson(text);
    REQUIRE(parsed.has_value());
    REQUIRE(parsed->size() == 2);

    REQUIRE(parsed->at(0).timestamp == a.timestamp);
    REQUIRE(parsed->at(0).rawTime == 12'345ms);
    REQUIRE(parsed->at(0).scramble == "R U R' U' F2");
    REQUIRE(parsed->at(0).comment == "line1\nline2");
    REQUIRE(parsed->at(0).puzzle == Puzzle::ThreeByThree);

    REQUIRE(parsed->at(1).puzzle == Puzzle::FourByFour);
    REQUIRE(parsed->at(1).penalty == Penalty::Dnf);
}

TEST_CASE("JSON parser rejects malformed input", "[json]")
{
    REQUIRE_FALSE(fromJson("not json").has_value());
    REQUIRE_FALSE(fromJson("[{").has_value());
}

TEST_CASE("JSON empty array yields empty vector", "[json]")
{
    auto const r = fromJson("[]");
    REQUIRE(r.has_value());
    REQUIRE(r->empty());
}
