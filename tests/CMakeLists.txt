find_package(GTest REQUIRED)

foreach(
    TEST
    001-empty
    002-interpreter-state
    003-literal-and-add-operations
    004-interpreter-global-state
    005-operator-eq
    006-operator-if
    007-opsequence
    008-lexicalpad
    009-recursive-factorial
    010-read-write-static-octets
    011-write-inttoascii
    012-read-int-and-terminator
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
