// SPDX-License-Identifier: Apache-2.0
#include <catch2/catch_test_macros.hpp>

#include <CubingDB/InMemorySolveStore.hpp>
#include <QtSql/QSqlDatabase>

using namespace CubingCore;
using namespace std::chrono_literals;
using CubingDB::InMemorySolveStore;

TEST_CASE("InMemorySolveStore creates profile then lists it", "[db]")
{
    InMemorySolveStore store;
    auto const p = store.createProfile("alice");
    REQUIRE(p.has_value());
    REQUIRE(p->id > 0);
    REQUIRE(p->name == "alice");

    auto const list = store.listProfiles();
    REQUIRE(list.has_value());
    REQUIRE(list->size() == 1);
    REQUIRE(list->at(0).id == p->id);
}

TEST_CASE("InMemorySolveStore round-trips a solve", "[db]")
{
    InMemorySolveStore store;
    auto const prof = store.createProfile("bob").value();
    auto const sess = store.createSession(prof.id, "weekend", Puzzle::ThreeByThree).value();

    Solve s;
    s.sessionId = sess.id;
    s.puzzle = Puzzle::ThreeByThree;
    s.timestamp = Timestamp { 1'700'000'000'000ms };
    s.rawTime = 12'345ms;
    s.penalty = Penalty::PlusTwo;
    s.scramble = "R U R' U'";
    s.comment = "ok";
    s.inspection = 8'000ms;

    auto const added = store.addSolve(s);
    REQUIRE(added.has_value());
    REQUIRE(added->id > 0);

    auto const loaded = store.loadSession(sess.id);
    REQUIRE(loaded.has_value());
    REQUIRE(loaded->size() == 1);
    auto const& got = loaded->at(0);
    REQUIRE(got.rawTime == 12'345ms);
    REQUIRE(got.penalty == Penalty::PlusTwo);
    REQUIRE(got.scramble == "R U R' U'");
    REQUIRE(got.comment == "ok");
    REQUIRE(got.inspection == 8'000ms);
    REQUIRE(got.puzzle == Puzzle::ThreeByThree);
}

TEST_CASE("deleting a profile cascades to sessions and solves", "[db]")
{
    InMemorySolveStore store;
    auto const prof = store.createProfile("carol").value();
    auto const sess = store.createSession(prof.id, "main", Puzzle::TwoByTwo).value();

    Solve s;
    s.sessionId = sess.id;
    s.puzzle = Puzzle::TwoByTwo;
    s.timestamp = Timestamp { 0ms };
    s.rawTime = 1'000ms;
    REQUIRE(store.addSolve(s).has_value());

    REQUIRE(store.deleteProfile(prof.id).has_value());
    REQUIRE(store.listProfiles().value().empty());
    REQUIRE(store.listSessions(prof.id).value().empty());
    REQUIRE(store.loadSession(sess.id).value().empty());
}

TEST_CASE("updateSolve modifies penalty and comment", "[db]")
{
    InMemorySolveStore store;
    auto const prof = store.createProfile("dave").value();
    auto const sess = store.createSession(prof.id, "s", Puzzle::ThreeByThree).value();

    Solve s;
    s.sessionId = sess.id;
    s.puzzle = Puzzle::ThreeByThree;
    s.timestamp = Timestamp { 0ms };
    s.rawTime = 10'000ms;
    auto added = store.addSolve(s).value();

    added.penalty = Penalty::Dnf;
    added.comment = "popped";
    REQUIRE(store.updateSolve(added).has_value());

    auto const reloaded = store.loadSession(sess.id).value();
    REQUIRE(reloaded.size() == 1);
    REQUIRE(reloaded[0].penalty == Penalty::Dnf);
    REQUIRE(reloaded[0].comment == "popped");
}

TEST_CASE("deleteSolve returns NotFound on missing id", "[db]")
{
    InMemorySolveStore store;
    auto const r = store.deleteSolve(99999);
    REQUIRE_FALSE(r.has_value());
    REQUIRE(r.error() == StoreError::NotFound);
}
