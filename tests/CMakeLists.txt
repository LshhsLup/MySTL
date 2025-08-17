
# add_executable(test_pair test_pair.cpp)
# add_executable(test_tuple test_tuple.cpp)
# target_include_directories(test_pair PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
# target_include_directories(test_tuple PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
# # 链接 test_pair 到我们自己的 mystl 库和 gtest_main
# # gtest_main 会自动把 gtest 库也链接进来, 并且提供 main 函数
# target_link_libraries(test_pair PRIVATE mystl gtest_main)
# target_link_libraries(test_tuple PRIVATE mystl gtest_main)

# # 将这个可执行文件注册为一个 CTest 测试
# add_test(NAME PairTest COMMAND test_pair)
# add_test(NAME TupleTest COMMAND test_tuple)

# 启用 CTest
enable_testing()

# 定义添加 mystl 测试的函数
function(mystl_add_test TEST_SOURCE)
    string(REPLACE ".cpp" "" TEST_NAME ${TEST_SOURCE})
    
    add_executable(${TEST_NAME} ${TEST_SOURCE})
    
    target_include_directories(${TEST_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    
    target_link_libraries(${TEST_NAME} PRIVATE mystl gtest_main)
    
    # 使用 GoogleTest 的 CMake 集成来自动发现测试用例
    include(GoogleTest)
    gtest_discover_tests(${TEST_NAME})
endfunction()

# --- 添加所有测试 ---
set(MYSTL_TESTS
    test_pair.cpp
    test_tuple.cpp
    test_array.cpp
)

foreach(test_file ${MYSTL_TESTS})
    mystl_add_test(${test_file})
endforeach()