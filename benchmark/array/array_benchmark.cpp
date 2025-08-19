#include <benchmark/benchmark.h>
#include <array>

#include "mystl/array.h" 

constexpr size_t N = 1000;

// --- 测试 mystl::array ---

static void BM_MyArray_Creation(benchmark::State& state) {
    for (auto _ : state) {
        mystl::array<int, N> arr;
        benchmark::DoNotOptimize(arr.data()); 
    }
}
BENCHMARK(BM_MyArray_Creation);

static void BM_MyArray_Iteration(benchmark::State& state) {
    mystl::array<int, N> arr;
    arr.fill(1);
    long long sum = 0;
    for (auto _ : state) {
        for (int x : arr) {
            sum += x;
        }
    }
    benchmark::DoNotOptimize(sum);
}
BENCHMARK(BM_MyArray_Iteration);


// --- 对比测试 std::array ---

static void BM_StdArray_Creation(benchmark::State& state) {
    for (auto _ : state) {
        std::array<int, N> arr;
        benchmark::DoNotOptimize(arr.data());
    }
}
BENCHMARK(BM_StdArray_Creation);

static void BM_StdArray_Iteration(benchmark::State& state) {
    std::array<int, N> arr;
    arr.fill(1);
    long long sum = 0;
    for (auto _ : state) {
        for (int x : arr) {
            sum += x;
        }
    }
    benchmark::DoNotOptimize(sum);
}
BENCHMARK(BM_StdArray_Iteration);


// 运行 benchmark
BENCHMARK_MAIN();