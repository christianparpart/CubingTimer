// SPDX-License-Identifier: Apache-2.0
#include <catch2/catch_test_macros.hpp>

#include <CubingTimer/TimerController.hpp>
#include <QtTest/QSignalSpy>
#include <QtTest/QTest>

using CubingTimer::TimerController;

TEST_CASE("Initial state is Idle", "[timer]")
{
    TimerController t;
    REQUIRE(t.state() == TimerController::State::Idle);
}

TEST_CASE("Hold-pending then released too soon returns to Idle", "[timer]")
{
    TimerController t;
    t.setHoldMs(200);
    t.holdBegin();
    REQUIRE(t.state() == TimerController::State::HoldPending);
    QTest::qWait(50);
    t.holdEnd();
    REQUIRE(t.state() == TimerController::State::Idle);
}

namespace
{
/// Polls until `state` is reached or `timeoutMs` elapses, returning whether the
/// state was observed. Uses Qt's event loop so QTimer fires. Lets us tolerate
/// CI jitter under ASan/UBSan without sleeping a fixed (long) amount.
bool waitForState(TimerController const& t, TimerController::State state, int timeoutMs = 2000)
{
    QElapsedTimer clock;
    clock.start();
    while (clock.elapsed() < timeoutMs)
    {
        if (t.state() == state)
            return true;
        QTest::qWait(10);
    }
    return t.state() == state;
}
} // namespace

TEST_CASE("Hold past threshold reaches Armed, release starts Running, stop ends in Stopped", "[timer]")
{
    TimerController t;
    t.setHoldMs(100);
    QSignalSpy finished(&t, &TimerController::solveFinished);

    t.holdBegin();
    REQUIRE(waitForState(t, TimerController::State::Armed));

    t.holdEnd();
    REQUIRE(t.state() == TimerController::State::Running);

    QTest::qWait(120);
    t.stop();
    REQUIRE(t.state() == TimerController::State::Stopped);
    REQUIRE(finished.count() == 1);
    auto const args = finished.takeFirst();
    REQUIRE_FALSE(args.empty());
    auto const rawMs = args.at(0).toLongLong();
    REQUIRE(rawMs >= 100);
}

TEST_CASE("Reset returns to Idle from Stopped", "[timer]")
{
    TimerController t;
    t.setHoldMs(50);
    t.holdBegin();
    REQUIRE(waitForState(t, TimerController::State::Armed));
    t.holdEnd();
    QTest::qWait(30);
    t.stop();
    REQUIRE(t.state() == TimerController::State::Stopped);
    t.reset();
    REQUIRE(t.state() == TimerController::State::Idle);
}
