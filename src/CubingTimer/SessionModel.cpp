// SPDX-License-Identifier: Apache-2.0
#include <chrono>

#include <CubingTimer/SessionModel.hpp>

using CubingCore::Penalty;
using CubingCore::Puzzle;
using CubingCore::Solve;

namespace CubingTimer
{

SessionModel::SessionModel(QObject* parent):
    QAbstractListModel(parent)
{
}

void SessionModel::setStore(CubingCore::ISolveStore* store)
{
    _store = store;
    if (_sessionId != 0)
        reload();
}

void SessionModel::setSessionId(qint64 sessionId)
{
    if (sessionId == _sessionId)
        return;
    _sessionId = sessionId;
    emit sessionIdChanged();
    reload();
}

int SessionModel::lastPenalty() const noexcept
{
    if (_solves.empty())
        return static_cast<int>(Penalty::Ok);
    return static_cast<int>(_solves.back().penalty);
}

qint64 SessionModel::lastEffectiveTimeMs() const noexcept
{
    if (_solves.empty())
        return -1;
    auto const t = _solves.back().effectiveTime();
    return t ? t->count() : -1;
}

void SessionModel::reload()
{
    beginResetModel();
    _solves.clear();
    if (_store && _sessionId != 0)
    {
        auto const loaded = _store->loadSession(_sessionId);
        if (loaded)
            _solves = *loaded;
    }
    endResetModel();
    emit countChanged();
    emit solvesChanged();
}

int SessionModel::rowCount(QModelIndex const& parent) const
{
    if (parent.isValid())
        return 0;
    return static_cast<int>(_solves.size());
}

QVariant SessionModel::data(QModelIndex const& index, int role) const
{
    auto const row = static_cast<std::size_t>(index.row());
    if (row >= _solves.size())
        return {};
    auto const& s = _solves[row];
    switch (static_cast<Roles>(role))
    {
        case Roles::TimestampMsRole:
            return QVariant::fromValue<qlonglong>(
                std::chrono::duration_cast<std::chrono::milliseconds>(s.timestamp.time_since_epoch()).count());
        case Roles::RawTimeMsRole:
            return QVariant::fromValue<qlonglong>(s.rawTime.count());
        case Roles::EffectiveTimeMsRole: {
            auto const t = s.effectiveTime();
            return t ? QVariant::fromValue<qlonglong>(t->count()) : QVariant::fromValue<qlonglong>(-1);
        }
        case Roles::PenaltyRole:
            return static_cast<int>(s.penalty);
        case Roles::ScrambleRole:
            return QString::fromStdString(s.scramble);
        case Roles::CommentRole:
            return QString::fromStdString(s.comment);
        case Roles::IdRole:
            return QVariant::fromValue<qlonglong>(s.id);
    }
    return {};
}

QHash<int, QByteArray> SessionModel::roleNames() const
{
    return {
        { static_cast<int>(Roles::TimestampMsRole), "timestampMs" },
        { static_cast<int>(Roles::RawTimeMsRole), "rawTimeMs" },
        { static_cast<int>(Roles::EffectiveTimeMsRole), "effectiveTimeMs" },
        { static_cast<int>(Roles::PenaltyRole), "penalty" },
        { static_cast<int>(Roles::ScrambleRole), "scramble" },
        { static_cast<int>(Roles::CommentRole), "comment" },
        { static_cast<int>(Roles::IdRole), "solveId" },
    };
}

void SessionModel::addSolve(
    qint64 rawMs, int penalty, QString const& scramble, qint64 inspectionMs, QString const& puzzleKey)
{
    if (!_store || _sessionId == 0)
        return;

    Solve s;
    s.sessionId = _sessionId;
    s.puzzle = CubingCore::puzzleFromKey(puzzleKey.toStdString());
    s.timestamp = std::chrono::system_clock::now();
    s.rawTime = std::chrono::milliseconds { rawMs };
    s.penalty = static_cast<Penalty>(penalty);
    s.scramble = scramble.toStdString();
    if (inspectionMs > 0)
        s.inspection = std::chrono::milliseconds { inspectionMs };

    auto const added = _store->addSolve(std::move(s));
    if (!added)
        return;
    auto const row = static_cast<int>(_solves.size());
    beginInsertRows({}, row, row);
    _solves.push_back(*added);
    endInsertRows();
    emit countChanged();
    emit solvesChanged();
}

void SessionModel::setLastPenalty(int penalty)
{
    if (_solves.empty() || !_store)
        return;
    auto& last = _solves.back();
    last.penalty = static_cast<Penalty>(penalty);
    (void) _store->updateSolve(last);
    auto const row = static_cast<int>(_solves.size() - 1);
    auto const idx = index(row);
    emit dataChanged(idx, idx, { static_cast<int>(Roles::PenaltyRole), static_cast<int>(Roles::EffectiveTimeMsRole) });
    emit solvesChanged();
}

void SessionModel::removeLast()
{
    if (_solves.empty() || !_store)
        return;
    auto const id = _solves.back().id;
    (void) _store->deleteSolve(id);
    auto const row = static_cast<int>(_solves.size() - 1);
    beginRemoveRows({}, row, row);
    _solves.pop_back();
    endRemoveRows();
    emit countChanged();
    emit solvesChanged();
}

} // namespace CubingTimer
