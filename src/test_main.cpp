// SPDX-License-Identifier: Apache-2.0
//
// Custom Catch2 main for tests that need a QCoreApplication.
//
// Why not Catch2WithMain? Because plain Catch2WithMain destructs at
// program-exit time. Qt's static teardown (plugin unloads, etc.) races with
// Catch2's atexit handlers and segfaults on Linux/macOS CI. Constructing
// QCoreApplication here and calling _Exit() after the test run skips those
// destructors entirely.

#include <QtCore/QCoreApplication>

#include <catch2/catch_session.hpp>

#include <cstdlib>

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    int const rc = Catch::Session().run(argc, argv);
    // Skip global static destructors — they race with Qt's plugin teardown.
    std::_Exit(rc);
}
