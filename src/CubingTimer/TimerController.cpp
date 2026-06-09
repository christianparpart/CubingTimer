// SPDX-License-Identifier: Apache-2.0
#include <CubingTimer/TimerController.hpp>

namespace CubingTimer
{

namespace
{
    constexpr int InspectionLimitMs = 15'000;
    constexpr int InspectionPlusTwoMs = 17'000;
    constexpr int UiTickIntervalMs = 16; // ~60Hz
} // namespace

TimerController::TimerController(QObject* parent):
    QObject(parent)
{
    _holdArmTimer.setSingleShot(true);
    connect(&_holdArmTimer, &QTimer::timeout, this, &TimerController::onHoldTimerTimeout);

    _uiTickTimer.setInterval(UiTickIntervalMs);
    connect(&_uiTickTimer, &QTimer::timeout, this, &TimerController::tick);
}

qint64 TimerController::elapsedMs() const noexcept
{
    switch (_state)
    {
        case State::Running:
            return _solveTimer.isValid() ? _solveTimer.elapsed() : 0;
        case State::Stopped:
            return _lastSolveMs;
        default:
            return 0;
    }
}

int TimerController::inspectionElapsedMs() const noexcept
{
    switch (_state)
    {
        case State::InspectionRunning:
        case State::HoldPending:
        case State::Armed:
            return _inspectionTimer.isValid() ? static_cast<int>(_inspectionTimer.elapsed()) : 0;
        case State::Running:
        case State::Stopped:
            return static_cast<int>(_lastInspectionMs);
        default:
            return 0;
    }
}

void TimerController::setHoldMs(int ms)
{
    if (ms == _holdMs)
        return;
    _holdMs = ms;
    emit holdMsChanged();
}

void TimerController::setInspectionEnabled(bool enabled)
{
    if (enabled == _inspectionEnabled)
        return;
    _inspectionEnabled = enabled;
    emit inspectionEnabledChanged();
}

void TimerController::enter(State next)
{
    if (_state == next)
        return;
    _state = next;
    emit stateChanged();
}

void TimerController::holdBegin()
{
    switch (_state)
    {
        case State::Idle:
            _pendingPenalty = CubingCore::Penalty::Ok;
            emit pendingPenaltyChanged();
            if (_inspectionEnabled)
                enter(State::InspectionPending);
            else
            {
                _holdTimer.restart();
                _holdArmTimer.start(_holdMs);
                enter(State::HoldPending);
            }
            break;
        case State::InspectionRunning:
            _holdTimer.restart();
            _holdArmTimer.start(_holdMs);
            enter(State::HoldPending);
            break;
        case State::Stopped:
            // Clicking again after a stop is a no-op until Reset() / acknowledge is called.
            break;
        default:
            break;
    }
}

void TimerController::holdEnd()
{
    switch (_state)
    {
        case State::InspectionPending:
            _inspectionTimer.restart();
            _uiTickTimer.start();
            enter(State::InspectionRunning);
            break;
        case State::HoldPending:
            // Released too soon — cancel and go back to where we came from.
            _holdArmTimer.stop();
            if (_inspectionEnabled && _inspectionTimer.isValid())
                enter(State::InspectionRunning);
            else
                enter(State::Idle);
            break;
        case State::Armed:
            _solveTimer.restart();
            _uiTickTimer.start();
            enter(State::Running);
            break;
        case State::Running:
            // Treat release as stop too (for touch UIs that send a single event).
            stop();
            break;
        default:
            break;
    }
}

void TimerController::stop()
{
    if (_state != State::Running)
        return;
    _lastSolveMs = _solveTimer.elapsed();
    _uiTickTimer.stop();

    // Decide inspection penalty.
    if (_inspectionEnabled && _inspectionTimer.isValid())
    {
        _lastInspectionMs = _inspectionTimer.elapsed();
        if (_lastInspectionMs > InspectionPlusTwoMs)
            _pendingPenalty = CubingCore::Penalty::Dnf;
        else if (_lastInspectionMs > InspectionLimitMs)
            _pendingPenalty = CubingCore::Penalty::PlusTwo;
        emit pendingPenaltyChanged();
    }

    enter(State::Stopped);
    emit elapsedChanged();
    emit solveFinished(_lastSolveMs, static_cast<int>(_pendingPenalty), _inspectionEnabled ? _lastInspectionMs : 0);
}

void TimerController::reset()
{
    _holdArmTimer.stop();
    _uiTickTimer.stop();
    _inspectionTimer.invalidate();
    _solveTimer.invalidate();
    _lastSolveMs = 0;
    _lastInspectionMs = 0;
    _pendingPenalty = CubingCore::Penalty::Ok;
    emit pendingPenaltyChanged();
    emit elapsedChanged();
    enter(State::Idle);
}

void TimerController::tick()
{
    emit elapsedChanged();
}

void TimerController::onHoldTimerTimeout()
{
    if (_state == State::HoldPending)
        enter(State::Armed);
}

} // namespace CubingTimer
