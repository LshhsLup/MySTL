
add_executable(test_pair test_pair.cpp)
target_include_directories(test_pair PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
# 链接 test_pair 到我们自己的 mystl 库和 gtest_main
# gtest_main 会自动把 gtest 库也链接进来, 并且提供 main 函数
target_link_libraries(test_pair PRIVATE mystl gtest_main)

# 将这个可执行文件注册为一个 CTest 测试
add_test(NAME PairTest COMMAND test_pair)