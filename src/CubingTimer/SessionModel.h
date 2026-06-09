// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <CubingCore/ISolveStore.h>
#include <CubingCore/Solve.h>

#include <QtCore/QAbstractListModel>
#include <QtQml/QQmlEngine>

#include <vector>

namespace CubingTimer
{

/// Exposes the solves of one session to QML as a list model. Holds the loaded
/// solves in memory and keeps the underlying store in sync as the user
/// adds / edits / deletes solves.
class SessionModel: public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(qint64 sessionId READ sessionId WRITE setSessionId NOTIFY sessionIdChanged)
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

  public:
    enum Roles
    {
        TimestampMsRole = Qt::UserRole + 1,
        RawTimeMsRole,
        EffectiveTimeMsRole, // qint64 or -1 for DNF
        PenaltyRole,         // int (0 OK, 1 +2, 2 DNF)
        ScrambleRole,
        CommentRole,
        IdRole,
    };

    explicit SessionModel(QObject* parent = nullptr);

    /// Injects the store. Must be set before any session is loaded.
    void setStore(CubingCore::ISolveStore* store);

    [[nodiscard]] qint64 sessionId() const noexcept { return _sessionId; }
    void setSessionId(qint64 sessionId);

    // QAbstractListModel
    [[nodiscard]] int rowCount(QModelIndex const& parent = {}) const override;
    [[nodiscard]] QVariant data(QModelIndex const& index, int role) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    /// Returns a copy of the loaded solves — used by StatsModel for computations.
    [[nodiscard]] std::vector<CubingCore::Solve> const& solves() const noexcept { return _solves; }

  public slots:
    /// Records a new solve for the currently active session.
    /// @param rawMs        unpenalised solve duration in milliseconds.
    /// @param penalty      one of CubingCore::Penalty (cast to int).
    /// @param scramble     WCA-notation scramble that was solved.
    /// @param inspectionMs inspection time used, or 0 if disabled.
    /// @param puzzleKey    "222" / "333" / "444".
    void addSolve(qint64 rawMs, int penalty, QString const& scramble, qint64 inspectionMs, QString const& puzzleKey);
    /// Sets the penalty of the most recent solve.
    /// @param penalty new penalty value cast to int.
    void setLastPenalty(int penalty);
    /// Removes the most recent solve.
    void removeLast();

  signals:
    void sessionIdChanged();
    void countChanged();
    void solvesChanged();

  private:
    void reload();

    CubingCore::ISolveStore* _store = nullptr;
    qint64 _sessionId = 0;
    std::vector<CubingCore::Solve> _solves;
};

} // namespace CubingTimer
