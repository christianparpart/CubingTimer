// SPDX-License-Identifier: Apache-2.0
#include <CubingTimer/ScrambleProvider.h>

#include <CubingCore/Scrambler.h>

#include <chrono>
#include <random>

namespace CubingTimer
{

namespace
{
    std::uint32_t seedFromClock()
    {
        std::random_device rd;
        return static_cast<std::uint32_t>(rd()) ^ static_cast<std::uint32_t>(
                   std::chrono::steady_clock::now().time_since_epoch().count());
    }
} // namespace

ScrambleProvider::ScrambleProvider(QObject* parent): QObject(parent), _rng(seedFromClock())
{
    next();
}

QString ScrambleProvider::puzzle() const
{
    return QString::fromUtf8(CubingCore::specOf(_puzzle).name.data(),
                             static_cast<qsizetype>(CubingCore::specOf(_puzzle).name.size()));
}

void ScrambleProvider::setPuzzle(QString const& key)
{
    auto const next = CubingCore::puzzleFromKey(key.toStdString());
    if (next == _puzzle)
        return;
    _puzzle = next;
    emit puzzleChanged();
    this->next();
}

void ScrambleProvider::next()
{
    CubingCore::Scrambler s(CubingCore::specOf(_puzzle),
                            [this]() { return _rng(); });
    _current = QString::fromStdString(s.generateString());
    emit currentChanged();
}

} // namespace CubingTimer
