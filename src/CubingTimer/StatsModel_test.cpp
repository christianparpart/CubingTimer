// SPDX-License-Identifier: Apache-2.0
#include <catch2/catch_test_macros.hpp>

#include <CubingDB/InMemorySolveStore.hpp>
#include <CubingTimer/SessionModel.hpp>
#include <CubingTimer/StatsModel.hpp>
#include <QtTest/QSignalSpy>

using namespace CubingCore;
using CubingDB::InMemorySolveStore;
using CubingTimer::SessionModel;
using CubingTimer::StatsModel;

TEST_CASE("formatMs renders centiseconds for short solves", "[stats-model]")
{
    REQUIRE(StatsModel::formatMs(12'345) == "12.35"); // rounds
    REQUIRE(StatsModel::formatMs(1'000) == "1.00");
    REQUIRE(StatsModel::formatMs(60'000) == "1:00.00");
    REQUIRE(StatsModel::formatMs(125'500) == "2:05.50");
}

TEST_CASE("StatsModel emits changed when source adds a solve", "[stats-model]")
{
    InMemorySolveStore store;
    auto const prof = store.createProfile("t").value();
    auto const sess = store.createSession(prof.id, "s", Puzzle::ThreeByThree).value();

    SessionModel sm;
    sm.setStore(&store);
    sm.setSessionId(sess.id);

    StatsModel stats;
    stats.setSource(&sm);
    QSignalSpy changed(&stats, &StatsModel::changed);

    sm.addSolve(10'000, 0, "R U", 0, "333");
    REQUIRE(changed.count() >= 1);
    REQUIRE(stats.best() == "10.00");
}
