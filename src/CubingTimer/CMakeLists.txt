qt_add_executable(CubingTimerApp
    main.cpp
    TimerController.cpp
    ScrambleProvider.cpp
    SessionModel.cpp
    StatsModel.cpp
    ProfileController.cpp
)

# QTP0004 only exists in Qt 6.8+. The policy fixes a warning about needing
# qmldir files for each QML subdirectory; on older Qt we simply live with
# the warning.
if(QT_KNOWN_POLICY_QTP0004)
    qt_policy(SET QTP0004 NEW)
endif()

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

# qt_add_qml_module appends generated cpp files (qmlcache loaders, type
# registrations) to the target. clang-tidy can't reason about them. Mark every
# generated source on the target as skip-linting.
get_target_property(_cubingtimer_sources CubingTimerApp SOURCES)
foreach(_src IN LISTS _cubingtimer_sources)
    get_source_file_property(_is_generated "${_src}" GENERATED)
    if(_is_generated)
        set_source_files_properties("${_src}" PROPERTIES SKIP_LINTING TRUE)
    endif()
endforeach()

target_include_directories(CubingTimerApp PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(CubingTimerApp PRIVATE
    CubingCore
    CubingDB
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Graphs
    Qt6::Sql
)

if(CUBINGTIMER_TESTING)
    find_package(Qt6 REQUIRED COMPONENTS Test)

    add_executable(CubingTimer_test
        ${CMAKE_SOURCE_DIR}/src/test_main.cpp
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
        Catch2::Catch2
    )
    add_test(NAME CubingTimer_test COMMAND CubingTimer_test)
endif()
