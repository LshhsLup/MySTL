# MySTL/benchmark/CMakeLists.txt

# 定义一个函数，用于添加一个新的 benchmark 可执行文件
# 这个函数负责创建可执行文件并链接所有必要的依赖
function(mystl_add_benchmark BENCH_SOURCE)
    # 1. 从源文件名中提取基本名称，用作可执行文件的名字
    #    例如: "array/array_benchmark.cpp" -> "array_benchmark"
    get_filename_component(BENCH_NAME ${BENCH_SOURCE} NAME_WE)

    # 2. 添加可执行文件
    add_executable(${BENCH_NAME} ${BENCH_SOURCE})

    # 3. 链接我们的 mystl 库和 Google Benchmark 库
    #    因为 mystl 是 INTERFACE 库，链接它会自动处理 include 目录
    target_link_libraries(${BENCH_NAME} PRIVATE
        mystl
        benchmark::benchmark
    )
    
    #  打印一条消息，方便调试
    message(STATUS "Added benchmark: ${BENCH_NAME} from ${BENCH_SOURCE}")
endfunction()

# --- 在这里列出你所有的 benchmark 源文件 ---
set(MYSTL_BENCHMARKS
    array/array_benchmark.cpp
)

# --- 自动为列表中的每个文件创建 benchmark ---
foreach(bench_file ${MYSTL_BENCHMARKS})
    mystl_add_benchmark(${bench_file})
endforeach()