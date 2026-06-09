// SPDX-License-Identifier: Apache-2.0
#include <atomic>
#include <chrono>

#include <CubingDB/SqliteSolveStore.hpp>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>

using CubingCore::Penalty;
using CubingCore::Profile;
using CubingCore::Puzzle;
using CubingCore::Session;
using CubingCore::Solve;
using CubingCore::StoreError;
using CubingCore::Timestamp;
using Milliseconds = CubingCore::Milliseconds;

namespace CubingDB
{

namespace
{
    constexpr int SchemaVersion = 2;

    /// Returns a process-unique connection name so multiple stores can coexist
    /// in tests / multiple-instance scenarios.
    QString makeConnectionName()
    {
        static std::atomic<std::uint64_t> counter { 0 };
        return QStringLiteral("cubingdb_%1").arg(counter.fetch_add(1U, std::memory_order_relaxed));
    }

    std::int64_t toMillis(Timestamp ts) noexcept
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(ts.time_since_epoch()).count();
    }

    Timestamp fromMillis(std::int64_t ms) noexcept
    {
        return Timestamp { Milliseconds { ms } };
    }

    QString puzzleKey(Puzzle p)
    {
        return QString::fromUtf8(CubingCore::specOf(p).name.data(),
                                 static_cast<qsizetype>(CubingCore::specOf(p).name.size()));
    }

    Puzzle puzzleFromQString(QString const& s)
    {
        return CubingCore::puzzleFromKey(s.toStdString());
    }
} // namespace

struct SqliteSolveStore::Impl
{
    QSqlDatabase db;
    QString connectionName;

    explicit Impl(std::string const& path):
        connectionName(makeConnectionName())
    {
        db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), connectionName);
        db.setDatabaseName(QString::fromStdString(path));
    }

    Impl(Impl const&) = delete;
    Impl(Impl&&) = delete;
    Impl& operator=(Impl const&) = delete;
    Impl& operator=(Impl&&) = delete;

    ~Impl()
    {
        db.close();
        db = QSqlDatabase();
        QSqlDatabase::removeDatabase(connectionName);
    }

    [[nodiscard]] bool open()
    {
        if (!db.open())
            return false;
        QSqlQuery q(db);
        q.exec(QStringLiteral("PRAGMA foreign_keys = ON"));
        return migrate();
    }

    [[nodiscard]] int currentVersion() const
    {
        QSqlQuery q(db);
        if (!q.exec(QStringLiteral("PRAGMA user_version")))
            return -1;
        if (!q.next())
            return -1;
        return q.value(0).toInt();
    }

    [[nodiscard]] bool migrate() const
    {
        auto const version = currentVersion();
        if (version == SchemaVersion)
            return true;

        QSqlQuery q(db);
        if (!q.exec(QStringLiteral(R"(
            CREATE TABLE IF NOT EXISTS profile (
                id         INTEGER PRIMARY KEY AUTOINCREMENT,
                name       TEXT NOT NULL,
                created_at INTEGER NOT NULL,
                sort_order INTEGER NOT NULL DEFAULT 0
            )
        )")))
            return false;

        // v1 -> v2: add sort_order column if upgrading an existing database.
        // (CREATE TABLE IF NOT EXISTS above is a no-op on existing tables,
        // so the column needs to be added explicitly.)
        if (version >= 1 && version < 2)
        {
            QSqlQuery alter(db);
            // sqlite ignores duplicate-column errors here when upgrading from a
            // freshly-created v1 table that already had the column, but a real
            // v1 database needs the ALTER.
            (void) alter.exec(QStringLiteral("ALTER TABLE profile ADD COLUMN sort_order INTEGER NOT NULL DEFAULT 0"));
        }
        if (!q.exec(QStringLiteral(R"(
            CREATE TABLE IF NOT EXISTS session (
                id          INTEGER PRIMARY KEY AUTOINCREMENT,
                profile_id  INTEGER NOT NULL REFERENCES profile(id) ON DELETE CASCADE,
                name        TEXT NOT NULL,
                puzzle_type TEXT NOT NULL,
                created_at  INTEGER NOT NULL
            )
        )")))
            return false;
        if (!q.exec(QStringLiteral(R"(
            CREATE TABLE IF NOT EXISTS solve (
                id            INTEGER PRIMARY KEY AUTOINCREMENT,
                session_id    INTEGER NOT NULL REFERENCES session(id) ON DELETE CASCADE,
                puzzle_type   TEXT NOT NULL,
                timestamp_ms  INTEGER NOT NULL,
                raw_time_ms   INTEGER NOT NULL,
                penalty       INTEGER NOT NULL,
                scramble      TEXT NOT NULL,
                comment       TEXT NOT NULL DEFAULT '',
                inspection_ms INTEGER
            )
        )")))
            return false;
        if (!q.exec(QStringLiteral("CREATE INDEX IF NOT EXISTS solve_session_idx "
                                   "ON solve(session_id, timestamp_ms)")))
            return false;
        return q.exec(QStringLiteral("PRAGMA user_version = %1").arg(SchemaVersion));
    }
};

SqliteSolveStore::SqliteSolveStore(std::string const& path):
    _impl(std::make_unique<Impl>(path))
{
    if (!_impl->open())
        throw std::runtime_error("Failed to open SQLite database: " + _impl->db.lastError().text().toStdString());
}

SqliteSolveStore::~SqliteSolveStore() = default;

std::expected<Profile, StoreError> SqliteSolveStore::createProfile(std::string_view name)
{
    QSqlQuery q(_impl->db);
    q.prepare(QStringLiteral("INSERT INTO profile (name, created_at) VALUES (?, ?)"));
    auto const now = std::chrono::system_clock::now();
    q.addBindValue(QString::fromUtf8(name.data(), static_cast<qsizetype>(name.size())));
    q.addBindValue(QVariant::fromValue<qlonglong>(toMillis(now)));
    if (!q.exec())
        return std::unexpected(StoreError::Backend);

    Profile p;
    p.id = q.lastInsertId().toLongLong();
    p.name = std::string(name);
    p.createdAt = now;
    return p;
}

std::expected<std::vector<Profile>, StoreError> SqliteSolveStore::listProfiles()
{
    QSqlQuery q(_impl->db);
    if (!q.exec(QStringLiteral("SELECT id, name, created_at FROM profile ORDER BY sort_order, id")))
        return std::unexpected(StoreError::Backend);
    std::vector<Profile> out;
    while (q.next())
    {
        Profile p;
        p.id = q.value(0).toLongLong();
        p.name = q.value(1).toString().toStdString();
        p.createdAt = fromMillis(q.value(2).toLongLong());
        out.push_back(std::move(p));
    }
    return out;
}

std::expected<void, StoreError> SqliteSolveStore::deleteProfile(std::int64_t profileId)
{
    QSqlQuery q(_impl->db);
    q.prepare(QStringLiteral("DELETE FROM profile WHERE id = ?"));
    q.addBindValue(QVariant::fromValue<qlonglong>(profileId));
    if (!q.exec())
        return std::unexpected(StoreError::Backend);
    if (q.numRowsAffected() == 0)
        return std::unexpected(StoreError::NotFound);
    return {};
}

std::expected<void, StoreError> SqliteSolveStore::renameProfile(std::int64_t profileId, std::string_view newName)
{
    if (newName.empty())
        return std::unexpected(StoreError::InvalidArgument);
    QSqlQuery q(_impl->db);
    q.prepare(QStringLiteral("UPDATE profile SET name = ? WHERE id = ?"));
    q.addBindValue(QString::fromUtf8(newName.data(), static_cast<qsizetype>(newName.size())));
    q.addBindValue(QVariant::fromValue<qlonglong>(profileId));
    if (!q.exec())
        return std::unexpected(StoreError::Backend);
    if (q.numRowsAffected() == 0)
        return std::unexpected(StoreError::NotFound);
    return {};
}

std::expected<void, StoreError> SqliteSolveStore::reorderProfiles(std::span<std::int64_t const> orderedIds)
{
    if (!_impl->db.transaction())
        return std::unexpected(StoreError::Backend);
    QSqlQuery q(_impl->db);
    q.prepare(QStringLiteral("UPDATE profile SET sort_order = ? WHERE id = ?"));
    int order = 0;
    for (auto const id: orderedIds)
    {
        q.addBindValue(order++);
        q.addBindValue(QVariant::fromValue<qlonglong>(id));
        if (!q.exec())
        {
            (void) _impl->db.rollback();
            return std::unexpected(StoreError::Backend);
        }
    }
    if (!_impl->db.commit())
        return std::unexpected(StoreError::Backend);
    return {};
}

std::expected<Session, StoreError> SqliteSolveStore::createSession(std::int64_t profileId,
                                                                   std::string_view name,
                                                                   Puzzle puzzle)
{
    QSqlQuery q(_impl->db);
    q.prepare(QStringLiteral("INSERT INTO session (profile_id, name, puzzle_type, created_at) "
                             "VALUES (?, ?, ?, ?)"));
    auto const now = std::chrono::system_clock::now();
    q.addBindValue(QVariant::fromValue<qlonglong>(profileId));
    q.addBindValue(QString::fromUtf8(name.data(), static_cast<qsizetype>(name.size())));
    q.addBindValue(puzzleKey(puzzle));
    q.addBindValue(QVariant::fromValue<qlonglong>(toMillis(now)));
    if (!q.exec())
        return std::unexpected(StoreError::Backend);
    Session s;
    s.id = q.lastInsertId().toLongLong();
    s.profileId = profileId;
    s.name = std::string(name);
    s.puzzle = puzzle;
    s.createdAt = now;
    return s;
}

std::expected<std::vector<Session>, StoreError> SqliteSolveStore::listSessions(std::int64_t profileId)
{
    QSqlQuery q(_impl->db);
    q.prepare(QStringLiteral("SELECT id, profile_id, name, puzzle_type, created_at "
                             "FROM session WHERE profile_id = ? ORDER BY id"));
    q.addBindValue(QVariant::fromValue<qlonglong>(profileId));
    if (!q.exec())
        return std::unexpected(StoreError::Backend);
    std::vector<Session> out;
    while (q.next())
    {
        Session s;
        s.id = q.value(0).toLongLong();
        s.profileId = q.value(1).toLongLong();
        s.name = q.value(2).toString().toStdString();
        s.puzzle = puzzleFromQString(q.value(3).toString());
        s.createdAt = fromMillis(q.value(4).toLongLong());
        out.push_back(std::move(s));
    }
    return out;
}

std::expected<void, StoreError> SqliteSolveStore::deleteSession(std::int64_t sessionId)
{
    QSqlQuery q(_impl->db);
    q.prepare(QStringLiteral("DELETE FROM session WHERE id = ?"));
    q.addBindValue(QVariant::fromValue<qlonglong>(sessionId));
    if (!q.exec())
        return std::unexpected(StoreError::Backend);
    if (q.numRowsAffected() == 0)
        return std::unexpected(StoreError::NotFound);
    return {};
}

std::expected<Solve, StoreError> SqliteSolveStore::addSolve(Solve solve)
{
    QSqlQuery q(_impl->db);
    q.prepare(QStringLiteral("INSERT INTO solve (session_id, puzzle_type, timestamp_ms, raw_time_ms, penalty, scramble, "
                             "comment, inspection_ms) VALUES (?, ?, ?, ?, ?, ?, ?, ?)"));
    q.addBindValue(QVariant::fromValue<qlonglong>(solve.sessionId));
    q.addBindValue(puzzleKey(solve.puzzle));
    q.addBindValue(QVariant::fromValue<qlonglong>(toMillis(solve.timestamp)));
    q.addBindValue(QVariant::fromValue<qlonglong>(solve.rawTime.count()));
    q.addBindValue(static_cast<int>(solve.penalty));
    q.addBindValue(QString::fromStdString(solve.scramble));
    q.addBindValue(QString::fromStdString(solve.comment));
    if (solve.inspection)
        q.addBindValue(QVariant::fromValue<qlonglong>(solve.inspection->count()));
    else
        q.addBindValue(QVariant(QMetaType(QMetaType::LongLong)));
    if (!q.exec())
        return std::unexpected(StoreError::Backend);
    solve.id = q.lastInsertId().toLongLong();
    return solve;
}

std::expected<void, StoreError> SqliteSolveStore::updateSolve(Solve const& solve)
{
    QSqlQuery q(_impl->db);
    q.prepare(QStringLiteral("UPDATE solve SET raw_time_ms = ?, penalty = ?, scramble = ?, comment = ?, "
                             "inspection_ms = ? WHERE id = ?"));
    q.addBindValue(QVariant::fromValue<qlonglong>(solve.rawTime.count()));
    q.addBindValue(static_cast<int>(solve.penalty));
    q.addBindValue(QString::fromStdString(solve.scramble));
    q.addBindValue(QString::fromStdString(solve.comment));
    if (solve.inspection)
        q.addBindValue(QVariant::fromValue<qlonglong>(solve.inspection->count()));
    else
        q.addBindValue(QVariant(QMetaType(QMetaType::LongLong)));
    q.addBindValue(QVariant::fromValue<qlonglong>(solve.id));
    if (!q.exec())
        return std::unexpected(StoreError::Backend);
    if (q.numRowsAffected() == 0)
        return std::unexpected(StoreError::NotFound);
    return {};
}

std::expected<void, StoreError> SqliteSolveStore::deleteSolve(std::int64_t solveId)
{
    QSqlQuery q(_impl->db);
    q.prepare(QStringLiteral("DELETE FROM solve WHERE id = ?"));
    q.addBindValue(QVariant::fromValue<qlonglong>(solveId));
    if (!q.exec())
        return std::unexpected(StoreError::Backend);
    if (q.numRowsAffected() == 0)
        return std::unexpected(StoreError::NotFound);
    return {};
}

std::expected<std::vector<Solve>, StoreError> SqliteSolveStore::loadSession(std::int64_t sessionId)
{
    QSqlQuery q(_impl->db);
    q.prepare(QStringLiteral("SELECT id, session_id, puzzle_type, timestamp_ms, raw_time_ms, penalty, scramble, comment, "
                             "inspection_ms FROM solve WHERE session_id = ? ORDER BY timestamp_ms, id"));
    q.addBindValue(QVariant::fromValue<qlonglong>(sessionId));
    if (!q.exec())
        return std::unexpected(StoreError::Backend);
    std::vector<Solve> out;
    while (q.next())
    {
        Solve s;
        s.id = q.value(0).toLongLong();
        s.sessionId = q.value(1).toLongLong();
        s.puzzle = puzzleFromQString(q.value(2).toString());
        s.timestamp = fromMillis(q.value(3).toLongLong());
        s.rawTime = Milliseconds { q.value(4).toLongLong() };
        s.penalty = static_cast<Penalty>(q.value(5).toInt());
        s.scramble = q.value(6).toString().toStdString();
        s.comment = q.value(7).toString().toStdString();
        if (!q.value(8).isNull())
            s.inspection = Milliseconds { q.value(8).toLongLong() };
        out.push_back(std::move(s));
    }
    return out;
}

} // namespace CubingDB
