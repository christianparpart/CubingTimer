// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <CubingCore/ISolveStore.hpp>
#include <CubingCore/Profile.hpp>
#include <CubingCore/Session.hpp>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariantList>
#include <QtQml/QQmlEngine>

namespace CubingTimer
{

/// Manages which profile and session are active. Exposes the lists to QML and
/// owns "create profile / session / pick existing" logic so QML stays declarative.
class ProfileController: public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QStringList profileNames READ profileNames NOTIFY profilesChanged)
    Q_PROPERTY(QVariantList sessions READ sessions NOTIFY sessionsChanged)
    Q_PROPERTY(qint64 currentProfileId READ currentProfileId NOTIFY currentProfileChanged)
    Q_PROPERTY(qint64 currentSessionId READ currentSessionId NOTIFY currentSessionChanged)
    Q_PROPERTY(QString currentProfileName READ currentProfileName NOTIFY currentProfileChanged)
    Q_PROPERTY(QString currentPuzzleKey READ currentPuzzleKey NOTIFY currentSessionChanged)

  public:
    explicit ProfileController(QObject* parent = nullptr);

    void setStore(CubingCore::ISolveStore* store);

    [[nodiscard]] QStringList profileNames() const;
    [[nodiscard]] QVariantList sessions() const;
    [[nodiscard]] qint64 currentProfileId() const noexcept
    {
        return _currentProfile.id;
    }
    [[nodiscard]] qint64 currentSessionId() const noexcept
    {
        return _currentSession.id;
    }
    [[nodiscard]] QString currentProfileName() const
    {
        return QString::fromStdString(_currentProfile.name);
    }
    [[nodiscard]] QString currentPuzzleKey() const;

  public slots:
    /// Creates a new profile, refreshes the list, and switches to it.
    /// @param name display name for the new profile.
    void createProfile(QString const& name);
    /// Switches to the profile at index `index` in `profileNames()`.
    /// @param index zero-based row index; out-of-range values are ignored.
    void selectProfile(int index);
    /// Renames the profile at `index`. Empty/whitespace names are ignored.
    /// @param index zero-based row index in `profileNames()`.
    /// @param newName new display name.
    void renameProfile(int index, QString const& newName);
    /// Deletes the profile at `index`. The last profile cannot be deleted —
    /// a default profile is recreated immediately if there would be none left.
    /// @param index zero-based row index in `profileNames()`.
    void deleteProfile(int index);
    /// Moves the profile at `from` to position `to` in the list.
    /// Both must be valid indices; otherwise no-op.
    /// @param from current zero-based index.
    /// @param to   new zero-based index.
    void moveProfile(int from, int to);

    /// Creates a session for the current profile and switches to it.
    /// @param name      display name for the new session.
    /// @param puzzleKey "222" / "333" / "444".
    void createSession(QString const& name, QString const& puzzleKey);
    /// Switches to the session at index `index` within the current profile.
    /// @param index zero-based row index; out-of-range values are ignored.
    void selectSession(int index);

    /// Refreshes the profile/session lists from the store.
    void reload();

  signals:
    void profilesChanged();
    void sessionsChanged();
    void currentProfileChanged();
    void currentSessionChanged();

  private:
    void ensureDefaults();

    CubingCore::ISolveStore* _store = nullptr;
    std::vector<CubingCore::Profile> _profiles;
    std::vector<CubingCore::Session> _sessions;
    CubingCore::Profile _currentProfile;
    CubingCore::Session _currentSession;
};

} // namespace CubingTimer
