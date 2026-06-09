// SPDX-License-Identifier: Apache-2.0
#include <algorithm>
#include <ranges>

#include <CubingTimer/ProfileController.hpp>
#include <QtCore/QVariantMap>

using CubingCore::Profile;
using CubingCore::Puzzle;
using CubingCore::Session;

namespace CubingTimer
{

ProfileController::ProfileController(QObject* parent):
    QObject(parent)
{
}

void ProfileController::setStore(CubingCore::ISolveStore* store)
{
    _store = store;
    reload();
}

void ProfileController::reload()
{
    if (!_store)
        return;
    auto const profiles = _store->listProfiles();
    if (!profiles)
        return;
    _profiles = *profiles;
    emit profilesChanged();

    if (_profiles.empty())
    {
        ensureDefaults();
        return;
    }

    // If current is missing, pick the first.
    auto const stillExists = std::ranges::any_of(_profiles, [this](auto const& p) { return p.id == _currentProfile.id; });
    if (!stillExists)
        _currentProfile = _profiles.front();

    auto const sessions = _store->listSessions(_currentProfile.id);
    if (!sessions)
        return;
    _sessions = *sessions;
    if (_sessions.empty())
    {
        auto const newS = _store->createSession(_currentProfile.id, "default", Puzzle::ThreeByThree);
        if (newS)
            _sessions.push_back(*newS);
    }
    _currentSession = _sessions.front();
    emit sessionsChanged();
    emit currentProfileChanged();
    emit currentSessionChanged();
}

void ProfileController::ensureDefaults()
{
    if (!_store)
        return;
    auto const p = _store->createProfile("default");
    if (!p)
        return;
    _currentProfile = *p;
    _profiles.push_back(*p);
    emit profilesChanged();

    auto const s = _store->createSession(p->id, "default", Puzzle::ThreeByThree);
    if (s)
    {
        _sessions.push_back(*s);
        _currentSession = *s;
    }
    emit sessionsChanged();
    emit currentProfileChanged();
    emit currentSessionChanged();
}

QStringList ProfileController::profileNames() const
{
    QStringList out;
    out.reserve(static_cast<qsizetype>(_profiles.size()));
    for (auto const& p: _profiles)
        out.push_back(QString::fromStdString(p.name));
    return out;
}

QVariantList ProfileController::sessions() const
{
    QVariantList out;
    out.reserve(static_cast<qsizetype>(_sessions.size()));
    for (auto const& s: _sessions)
    {
        QVariantMap m;
        m["id"] = QVariant::fromValue<qlonglong>(s.id);
        m["name"] = QString::fromStdString(s.name);
        m["puzzle"] = QString::fromUtf8(CubingCore::specOf(s.puzzle).name.data(),
                                        static_cast<qsizetype>(CubingCore::specOf(s.puzzle).name.size()));
        out.push_back(m);
    }
    return out;
}

QString ProfileController::currentPuzzleKey() const
{
    return QString::fromUtf8(CubingCore::specOf(_currentSession.puzzle).name.data(),
                             static_cast<qsizetype>(CubingCore::specOf(_currentSession.puzzle).name.size()));
}

void ProfileController::createProfile(QString const& name)
{
    if (!_store)
        return;
    auto const p = _store->createProfile(name.toStdString());
    if (!p)
        return;
    _profiles.push_back(*p);
    _currentProfile = *p;
    emit profilesChanged();

    auto const s = _store->createSession(p->id, "default", Puzzle::ThreeByThree);
    if (s)
    {
        _sessions.clear();
        _sessions.push_back(*s);
        _currentSession = *s;
        emit sessionsChanged();
    }
    emit currentProfileChanged();
    emit currentSessionChanged();
}

void ProfileController::selectProfile(int index)
{
    if (index < 0 || static_cast<std::size_t>(index) >= _profiles.size())
        return;
    _currentProfile = _profiles[static_cast<std::size_t>(index)];
    auto const sessions = _store ? _store->listSessions(_currentProfile.id)
                                 : std::expected<std::vector<Session>, CubingCore::StoreError>(std::vector<Session> {});
    if (sessions)
        _sessions = *sessions;
    if (_sessions.empty() && _store)
    {
        auto const s = _store->createSession(_currentProfile.id, "default", Puzzle::ThreeByThree);
        if (s)
            _sessions.push_back(*s);
    }
    if (!_sessions.empty())
        _currentSession = _sessions.front();
    emit sessionsChanged();
    emit currentProfileChanged();
    emit currentSessionChanged();
}

void ProfileController::createSession(QString const& name, QString const& puzzleKey)
{
    if (!_store || _currentProfile.id == 0)
        return;
    auto const puzzle = CubingCore::puzzleFromKey(puzzleKey.toStdString());
    auto const s = _store->createSession(_currentProfile.id, name.toStdString(), puzzle);
    if (!s)
        return;
    _sessions.push_back(*s);
    _currentSession = *s;
    emit sessionsChanged();
    emit currentSessionChanged();
}

void ProfileController::selectSession(int index)
{
    if (index < 0 || static_cast<std::size_t>(index) >= _sessions.size())
        return;
    _currentSession = _sessions[static_cast<std::size_t>(index)];
    emit currentSessionChanged();
}

} // namespace CubingTimer
