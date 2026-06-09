qt_add_executable(CubingTimerApp
    main.cpp
    TimerController.cpp
    ScrambleProvider.cpp
    SessionModel.cpp
    StatsModel.cpp
    ProfileController.cpp
)

qt_policy(SET QTP0004 NEW)

qt_add_qml_module(CubingTimerApp
    URI CubingTimer
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/TimerScreen.qml
        qml/StatsPanel.qml
        qml/HistoryGraph.qml
        qml/SolveList.qml
)

target_include_directories(CubingTimerApp PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(CubingTimerApp PRIVATE
    CubingCore
    CubingDB
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Charts
    Qt6::Sql
)

if(CUBINGTIMER_TESTING)
    find_package(Qt6 REQUIRED COMPONENTS Test)

    add_executable(CubingTimer_test
        TimerController.cpp
        SessionModel.cpp
        StatsModel.cpp
        TimerController_test.cpp
        StatsModel_test.cpp
    )
    set_target_properties(CubingTimer_test PROPERTIES
        AUTOMOC ON
    )
    target_include_directories(CubingTimer_test PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(CubingTimer_test PRIVATE
        CubingDB
        Qt6::Core
        Qt6::Qml
        Qt6::Test
        Catch2::Catch2WithMain
    )
    add_test(NAME CubingTimer_test COMMAND CubingTimer_test)
endif()
