// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <cstdint>

#include <CubingCore/Solve.hpp>
#include <QtCore/QElapsedTimer>
#include <QtCore/QObject>
#include <QtCore/QTimer>
#include <QtQml/QQmlEngine>

namespace CubingTimer
{

/// State machine for the WCA-style timing protocol.
///
/// Lifecycle (no inspection):
///   Idle → HoldPending (key down) → Armed (held ≥ holdMs) → Running (key release) → Stopped
///
/// Lifecycle (inspection on):
///   Idle → InspectionPending → InspectionRunning (15s ticking) → HoldPending → Armed → Running → Stopped
///
/// Input is keyed by holdBegin / holdEnd / stop slots, so the same FSM drives
/// keyboard (spacebar) on desktop and touch on mobile.
class TimerController: public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(State state READ state NOTIFY stateChanged)
    Q_PROPERTY(qint64 elapsedMs READ elapsedMs NOTIFY elapsedChanged)
    Q_PROPERTY(int holdMs READ holdMs WRITE setHoldMs NOTIFY holdMsChanged)
    Q_PROPERTY(bool inspectionEnabled READ inspectionEnabled WRITE setInspectionEnabled NOTIFY inspectionEnabledChanged)
    Q_PROPERTY(int inspectionElapsedMs READ inspectionElapsedMs NOTIFY elapsedChanged)
    Q_PROPERTY(CubingCore::Penalty pendingPenalty READ pendingPenalty NOTIFY pendingPenaltyChanged)

  public:
    enum class State : std::uint8_t
    {
        Idle,
        InspectionPending, ///< user has tapped to start inspection but not yet released
        InspectionRunning, ///< 15s WCA inspection is ticking
        HoldPending,       ///< spacebar pressed, not yet held long enough
        Armed,             ///< held long enough; release will start the run
        Running,           ///< timer is running
        Stopped,           ///< just finished a solve; awaits acknowledgement
    };
    Q_ENUM(State)

    explicit TimerController(QObject* parent = nullptr);

    [[nodiscard]] State state() const noexcept
    {
        return _state;
    }
    [[nodiscard]] qint64 elapsedMs() const noexcept;
    [[nodiscard]] int holdMs() const noexcept
    {
        return _holdMs;
    }
    void setHoldMs(int ms);
    [[nodiscard]] bool inspectionEnabled() const noexcept
    {
        return _inspectionEnabled;
    }
    void setInspectionEnabled(bool enabled);
    [[nodiscard]] int inspectionElapsedMs() const noexcept;
    [[nodiscard]] CubingCore::Penalty pendingPenalty() const noexcept
    {
        return _pendingPenalty;
    }

  public slots:
    /// Press event (spacebar down or finger touch). Drives Idle→HoldPending or
    /// InspectionRunning→HoldPending depending on inspection setting.
    void holdBegin();
    /// Release event. From Armed it starts the run; from HoldPending it cancels.
    void holdEnd();
    /// Stop event — any subsequent key while in Running ends the solve.
    void stop();
    /// Reset to Idle without recording anything (typically bound to Escape).
    void reset();

  signals:
    void stateChanged();
    void elapsedChanged();
    void holdMsChanged();
    void inspectionEnabledChanged();
    void pendingPenaltyChanged();

    /// Emitted when a solve has finished — the caller is responsible for
    /// pairing the raw time with the active session, puzzle and scramble.
    /// @param rawMs        unpenalised solve duration in milliseconds.
    /// @param penalty      one of CubingCore::Penalty (cast to int).
    /// @param inspectionMs inspection time used, or 0 if disabled.
    void solveFinished(qint64 rawMs, int penalty, qint64 inspectionMs);

  private:
    void enter(State next);
    void tick();
    void onHoldTimerTimeout();

    State _state = State::Idle;
    int _holdMs = 550;
    bool _inspectionEnabled = false;
    CubingCore::Penalty _pendingPenalty = CubingCore::Penalty::Ok;

    QElapsedTimer _solveTimer;
    QElapsedTimer _inspectionTimer;
    QElapsedTimer _holdTimer;
    QTimer _holdArmTimer;
    QTimer _uiTickTimer;

    qint64 _lastSolveMs = 0;
    qint64 _lastInspectionMs = 0;
};

} // namespace CubingTimer
