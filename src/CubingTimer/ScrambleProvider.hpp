// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <random>

#include <CubingCore/Puzzle.hpp>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtQml/QQmlEngine>

namespace CubingTimer
{

/// QML-facing wrapper around CubingCore::Scrambler.
class ScrambleProvider: public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QString puzzle READ puzzle WRITE setPuzzle NOTIFY puzzleChanged)
    Q_PROPERTY(QString current READ current NOTIFY currentChanged)

  public:
    explicit ScrambleProvider(QObject* parent = nullptr);

    [[nodiscard]] QString puzzle() const;
    void setPuzzle(QString const& key);

    [[nodiscard]] QString current() const
    {
        return _current;
    }

  public slots:
    /// Generates a new scramble for the active puzzle and updates `current`.
    void next();

  signals:
    void puzzleChanged();
    void currentChanged();

  private:
    CubingCore::Puzzle _puzzle = CubingCore::Puzzle::ThreeByThree;
    QString _current;
    std::mt19937 _rng;
};

} // namespace CubingTimer
