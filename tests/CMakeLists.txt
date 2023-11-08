find_package(GTest REQUIRED)

foreach(
    TEST
    001-empty
    002-interpreter-state
    003-literal-and-add-operations
    004-interpreter-global-state
    005-operator-eq
)
    add_executable(${TEST}.t ${TEST}.cpp)
    target_link_libraries(
        ${TEST}.t
        PUBLIC
        networkprotocoldsl
        ${GTEST_BOTH_LIBRARIES}
    )
    add_test(NAME ${TEST} COMMAND ${TEST}.t)
endforeach()
