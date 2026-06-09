// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <CubingCore/Profile.h>
#include <CubingCore/Session.h>
#include <CubingCore/Solve.h>

#include <cstdint>
#include <expected>
#include <string>
#include <string_view>
#include <vector>

namespace CubingCore
{

/// Reasons a store call may fail. Kept abstract so different backends (SQLite,
/// in-memory, future cloud) can map their errors uniformly.
enum class StoreError
{
    NotFound,
    Backend,        ///< underlying storage reported an error
    InvalidArgument,
};

/// Abstract persistence interface. The UI layer talks to this; the SQLite
/// implementation (CubingDB) and any in-memory store both implement it.
class ISolveStore
{
  public:
    virtual ~ISolveStore() = default;

    // --- profiles ---
    /// Creates a new profile.
    /// @param name display name.
    /// @return the created profile with `id` populated, or a StoreError.
    [[nodiscard]] virtual std::expected<Profile, StoreError> createProfile(std::string_view name) = 0;
    /// Lists all profiles in insertion order.
    /// @return the profiles, or a StoreError.
    [[nodiscard]] virtual std::expected<std::vector<Profile>, StoreError> listProfiles() = 0;
    /// Deletes a profile; cascades to sessions and solves.
    /// @param profileId id of the profile to delete.
    /// @return void on success; NotFound if no row matched.
    [[nodiscard]] virtual std::expected<void, StoreError> deleteProfile(std::int64_t profileId) = 0;

    // --- sessions ---
    /// Creates a session under a profile.
    /// @param profileId owning profile.
    /// @param name      display name.
    /// @param puzzle    puzzle this session records solves for.
    /// @return the created session with `id` populated, or a StoreError.
    [[nodiscard]] virtual std::expected<Session, StoreError> createSession(std::int64_t profileId,
                                                                            std::string_view name,
                                                                            Puzzle puzzle) = 0;
    /// Lists the sessions of a profile in insertion order.
    /// @param profileId owning profile.
    /// @return the sessions, or a StoreError.
    [[nodiscard]] virtual std::expected<std::vector<Session>, StoreError>
        listSessions(std::int64_t profileId) = 0;
    /// Deletes a session; cascades to its solves.
    /// @param sessionId id of the session to delete.
    /// @return void on success; NotFound if no row matched.
    [[nodiscard]] virtual std::expected<void, StoreError> deleteSession(std::int64_t sessionId) = 0;

    // --- solves ---
    /// Inserts a solve.
    /// @param solve solve to insert; its `id` is ignored.
    /// @return the solve with `id` populated, or a StoreError.
    [[nodiscard]] virtual std::expected<Solve, StoreError> addSolve(Solve solve) = 0;
    /// Updates an existing solve's mutable fields (penalty, comment, raw time, inspection).
    /// @param solve solve identified by `id`.
    /// @return void on success; NotFound if no row matched.
    [[nodiscard]] virtual std::expected<void, StoreError> updateSolve(Solve const& solve) = 0;
    /// Deletes a single solve.
    /// @param solveId id of the solve to delete.
    /// @return void on success; NotFound if no row matched.
    [[nodiscard]] virtual std::expected<void, StoreError> deleteSolve(std::int64_t solveId) = 0;
    /// Loads all solves of a session in chronological order.
    /// @param sessionId session whose solves to load.
    /// @return the solves, or a StoreError.
    [[nodiscard]] virtual std::expected<std::vector<Solve>, StoreError>
        loadSession(std::int64_t sessionId) = 0;
};

} // namespace CubingCore
