// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <CubingTimer/SessionModel.hpp>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtQml/QQmlEngine>

namespace CubingTimer
{

/// Computes statistics over a SessionModel's solves and exposes them as
/// Q_PROPERTYs. Recomputes lazily on solvesChanged.
class StatsModel: public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(SessionModel* source READ source WRITE setSource NOTIFY sourceChanged)

    // All-time
    Q_PROPERTY(QString best READ best NOTIFY changed)
    Q_PROPERTY(QString currentMo3 READ currentMo3 NOTIFY changed)
    Q_PROPERTY(QString currentAo5 READ currentAo5 NOTIFY changed)
    Q_PROPERTY(QString currentAo12 READ currentAo12 NOTIFY changed)
    Q_PROPERTY(QString currentAo100 READ currentAo100 NOTIFY changed)
    Q_PROPERTY(QString bestMo3 READ bestMo3 NOTIFY changed)
    Q_PROPERTY(QString bestAo5 READ bestAo5 NOTIFY changed)
    Q_PROPERTY(QString bestAo12 READ bestAo12 NOTIFY changed)
    Q_PROPERTY(QString bestAo100 READ bestAo100 NOTIFY changed)

    // Last 90 days
    Q_PROPERTY(QString bestLast90Days READ bestLast90Days NOTIFY changed)
    Q_PROPERTY(QString bestAo5Last90Days READ bestAo5Last90Days NOTIFY changed)
    Q_PROPERTY(QString bestAo12Last90Days READ bestAo12Last90Days NOTIFY changed)

  public:
    explicit StatsModel(QObject* parent = nullptr);

    [[nodiscard]] SessionModel* source() const noexcept
    {
        return _source;
    }
    void setSource(SessionModel* source);

    [[nodiscard]] QString best() const;
    [[nodiscard]] QString currentMo3() const;
    [[nodiscard]] QString currentAo5() const;
    [[nodiscard]] QString currentAo12() const;
    [[nodiscard]] QString currentAo100() const;
    [[nodiscard]] QString bestMo3() const;
    [[nodiscard]] QString bestAo5() const;
    [[nodiscard]] QString bestAo12() const;
    [[nodiscard]] QString bestAo100() const;

    [[nodiscard]] QString bestLast90Days() const;
    [[nodiscard]] QString bestAo5Last90Days() const;
    [[nodiscard]] QString bestAo12Last90Days() const;

    /// Formats a milliseconds value as "ss.cc" or "m:ss.cc" — invokable from QML.
    /// @param ms duration in milliseconds.
    /// @return human-readable centisecond-precision string.
    Q_INVOKABLE static QString formatMs(qlonglong ms);

  signals:
    void sourceChanged();
    void changed();

  private:
    SessionModel* _source = nullptr;
};

} // namespace CubingTimer
