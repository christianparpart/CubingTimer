// SPDX-License-Identifier: Apache-2.0
#include <CubingTimer/SessionModel.h>

#include <chrono>

using CubingCore::Penalty;
using CubingCore::Puzzle;
using CubingCore::Solve;

namespace CubingTimer
{

SessionModel::SessionModel(QObject* parent): QAbstractListModel(parent) {}

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
    switch (role)
    {
        case TimestampMsRole:
            return QVariant::fromValue<qlonglong>(
                std::chrono::duration_cast<std::chrono::milliseconds>(s.timestamp.time_since_epoch())
                    .count());
        case RawTimeMsRole: return QVariant::fromValue<qlonglong>(s.rawTime.count());
        case EffectiveTimeMsRole:
        {
            auto const t = s.effectiveTime();
            return t ? QVariant::fromValue<qlonglong>(t->count())
                     : QVariant::fromValue<qlonglong>(-1);
        }
        case PenaltyRole: return static_cast<int>(s.penalty);
        case ScrambleRole: return QString::fromStdString(s.scramble);
        case CommentRole: return QString::fromStdString(s.comment);
        case IdRole: return QVariant::fromValue<qlonglong>(s.id);
        default: return {};
    }
}

QHash<int, QByteArray> SessionModel::roleNames() const
{
    return {
        { TimestampMsRole, "timestampMs" },
        { RawTimeMsRole, "rawTimeMs" },
        { EffectiveTimeMsRole, "effectiveTimeMs" },
        { PenaltyRole, "penalty" },
        { ScrambleRole, "scramble" },
        { CommentRole, "comment" },
        { IdRole, "solveId" },
    };
}

void SessionModel::addSolve(qint64 rawMs,
                            int penalty,
                            QString const& scramble,
                            qint64 inspectionMs,
                            QString const& puzzleKey)
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
    emit dataChanged(idx, idx, { PenaltyRole, EffectiveTimeMsRole });
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
