// SPDX-License-Identifier: Apache-2.0
#include <CubingDB/InMemorySolveStore.h>
#include <CubingTimer/SessionModel.h>
#include <CubingTimer/StatsModel.h>

#include <QtCore/QCoreApplication>
#include <QtTest/QSignalSpy>

#include <catch2/catch_test_macros.hpp>

namespace
{
    struct QtFixture
    {
        QtFixture()
        {
            if (!QCoreApplication::instance())
            {
                static int argc = 0;
                static char* argv[] = { nullptr };
                static QCoreApplication app(argc, argv);
                (void) app;
            }
        }
    };
    QtFixture gQt;
} // namespace

using namespace CubingCore;
using CubingTimer::SessionModel;
using CubingTimer::StatsModel;
using CubingDB::InMemorySolveStore;

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
