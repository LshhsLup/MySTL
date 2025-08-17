#include "gtest/gtest.h"
#include "mystl/array.h" 
#include <string>
#include <numeric> // for std::accumulate
#include <memory>  // for std::unique_ptr

// --- 测试 mystl::array ---

TEST(ArrayTest, ConstructionAndInitialization) {
  // 1. 默认构造 (值是未初始化的，但大小和容量正确)
  mystl::array<int, 5> a1;
  EXPECT_EQ(a1.size(), 5);

  // 2. 聚合初始化 (最常见的用法)
  mystl::array<int, 3> a2 = {10, 20, 30};
  EXPECT_EQ(a2[0], 10);
  EXPECT_EQ(a2[1], 20);
  EXPECT_EQ(a2[2], 30);

  // 3. 使用非 POD 类型进行聚合初始化
  mystl::array<std::string, 2> a3 = {"hello", "world"};
  EXPECT_EQ(a3[0], "hello");
  EXPECT_EQ(a3[1], "world");

  // 4. 初始化列表比 N 小，其余元素进行值初始化 (对于 int 是 0)
  mystl::array<int, 5> a4 = {1, 2, 3};
  EXPECT_EQ(a4[0], 1);
  EXPECT_EQ(a4[1], 2);
  EXPECT_EQ(a4[2], 3);
  EXPECT_EQ(a4[3], 0); // 值初始化
  EXPECT_EQ(a4[4], 0);
}

TEST(ArrayTest, Capacity) {
  // 1. 测试非空 array
  mystl::array<int, 5> a1;
  EXPECT_EQ(a1.size(), 5);
  EXPECT_EQ(a1.max_size(), 5);
  EXPECT_FALSE(a1.empty());

  // 2. 测试大小为 0 的 array (特殊情况)
  mystl::array<int, 0> a2;
  EXPECT_EQ(a2.size(), 0);
  EXPECT_EQ(a2.max_size(), 0);
  EXPECT_TRUE(a2.empty());
}

TEST(ArrayTest, ElementAccess) {
  mystl::array<int, 4> arr = {10, 20, 30, 40};
  const mystl::array<int, 4>& carr = arr;

  // 1. operator[]
  EXPECT_EQ(arr[0], 10);
  EXPECT_EQ(carr[1], 20);
  arr[0] = 100;
  EXPECT_EQ(arr[0], 100);

  // 2. at()
  EXPECT_EQ(arr.at(2), 30);
  EXPECT_EQ(carr.at(3), 40);
  arr.at(1) = 200;
  EXPECT_EQ(arr.at(1), 200);

  // 3. at() 边界检查
  EXPECT_THROW(arr.at(4), std::out_of_range);
  EXPECT_THROW(carr.at(5), std::out_of_range);

  // 4. front() 和 back()
  EXPECT_EQ(arr.front(), 100);
  EXPECT_EQ(carr.front(), 100);
  EXPECT_EQ(arr.back(), 40);
  EXPECT_EQ(carr.back(), 40);
  arr.back() = 400;
  EXPECT_EQ(arr.back(), 400);

  // 5. data()
  EXPECT_EQ(*arr.data(), 100);
  EXPECT_EQ(arr.data(), &arr[0]);
  EXPECT_EQ(carr.data()[1], 200);
}

TEST(ArrayTest, Iterators) {
  mystl::array<int, 5> arr = {1, 2, 3, 4, 5};
  const mystl::array<int, 5> carr = arr;

  // 1. begin(), end() 和 cbegin(), cend()
  int sum = std::accumulate(arr.begin(), arr.end(), 0);
  EXPECT_EQ(sum, 15);
  sum = std::accumulate(carr.begin(), carr.end(), 0);
  EXPECT_EQ(sum, 15);
  sum = std::accumulate(arr.cbegin(), arr.cend(), 0);
  EXPECT_EQ(sum, 15);
  
  // 2. rbegin(), rend() 和 crbegin(), crend()
  std::string s;
  for (auto it = arr.rbegin(); it != arr.rend(); ++it) {
    s += std::to_string(*it);
  }
  EXPECT_EQ(s, "54321");
  
  s.clear();
  for (auto it = carr.crbegin(); it != carr.crend(); ++it) {
    s += std::to_string(*it);
  }
  EXPECT_EQ(s, "54321");

  // 3. N=0 的特殊情况
  mystl::array<int, 0> zero_arr;
  EXPECT_EQ(zero_arr.begin(), zero_arr.end());
  EXPECT_EQ(zero_arr.cbegin(), zero_arr.cend());
  EXPECT_EQ(zero_arr.rbegin(), zero_arr.rend());
  EXPECT_EQ(zero_arr.crbegin(), zero_arr.crend());
}

TEST(ArrayTest, Modifiers) {
  // 1. fill()
  mystl::array<int, 5> a1;
  a1.fill(123);
  for (size_t i = 0; i < a1.size(); ++i) {
    EXPECT_EQ(a1[i], 123);
  }

  // 2. swap()
  mystl::array<std::string, 2> a2 = {"A", "B"};
  mystl::array<std::string, 2> a3 = {"C", "D"};
  
  a2.swap(a3);
  EXPECT_EQ(a2[0], "C");
  EXPECT_EQ(a2[1], "D");
  EXPECT_EQ(a3[0], "A");
  EXPECT_EQ(a3[1], "B");
  
  // 3. 非成员 swap()
  std::swap(a2, a3);
  EXPECT_EQ(a2[0], "A");
  EXPECT_EQ(a2[1], "B");
  EXPECT_EQ(a3[0], "C");
  EXPECT_EQ(a3[1], "D");
}

TEST(ArrayTest, ComparisonOperators) {
  mystl::array<int, 3> a1 = {1, 2, 3};
  mystl::array<int, 3> a2 = {1, 2, 3};
  mystl::array<int, 3> a3 = {1, 2, 4};
  mystl::array<int, 3> a4 = {3, 2, 1};

  // == 和 !=
  EXPECT_TRUE(a1 == a2);
  EXPECT_FALSE(a1 == a3);
  EXPECT_TRUE(a1 != a3);

  // < 和 >
  EXPECT_TRUE(a1 < a3);
  EXPECT_TRUE(a1 < a4);
  EXPECT_TRUE(a4 > a1);
  EXPECT_FALSE(a1 > a3);

  // <= 和 >=
  EXPECT_TRUE(a1 <= a2);
  EXPECT_TRUE(a1 <= a3);
  EXPECT_TRUE(a2 >= a1);
  EXPECT_TRUE(a3 >= a1);
}

TEST(ArrayTest, Get) {
  mystl::array<int, 3> arr = {10, 20, 30};

  // 1. get from lvalue array
  EXPECT_EQ(mystl::get<0>(arr), 10);
  mystl::get<1>(arr) = 200;
  EXPECT_EQ(arr[1], 200);

  // 2. get from const lvalue array
  const mystl::array<int, 3>& carr = arr;
  EXPECT_EQ(mystl::get<2>(carr), 30);

  // 3. get from rvalue array
  EXPECT_EQ(mystl::get<0>(mystl::array<int, 1>{99}), 99);
  
  // 4. get rvalue from rvalue array (测试移动语义)
  mystl::array<std::unique_ptr<int>, 1> ptr_arr;
  ptr_arr[0] = std::make_unique<int>(500);
  
  std::unique_ptr<int> moved_ptr = mystl::get<0>(std::move(ptr_arr));
  EXPECT_NE(moved_ptr, nullptr);
  EXPECT_EQ(*moved_ptr, 500);
  EXPECT_EQ(ptr_arr[0], nullptr); // 原始 array 中的资源已被移走
}

TEST(ArrayTest, TupleInterface) {
  // 这个测试用例的目的是验证 std 命名空间下的特化是否正确
  // 只要能编译通过，就证明特化是有效的
  using MyArray = mystl::array<int, 5>;
  
  // 1. 测试 std::tuple_size
  static_assert(std::tuple_size<MyArray>::value == 5, 
                "tuple_size specialization failed");
  
  // 2. 测试 std::tuple_element
  static_assert(std::is_same<std::tuple_element<0, MyArray>::type, int>::value,
                "tuple_element specialization failed for index 0");
  static_assert(std::is_same<std::tuple_element<4, MyArray>::type, int>::value,
                "tuple_element specialization failed for index 4");

#if __cplusplus >= 201703L
  static_assert(std::tuple_size_v<MyArray> == 5, "tuple_size_v failed");
  static_assert(std::is_same_v<std::tuple_element_t<2, MyArray>, int>,
                "tuple_element_t failed");
#endif

  // 仅仅为了让测试不显示 "empty test" 警告
  EXPECT_TRUE(true);
}