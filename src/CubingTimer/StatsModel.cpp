// SPDX-License-Identifier: Apache-2.0
#include <chrono>

#include <CubingCore/Stats.hpp>
#include <CubingTimer/StatsModel.hpp>

using CubingCore::Milliseconds;
using CubingCore::Solve;
namespace stats = CubingCore::stats;

namespace CubingTimer
{

namespace
{
    QString fmtDuration(Milliseconds ms)
    {
        return StatsModel::formatMs(static_cast<qlonglong>(ms.count()));
    }

    QString fmtSingle(std::expected<Milliseconds, stats::StatsError> v)
    {
        if (!v)
            return QStringLiteral("—");
        return fmtDuration(*v);
    }

    QString fmtAvg(std::expected<stats::AverageResult, stats::StatsError> v)
    {
        if (!v)
            return QStringLiteral("—");
        if (v->isDnf() || !v->value.has_value())
            return QStringLiteral("DNF");
        return fmtDuration(*v->value);
    }
} // namespace

StatsModel::StatsModel(QObject* parent):
    QObject(parent)
{
}

void StatsModel::setSource(SessionModel* source)
{
    if (source == _source)
        return;
    if (_source)
        disconnect(_source, &SessionModel::solvesChanged, this, &StatsModel::changed);
    _source = source;
    if (_source)
        connect(_source, &SessionModel::solvesChanged, this, &StatsModel::changed);
    emit sourceChanged();
    emit changed();
}

QString StatsModel::best() const
{
    if (!_source)
        return QStringLiteral("—");
    return fmtSingle(stats::best(_source->solves()));
}

QString StatsModel::currentMo3() const
{
    if (!_source)
        return QStringLiteral("—");
    return fmtAvg(stats::meanOfLast(_source->solves(), 3));
}

QString StatsModel::currentAo5() const
{
    if (!_source)
        return QStringLiteral("—");
    return fmtAvg(stats::averageOfLast(_source->solves(), 5));
}

QString StatsModel::currentAo12() const
{
    if (!_source)
        return QStringLiteral("—");
    return fmtAvg(stats::averageOfLast(_source->solves(), 12));
}

QString StatsModel::currentAo100() const
{
    if (!_source)
        return QStringLiteral("—");
    return fmtAvg(stats::averageOfLast(_source->solves(), 100));
}

QString StatsModel::bestMo3() const
{
    if (!_source)
        return QStringLiteral("—");
    return fmtAvg(stats::bestMeanOf(_source->solves(), 3));
}

QString StatsModel::bestAo5() const
{
    if (!_source)
        return QStringLiteral("—");
    return fmtAvg(stats::bestAverageOf(_source->solves(), 5));
}

QString StatsModel::bestAo12() const
{
    if (!_source)
        return QStringLiteral("—");
    return fmtAvg(stats::bestAverageOf(_source->solves(), 12));
}

QString StatsModel::bestAo100() const
{
    if (!_source)
        return QStringLiteral("—");
    return fmtAvg(stats::bestAverageOf(_source->solves(), 100));
}

QString StatsModel::bestLast90Days() const
{
    if (!_source)
        return QStringLiteral("—");
    auto const filtered = stats::filterLastDays(_source->solves(), std::chrono::system_clock::now(), 90);
    return fmtSingle(stats::best(filtered));
}

QString StatsModel::bestAo5Last90Days() const
{
    if (!_source)
        return QStringLiteral("—");
    auto const filtered = stats::filterLastDays(_source->solves(), std::chrono::system_clock::now(), 90);
    return fmtAvg(stats::bestAverageOf(filtered, 5));
}

QString StatsModel::bestAo12Last90Days() const
{
    if (!_source)
        return QStringLiteral("—");
    auto const filtered = stats::filterLastDays(_source->solves(), std::chrono::system_clock::now(), 90);
    return fmtAvg(stats::bestAverageOf(filtered, 12));
}

QString StatsModel::formatMs(qlonglong ms)
{
    auto const totalCs = (ms + 5) / 10; // round to centiseconds
    auto const seconds = totalCs / 100;
    auto const centis = totalCs % 100;
    if (seconds < 60)
        return QString::asprintf("%lld.%02lld", seconds, centis);
    auto const minutes = seconds / 60;
    auto const rem = seconds % 60;
    return QString::asprintf("%lld:%02lld.%02lld", minutes, rem, centis);
}

} // namespace CubingTimer
