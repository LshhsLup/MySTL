cmake_minimum_required(VERSION 3.11)
project(mystl CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(mystl INTERFACE)

target_include_directories(mystl INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 开启测试功能
enable_testing()

# --- 集成 GoogleTest ---
# 将 GoogleTest 源码目录添加到 CMake 构建树中
# 这会执行 3rdparty/googletest/CMakeLists.txt
# 它会定义 gtest, gmock, gtest_main 等 target
add_subdirectory(3rdparty/googletest)


# --- 添加 tests 子目录 ---
# 现在可以安全地添加 tests 目录了, 因为里面的 CMake 文件
# 需要 gtest_main 等 target, 而这些 target 刚刚已经被定义
add_subdirectory(tests)