add_library(CubingCore STATIC
    Scrambler.cpp
    Stats.cpp
    CsvIo.cpp
    JsonIo.cpp
)

target_include_directories(CubingCore PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
)

target_compile_features(CubingCore PUBLIC cxx_std_23)

set_target_properties(CubingCore PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

if(CUBINGTIMER_TESTING)
    add_executable(CubingCore_test
        ${CMAKE_SOURCE_DIR}/src/test_main.cpp
        Scrambler_test.cpp
        Solve_test.cpp
        Stats_test.cpp
        CsvIo_test.cpp
        JsonIo_test.cpp
        Puzzle_test.cpp
    )
    target_link_libraries(CubingCore_test PRIVATE CubingCore Qt6::Core Catch2::Catch2)
    add_test(NAME CubingCore_test COMMAND CubingCore_test)
endif()
