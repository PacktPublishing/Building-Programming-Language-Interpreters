find_package(GTest REQUIRED)

foreach(
    TEST
    001-empty
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
