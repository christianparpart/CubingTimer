cmake_minimum_required(VERSION 3.25 FATAL_ERROR)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(Version)
GetVersionInformation(CUBINGTIMER_VERSION CUBINGTIMER_VERSION_STRING)
project(CubingTimer VERSION "${CUBINGTIMER_VERSION}" LANGUAGES CXX)

if(NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 23)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_COLOR_DIAGNOSTICS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose the build mode." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release MinSizeRel RelWithDebInfo)
endif()

if((CMAKE_CXX_COMPILER_ID STREQUAL "Clang") OR (CMAKE_CXX_COMPILER_ID STREQUAL "GNU"))
    set(CMAKE_CXX_FLAGS_DEBUG "-Og -g")
endif()

if(APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexperimental-library")
endif()

include(EnableCcache)
include(ClangTidy)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(MAINTAINER_MODE ON)
else()
    set(MAINTAINER_MODE OFF)
endif()

option(CUBINGTIMER_TESTING "Enable unit tests [default: ${MAINTAINER_MODE}]" ${MAINTAINER_MODE})
option(CUBINGTIMER_COVERAGE "Build with coverage instrumentation [default: OFF]" OFF)
option(PEDANTIC_COMPILER "Enable pedantic warnings" ON)
option(PEDANTIC_COMPILER_WERROR "Treat warnings as errors" OFF)

include(PedanticCompiler)
include(Sanitizers)

# Qt6 ---------------------------------------------------------------------
set(QT_MIN_VERSION 6.6) # QtGraphs lands in 6.6
find_package(Qt6 ${QT_MIN_VERSION} REQUIRED
    COMPONENTS Core Gui Qml Quick QuickControls2 Graphs Sql)

qt_standard_project_setup(REQUIRES ${QT_MIN_VERSION})

# Testing -----------------------------------------------------------------
if(CUBINGTIMER_TESTING)
    enable_testing()
    # Prefer a system-provided Catch2 (faster, cached by package manager).
    # Fall back to CPM if not installed, which downloads and pins the release.
    find_package(Catch2 3 QUIET)
    if(NOT Catch2_FOUND)
        message(STATUS "Catch2 not found via find_package, fetching via CPM")
        include(cmake/CPM.cmake)
        CPMAddPackage(
            NAME Catch2
            GITHUB_REPOSITORY catchorg/Catch2
            VERSION 3.5.4
            EXCLUDE_FROM_ALL YES
            SYSTEM YES
        )
    endif()
endif()

if(CUBINGTIMER_COVERAGE)
    add_compile_options(--coverage -O0 -g)
    add_link_options(--coverage)
endif()

add_subdirectory(src)

message(STATUS "")
message(STATUS "CubingTimer ${CUBINGTIMER_VERSION_STRING}")
message(STATUS "  Build type:  ${CMAKE_BUILD_TYPE}")
message(STATUS "  Qt version:  ${Qt6_VERSION}")
message(STATUS "  Tests:       ${CUBINGTIMER_TESTING}")
message(STATUS "")
