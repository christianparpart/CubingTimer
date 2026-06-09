// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <memory>
#include <string>

#include <CubingCore/ISolveStore.hpp>
#include <QtSql/QSqlDatabase>

namespace CubingDB
{

/// SQLite-backed solve store. Holds its own QSqlDatabase connection (uniquely
/// named) and applies a versioned schema on construction.
class SqliteSolveStore: public CubingCore::ISolveStore
{
  public:
    /// Opens (creating if necessary) the SQLite database at the given path.
    /// Use ":memory:" for an in-memory database (used by WASM and by tests).
    explicit SqliteSolveStore(std::string const& path);
    ~SqliteSolveStore() override;

    SqliteSolveStore(SqliteSolveStore const&) = delete;
    SqliteSolveStore(SqliteSolveStore&&) = delete;
    SqliteSolveStore& operator=(SqliteSolveStore const&) = delete;
    SqliteSolveStore& operator=(SqliteSolveStore&&) = delete;

    // ISolveStore implementation
    [[nodiscard]] std::expected<CubingCore::Profile, CubingCore::StoreError> createProfile(std::string_view name) override;
    [[nodiscard]] std::expected<std::vector<CubingCore::Profile>, CubingCore::StoreError> listProfiles() override;
    [[nodiscard]] std::expected<void, CubingCore::StoreError> deleteProfile(std::int64_t profileId) override;
    [[nodiscard]] std::expected<void, CubingCore::StoreError> renameProfile(std::int64_t profileId,
                                                                            std::string_view newName) override;
    [[nodiscard]] std::expected<void, CubingCore::StoreError> reorderProfiles(
        std::span<std::int64_t const> orderedIds) override;

    [[nodiscard]] std::expected<CubingCore::Session, CubingCore::StoreError> createSession(
        std::int64_t profileId, std::string_view name, CubingCore::Puzzle puzzle) override;
    [[nodiscard]] std::expected<std::vector<CubingCore::Session>, CubingCore::StoreError> listSessions(
        std::int64_t profileId) override;
    [[nodiscard]] std::expected<void, CubingCore::StoreError> deleteSession(std::int64_t sessionId) override;

    [[nodiscard]] std::expected<CubingCore::Solve, CubingCore::StoreError> addSolve(CubingCore::Solve solve) override;
    [[nodiscard]] std::expected<void, CubingCore::StoreError> updateSolve(CubingCore::Solve const& solve) override;
    [[nodiscard]] std::expected<void, CubingCore::StoreError> deleteSolve(std::int64_t solveId) override;
    [[nodiscard]] std::expected<std::vector<CubingCore::Solve>, CubingCore::StoreError> loadSession(
        std::int64_t sessionId) override;

  private:
    struct Impl;
    std::unique_ptr<Impl> _impl;
};

} // namespace CubingDB
