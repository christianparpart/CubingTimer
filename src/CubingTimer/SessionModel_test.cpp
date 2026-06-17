// SPDX-License-Identifier: Apache-2.0
#include <catch2/catch_test_macros.hpp>

#include <CubingDB/InMemorySolveStore.hpp>
#include <CubingTimer/SessionModel.hpp>
#include <QtTest/QSignalSpy>

using namespace CubingCore;
using CubingDB::InMemorySolveStore;
using CubingTimer::SessionModel;

namespace
{

/// Wires a SessionModel to a fresh in-memory store with one new session
/// selected, ready to accept solves. SessionModel is non-copyable (it derives
/// from QAbstractListModel), so the model is configured in place rather than
/// returned by value.
/// @param store the store to wire in (kept alive by the caller).
/// @param model the model to configure in place.
void setUpModel(InMemorySolveStore& store, SessionModel& model)
{
    auto const prof = store.createProfile("t").value();
    auto const sess = store.createSession(prof.id, "s", Puzzle::ThreeByThree).value();

    model.setStore(&store);
    model.setSessionId(sess.id);
}

} // namespace

TEST_CASE("lastPenalty / lastEffectiveTimeMs default when no solve recorded", "[session-model]")
{
    InMemorySolveStore store;
    SessionModel model;
    setUpModel(store, model);

    REQUIRE(model.lastPenalty() == static_cast<int>(Penalty::Ok));
    REQUIRE(model.lastEffectiveTimeMs() == -1);
}

TEST_CASE("lastEffectiveTimeMs reflects the most recent solve's raw time", "[session-model]")
{
    InMemorySolveStore store;
    SessionModel model;
    setUpModel(store, model);

    model.addSolve(10'000, static_cast<int>(Penalty::Ok), "R U", 0, "333");

    REQUIRE(model.lastPenalty() == static_cast<int>(Penalty::Ok));
    REQUIRE(model.lastEffectiveTimeMs() == 10'000);
}

TEST_CASE("setLastPenalty applies +2 and updates the effective time", "[session-model]")
{
    InMemorySolveStore store;
    SessionModel model;
    setUpModel(store, model);
    model.addSolve(10'000, static_cast<int>(Penalty::Ok), "R U", 0, "333");

    QSignalSpy solvesChanged(&model, &SessionModel::solvesChanged);
    QSignalSpy dataChanged(&model, &SessionModel::dataChanged);
    model.setLastPenalty(static_cast<int>(Penalty::PlusTwo));

    REQUIRE(solvesChanged.count() == 1);
    REQUIRE(model.lastPenalty() == static_cast<int>(Penalty::PlusTwo));
    REQUIRE(model.lastEffectiveTimeMs() == 12'000); // +2 seconds

    // The history view relies on dataChanged carrying the affected row plus the
    // penalty and effective-time roles so the solve row repaints in place.
    REQUIRE(dataChanged.count() == 1);
    auto const args = dataChanged.takeFirst();
    auto const topLeft = args.at(0).toModelIndex();
    REQUIRE(topLeft.row() == 0); // the only (and last) solve
    auto const roles = args.at(2).value<QList<int>>();
    REQUIRE(roles.contains(static_cast<int>(SessionModel::Roles::PenaltyRole)));
    REQUIRE(roles.contains(static_cast<int>(SessionModel::Roles::EffectiveTimeMsRole)));
}

TEST_CASE("setLastPenalty applies DNF as a -1 effective time", "[session-model]")
{
    InMemorySolveStore store;
    SessionModel model;
    setUpModel(store, model);
    model.addSolve(10'000, static_cast<int>(Penalty::Ok), "R U", 0, "333");

    model.setLastPenalty(static_cast<int>(Penalty::Dnf));

    REQUIRE(model.lastPenalty() == static_cast<int>(Penalty::Dnf));
    REQUIRE(model.lastEffectiveTimeMs() == -1);
}

TEST_CASE("setLastPenalty reverts back to OK", "[session-model]")
{
    InMemorySolveStore store;
    SessionModel model;
    setUpModel(store, model);
    model.addSolve(10'000, static_cast<int>(Penalty::Ok), "R U", 0, "333");

    model.setLastPenalty(static_cast<int>(Penalty::Dnf));
    REQUIRE(model.lastEffectiveTimeMs() == -1);

    model.setLastPenalty(static_cast<int>(Penalty::Ok));
    REQUIRE(model.lastPenalty() == static_cast<int>(Penalty::Ok));
    REQUIRE(model.lastEffectiveTimeMs() == 10'000);
}

TEST_CASE("setLastPenalty on an empty session is a no-op", "[session-model]")
{
    InMemorySolveStore store;
    SessionModel model;
    setUpModel(store, model);

    QSignalSpy solvesChanged(&model, &SessionModel::solvesChanged);
    model.setLastPenalty(static_cast<int>(Penalty::PlusTwo));

    REQUIRE(solvesChanged.count() == 0);
    REQUIRE(model.lastEffectiveTimeMs() == -1);
}

TEST_CASE("setLastPenalty persists through to the store", "[session-model]")
{
    InMemorySolveStore store;
    SessionModel model;
    setUpModel(store, model);
    model.addSolve(10'000, static_cast<int>(Penalty::Ok), "R U", 0, "333");
    auto const sessionId = model.sessionId();

    model.setLastPenalty(static_cast<int>(Penalty::PlusTwo));

    // A fresh model loading the same session must see the persisted penalty.
    SessionModel reloaded;
    reloaded.setStore(&store);
    reloaded.setSessionId(sessionId);
    REQUIRE(reloaded.lastPenalty() == static_cast<int>(Penalty::PlusTwo));
    REQUIRE(reloaded.lastEffectiveTimeMs() == 12'000);
}
