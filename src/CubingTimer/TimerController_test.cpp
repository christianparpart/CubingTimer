// SPDX-License-Identifier: Apache-2.0
#include <CubingTimer/TimerController.h>

#include <QtCore/QCoreApplication>
#include <QtTest/QSignalSpy>
#include <QtTest/QTest>

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

TEST_CASE("Hold past threshold reaches Armed, release starts Running, stop ends in Stopped", "[timer]")
{
    TimerController t;
    t.setHoldMs(100);
    QSignalSpy finished(&t, &TimerController::solveFinished);

    t.holdBegin();
    QTest::qWait(150);
    REQUIRE(t.state() == TimerController::State::Armed);

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
    QTest::qWait(100);
    t.holdEnd();
    QTest::qWait(30);
    t.stop();
    REQUIRE(t.state() == TimerController::State::Stopped);
    t.reset();
    REQUIRE(t.state() == TimerController::State::Idle);
}
