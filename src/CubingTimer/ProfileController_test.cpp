// SPDX-License-Identifier: Apache-2.0
#include <catch2/catch_test_macros.hpp>

#include <CubingDB/InMemorySolveStore.hpp>
#include <CubingTimer/ProfileController.hpp>
#include <QtTest/QSignalSpy>

using CubingDB::InMemorySolveStore;
using CubingTimer::ProfileController;

namespace
{
/// Creates a controller backed by a fresh in-memory store, with `extra` extra
/// profiles added on top of the auto-created "default" one. Returns the
/// controller and the store (the store must outlive the controller).
struct Fixture
{
    InMemorySolveStore store;
    ProfileController controller;

    explicit Fixture(int extraProfiles = 0)
    {
        controller.setStore(&store);
        for (int i = 0; i < extraProfiles; ++i)
            controller.createProfile(QString("p%1").arg(i + 1));
    }
};
} // namespace

TEST_CASE("ProfileController auto-creates a default profile on first run", "[profile]")
{
    Fixture fx;
    auto const names = fx.controller.profileNames();
    REQUIRE(names.size() == 1);
    REQUIRE(names.at(0) == "default");
    REQUIRE(fx.controller.currentProfileId() > 0);
}

TEST_CASE("renameProfile updates listing and currentProfileName when active", "[profile]")
{
    Fixture fx;
    QSignalSpy nameSpy(&fx.controller, &ProfileController::currentProfileChanged);

    fx.controller.renameProfile(0, "alice");

    REQUIRE(fx.controller.profileNames().at(0) == "alice");
    REQUIRE(fx.controller.currentProfileName() == "alice");
    REQUIRE(nameSpy.count() >= 1);
}

TEST_CASE("renameProfile ignores empty/whitespace names", "[profile]")
{
    Fixture fx;
    fx.controller.renameProfile(0, "   ");
    REQUIRE(fx.controller.profileNames().at(0) == "default");
}

TEST_CASE("renameProfile is a no-op for out-of-range index", "[profile]")
{
    Fixture fx;
    fx.controller.renameProfile(42, "won't apply");
    REQUIRE(fx.controller.profileNames().at(0) == "default");
}

TEST_CASE("deleteProfile of last remaining profile recreates a default", "[profile]")
{
    Fixture fx;
    auto const originalId = fx.controller.currentProfileId();

    fx.controller.deleteProfile(0);

    auto const names = fx.controller.profileNames();
    REQUIRE(names.size() == 1);
    REQUIRE(names.at(0) == "default");
    REQUIRE(fx.controller.currentProfileId() != originalId);
    REQUIRE(fx.controller.currentProfileId() > 0);
}

TEST_CASE("deleteProfile switches to first remaining when deleting the active profile", "[profile]")
{
    Fixture fx(2); // default + p1 + p2; current is p2 (newly created)
    REQUIRE(fx.controller.profileNames().size() == 3);
    auto const currentBefore = fx.controller.currentProfileId();

    // Delete the active profile (last in the list).
    fx.controller.deleteProfile(2);

    auto const names = fx.controller.profileNames();
    REQUIRE(names.size() == 2);
    REQUIRE(fx.controller.currentProfileId() != currentBefore);
    // currentProfile should now be one of the survivors.
    REQUIRE_FALSE(fx.controller.currentProfileName().isEmpty());
}

TEST_CASE("deleteProfile preserves the active profile when deleting another", "[profile]")
{
    Fixture fx(2); // default, p1, p2; current is p2
    auto const stayId = fx.controller.currentProfileId();

    // Delete "p1" (index 1, not the current one).
    fx.controller.deleteProfile(1);

    REQUIRE(fx.controller.profileNames().size() == 2);
    REQUIRE(fx.controller.currentProfileId() == stayId);
}

TEST_CASE("moveProfile reorders the listing and persists", "[profile]")
{
    Fixture fx(2);
    // Initial: [default, p1, p2]
    auto const before = fx.controller.profileNames();
    REQUIRE(before.size() == 3);
    REQUIRE(before.at(0) == "default");
    REQUIRE(before.at(1) == "p1");
    REQUIRE(before.at(2) == "p2");

    fx.controller.moveProfile(2, 0);

    auto const after = fx.controller.profileNames();
    REQUIRE(after.at(0) == "p2");
    REQUIRE(after.at(1) == "default");
    REQUIRE(after.at(2) == "p1");

    // Reload should yield the same order (proves it's persisted).
    fx.controller.reload();
    auto const reloaded = fx.controller.profileNames();
    REQUIRE(reloaded.at(0) == "p2");
    REQUIRE(reloaded.at(1) == "default");
    REQUIRE(reloaded.at(2) == "p1");
}

TEST_CASE("moveProfile rejects out-of-range indices", "[profile]")
{
    Fixture fx(2);
    auto const before = fx.controller.profileNames();
    fx.controller.moveProfile(-1, 0);
    fx.controller.moveProfile(0, 99);
    fx.controller.moveProfile(0, 0); // same position
    REQUIRE(fx.controller.profileNames() == before);
}
