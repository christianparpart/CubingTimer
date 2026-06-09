// SPDX-License-Identifier: Apache-2.0
#include <CubingDB/InMemorySolveStore.h>
#include <CubingDB/SqliteSolveStore.h>
#include <CubingTimer/ProfileController.h>
#include <CubingTimer/ScrambleProvider.h>
#include <CubingTimer/SessionModel.h>
#include <CubingTimer/StatsModel.h>
#include <CubingTimer/TimerController.h>

#include <QtCore/QDir>
#include <QtCore/QStandardPaths>
#include <QtGui/QGuiApplication>
#include <QtQml/QQmlApplicationEngine>
#include <QtQml/QQmlContext>
#include <QtQuickControls2/QQuickStyle>

#include <memory>

namespace
{
    /// Picks a storage backend depending on platform. WASM gets the in-memory
    /// store because the browser sandbox doesn't expose a real filesystem;
    /// desktop / Android open a file under writable AppLocalData.
    std::unique_ptr<CubingCore::ISolveStore> makeStore()
    {
#ifdef Q_OS_WASM
        return std::make_unique<CubingDB::InMemorySolveStore>();
#else
        auto const dir = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
        QDir().mkpath(dir);
        auto const path = QDir(dir).filePath(QStringLiteral("cubingtimer.sqlite"));
        return std::make_unique<CubingDB::SqliteSolveStore>(path.toStdString());
#endif
    }
} // namespace

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);
    QGuiApplication::setApplicationName("CubingTimer");
    QGuiApplication::setOrganizationName("CubingTimer");
    QQuickStyle::setStyle("Material");

    auto store = makeStore();

    CubingTimer::ProfileController profileController;
    profileController.setStore(store.get());

    CubingTimer::SessionModel sessionModel;
    sessionModel.setStore(store.get());
    sessionModel.setSessionId(profileController.currentSessionId());

    QObject::connect(&profileController, &CubingTimer::ProfileController::currentSessionChanged,
                     &sessionModel, [&]() {
                         sessionModel.setSessionId(profileController.currentSessionId());
                     });

    CubingTimer::StatsModel statsModel;
    statsModel.setSource(&sessionModel);

    CubingTimer::ScrambleProvider scrambleProvider;
    QObject::connect(&profileController, &CubingTimer::ProfileController::currentSessionChanged,
                     &scrambleProvider, [&]() {
                         scrambleProvider.setPuzzle(profileController.currentPuzzleKey());
                     });
    scrambleProvider.setPuzzle(profileController.currentPuzzleKey());

    CubingTimer::TimerController timerController;
    QObject::connect(&timerController, &CubingTimer::TimerController::solveFinished,
                     &sessionModel, [&](qint64 rawMs, int penalty, qint64 inspectionMs) {
                         sessionModel.addSolve(rawMs,
                                               penalty,
                                               scrambleProvider.current(),
                                               inspectionMs,
                                               scrambleProvider.puzzle());
                         scrambleProvider.next();
                     });

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("profileController", &profileController);
    engine.rootContext()->setContextProperty("sessionModel", &sessionModel);
    engine.rootContext()->setContextProperty("statsModel", &statsModel);
    engine.rootContext()->setContextProperty("scrambleProvider", &scrambleProvider);
    engine.rootContext()->setContextProperty("timerController", &timerController);

    engine.loadFromModule("CubingTimer", "Main");
    if (engine.rootObjects().isEmpty())
        return 1;
    return QGuiApplication::exec();
}
