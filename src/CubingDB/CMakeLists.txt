add_library(CubingDB STATIC
    SqliteSolveStore.cpp
)

target_include_directories(CubingDB PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
)

target_link_libraries(CubingDB
    PUBLIC CubingCore Qt6::Core Qt6::Sql
)

target_compile_features(CubingDB PUBLIC cxx_std_23)

set_target_properties(CubingDB PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    AUTOMOC OFF
    AUTORCC OFF
    AUTOUIC OFF
)

if(CUBINGTIMER_TESTING)
    add_executable(CubingDB_test
        ${CMAKE_SOURCE_DIR}/src/test_main.cpp
        SqliteSolveStore_test.cpp
    )
    target_link_libraries(CubingDB_test PRIVATE CubingDB Catch2::Catch2)
    add_test(NAME CubingDB_test COMMAND CubingDB_test)
endif()
