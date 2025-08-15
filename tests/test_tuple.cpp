#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>
#include "mystl/tuple.h"
#include "utils/test_types.h"

using namespace mystl::test;

TEST(TupleTest, DefaultConstructors) {
  // 编译出错
  // mystl::tuple<int, double, NoDefault> t0;
  mystl::tuple<int, std::string, std::vector<int>> t1;
  std::vector<int> v1{};
  EXPECT_EQ(mystl::get<0>(t1), 0);
  EXPECT_EQ(mystl::get<1>(t1), "");
  EXPECT_EQ(mystl::get<2>(t1), v1);
  EXPECT_EQ(mystl::get<int>(t1), 0);
  EXPECT_EQ(mystl::get<std::string>(t1), "");
  EXPECT_EQ(mystl::get<std::vector<int>>(t1), v1);
}

TEST(TupleTest, CopyAndMoveConstructors) {
  // 1. 测试拷贝构造函数
  mystl::tuple<int, std::string> t1(123, "hello");
  mystl::tuple<int, std::string> t2(t1);  // 拷贝构造

  EXPECT_EQ(mystl::get<0>(t1), 123);
  EXPECT_EQ(mystl::get<1>(t1), "hello");
  EXPECT_EQ(mystl::get<0>(t2), 123);
  EXPECT_EQ(mystl::get<1>(t2), "hello");

  // 修改 t2，t1 不应受影响
  mystl::get<1>(t2) = "world";
  EXPECT_EQ(mystl::get<1>(t1), "hello");
  EXPECT_EQ(mystl::get<1>(t2), "world");

  // 2. 测试移动构造函数
  mystl::tuple<int, std::unique_ptr<std::string>> t3(
      456, std::make_unique<std::string>("move me"));
  mystl::tuple<int, std::unique_ptr<std::string>> t4(
      std::move(t3));  // 移动构造

  EXPECT_EQ(mystl::get<0>(t4), 456);
  EXPECT_NE(mystl::get<1>(t4), nullptr);
  EXPECT_EQ(*mystl::get<1>(t4), "move me");
  // t3 的 unique_ptr 资源已被移走
  EXPECT_EQ(mystl::get<1>(t3), nullptr);
}

// 测试：转换构造函数 (不同类型)
TEST(TupleTest, ConvertingConstructors) {
  // 1. 从 const lvalue tuple 转换 (应为拷贝转换)
  mystl::tuple<long, const char*> t_source1(99L, "test source");
  mystl::tuple<int, std::string> t_dest1(t_source1);

  EXPECT_EQ(mystl::get<0>(t_dest1), 99);
  EXPECT_EQ(mystl::get<1>(t_dest1), "test source");
  // 源 tuple 保持不变
  EXPECT_EQ(mystl::get<0>(t_source1), 99L);

  // 2. 从 rvalue tuple 转换 (应为移动转换)
  mystl::tuple<int, std::string> t_dest2(
      mystl::tuple<long, const char*>(88L, "another test"));

  EXPECT_EQ(mystl::get<0>(t_dest2), 88);
  EXPECT_EQ(mystl::get<1>(t_dest2), "another test");

  // 3. 使用 move-only 类型测试 rvalue 转换构造
  auto source_ptr = std::make_unique<int>(100);
  mystl::tuple<std::unique_ptr<int>> t_source3(std::move(source_ptr));
  // 从 tuple<unique_ptr<int>> 移动构造成 tuple<unique_ptr<const int>>
  mystl::tuple<std::unique_ptr<const int>> t_dest3(std::move(t_source3));

  ASSERT_NE(mystl::get<0>(t_dest3), nullptr);
  EXPECT_EQ(*mystl::get<0>(t_dest3), 100);
  // 验证源 unique_ptr 已被移走
  EXPECT_EQ(mystl::get<0>(t_source3), nullptr);

  // 4. 测试单元素元组的转换规则
  mystl::tuple<long> single_source(12345L);
  mystl::tuple<int> single_dest(single_source);  // 转换构造
  EXPECT_EQ(mystl::get<0>(single_dest), 12345);

  mystl::tuple<int> single_source_same(99);
  mystl::tuple<int> single_dest_same(
      single_source_same);  // 应该调用拷贝构造，而不是转换构造
  EXPECT_EQ(mystl::get<0>(single_dest_same), 99);
}

// 测试：从 pair 构造元组
TEST(TupleTest, PairConstructors) {
  // 1. 从 const lvalue pair 构造 (拷贝转换)
  mystl::pair<int, std::string> p1(200, "from pair");
  mystl::tuple<long, std::string> t1(p1);

  EXPECT_EQ(mystl::get<0>(t1), 200L);
  EXPECT_EQ(mystl::get<1>(t1), "from pair");
  // 原始 pair 不变
  EXPECT_EQ(p1.first, 200);

  // 2. 从 rvalue std::pair 构造 (移动转换)
  mystl::tuple<int, std::unique_ptr<int>> t2(
      mystl::make_pair(300, std::make_unique<int>(400)));

  EXPECT_EQ(mystl::get<0>(t2), 300);
  ASSERT_NE(mystl::get<1>(t2), nullptr);
  EXPECT_EQ(*mystl::get<1>(t2), 400);

  // 3. 从 rvalue mystl::pair 构造 (移动转换)
  mystl::pair<int, std::unique_ptr<int>> p3(500, std::make_unique<int>(600));
  mystl::tuple<long, std::unique_ptr<int>> t3(std::move(p3));

  EXPECT_EQ(mystl::get<0>(t3), 500L);
  ASSERT_NE(mystl::get<1>(t3), nullptr);
  EXPECT_EQ(*mystl::get<1>(t3), 600);
  // 验证原始 pair 的资源已被移走
  EXPECT_EQ(p3.second, nullptr);
}

TEST(TupleTest, AssignOperators) {
  // 1. 同类型 copy assign
  mystl::tuple<int, std::string> t1(1, "hello");
  mystl::tuple<int, std::string> t2;
  t2 = t1;
  EXPECT_EQ(mystl::get<0>(t2), 1);
  EXPECT_EQ(mystl::get<1>(t2), "hello");

  // 2. 同类型 move assign
  mystl::tuple<int, std::string> t3;
  t3 = std::move(t1);
  EXPECT_EQ(mystl::get<0>(t3), 1);
  EXPECT_EQ(mystl::get<1>(t3), "hello");

  // 3. 不同类型（可转换） copy assign
  mystl::tuple<int, double> t4(42, 3.14);
  mystl::tuple<long, long double> t5;
  t5 = t4;
  EXPECT_EQ(mystl::get<0>(t5), 42);
  EXPECT_EQ(mystl::get<1>(t5), 3.14);

  // 4. 不同类型 move assign
  mystl::tuple<long, std::string> t6(99, "world");
  mystl::tuple<int, std::string> t7;
  t7 = std::move(t6);
  EXPECT_EQ(mystl::get<0>(t7), 99);
  EXPECT_EQ(mystl::get<1>(t7), "world");

  // 5. 从 pair copy assign
  mystl::pair<int, std::string> p1(77, "pair");
  mystl::tuple<int, std::string> t8;
  t8 = p1;
  EXPECT_EQ(mystl::get<0>(t8), 77);
  EXPECT_EQ(mystl::get<1>(t8), "pair");

  // 6. 从 pair move assign
  mystl::pair<int, std::string> p2(88, "move");
  mystl::tuple<int, std::string> t9;
  t9 = std::move(p2);
  EXPECT_EQ(mystl::get<0>(t9), 88);
  EXPECT_EQ(mystl::get<1>(t9), "move");
}

// ---------- TEST 2: make_tuple ----------
TEST(TupleTest, MakeTuple) {
  // 1. 基本类型
  auto t1 = mystl::make_tuple(1, 2.5, 'a');
  EXPECT_EQ(mystl::get<0>(t1), 1);
  EXPECT_DOUBLE_EQ(mystl::get<1>(t1), 2.5);
  EXPECT_EQ(mystl::get<2>(t1), 'a');

  // 2. 引用包装
  int x = 42;
  auto t2 = mystl::make_tuple(std::ref(x));
  mystl::get<0>(t2) = 99;
  EXPECT_EQ(x, 99);

  // 3. 混合类型（std::string + const char*）
  auto t3 = mystl::make_tuple(std::string("abc"), "xyz");
  EXPECT_EQ(mystl::get<0>(t3), "abc");
  EXPECT_EQ(mystl::get<1>(t3), std::string("xyz"));
}

// ---------- TEST 3: tie & swap ----------
TEST(TupleTest, TieAndSwap) {
  // 1. tie 绑定引用
  int a = 1, b = 2;
  mystl::tie(a, b) = mystl::make_pair(10, 20);
  EXPECT_EQ(a, 10);
  EXPECT_EQ(b, 20);

  // 2. ignore
  mystl::tie(a, mystl::ignore) = mystl::make_tuple(42, 999);
  EXPECT_EQ(a, 42);
  EXPECT_EQ(b, 20);  // b 未被修改

  // 3. swap
  mystl::tuple<int, std::string> t1(1, "hello");
  mystl::tuple<int, std::string> t2(2, "world");
  t1.swap(t2);
  EXPECT_EQ(mystl::get<0>(t1), 2);
  EXPECT_EQ(mystl::get<1>(t1), "world");
  EXPECT_EQ(mystl::get<0>(t2), 1);
  EXPECT_EQ(mystl::get<1>(t2), "hello");
}

TEST(TupleTest, ComparisonOperators) {
  mystl::tuple<int, std::string> t1(1, "hello");
  mystl::tuple<int, std::string> t2(1, "hello");
  mystl::tuple<int, std::string> t3(2, "hello");
  mystl::tuple<int, std::string> t4(1, "world");

  // Equality
  EXPECT_TRUE(t1 == t2);
  EXPECT_FALSE(t1 == t3);
  EXPECT_FALSE(t1 == t4);

  // Inequality
  EXPECT_FALSE(t1 != t2);
  EXPECT_TRUE(t1 != t3);
  EXPECT_TRUE(t1 != t4);

  mystl::tuple<int, double, const char*> t5(10, 3.14, "test");
  mystl::tuple<long, float, std::string> t6(10, 3.14f, "test");
  mystl::tuple<short, float, std::string> t7(11, 3.14f, "test");

  // Equality with convertible types
  EXPECT_TRUE(t5 != t6);
  EXPECT_FALSE(t5 == t7);

  mystl::tuple<> empty1;
  mystl::tuple<> empty2;
  mystl::tuple<int> not_empty(1);

  EXPECT_TRUE(empty1 == empty2);
  EXPECT_FALSE(empty1 != empty2);
  // Compare first element
  mystl::tuple<int, int> t8(1, 100);
  mystl::tuple<int, int> t9(2, 1);
  EXPECT_TRUE(t8 < t9);
  EXPECT_FALSE(t9 < t8);

  // First elements are equal, compare second
  mystl::tuple<int, std::string> t10(5, "apple");
  mystl::tuple<int, std::string> t11(5, "banana");
  EXPECT_TRUE(t10 < t11);
  EXPECT_FALSE(t11 < t10);

  // Tuples are identical, so not less
  mystl::tuple<int, int> t12(10, 20);
  mystl::tuple<int, int> t13(10, 20);
  EXPECT_FALSE(t12 < t13);

  mystl::tuple<int, std::string> t14(10, "a");
  mystl::tuple<long, const char*> t15(10, "b");
  mystl::tuple<int, const char*> t16(9, "z");

  EXPECT_TRUE(t14 < t15);  // 10 == 10, "a" < "b"
  EXPECT_FALSE(t15 < t14);
  EXPECT_TRUE(t16 < t14);  // 9 < 10
  EXPECT_FALSE(t14 < t16);

  mystl::tuple<int, int> t_small(1, 1);
  mystl::tuple<int, int> t_large(1, 2);
  mystl::tuple<int, int> t_equal(1, 1);

  // Operator <
  EXPECT_TRUE(t_small < t_large);
  EXPECT_FALSE(t_large < t_small);
  EXPECT_FALSE(t_small < t_equal);

  // Operator <=
  EXPECT_TRUE(t_small <= t_large);
  EXPECT_TRUE(t_small <= t_equal);
  EXPECT_FALSE(t_large <= t_small);

  // Operator >
  EXPECT_TRUE(t_large > t_small);
  EXPECT_FALSE(t_small > t_large);
  EXPECT_FALSE(t_equal > t_small);

  // Operator >=
  EXPECT_TRUE(t_large >= t_small);
  EXPECT_TRUE(t_equal >= t_small);
  EXPECT_FALSE(t_small >= t_large);

  mystl::tuple<> empty3;
  mystl::tuple<> empty4;

  EXPECT_FALSE(empty3 < empty4);
  EXPECT_FALSE(empty3 > empty4);
  EXPECT_TRUE(empty3 <= empty4);

  EXPECT_TRUE(empty3 >= empty4);
}

TEST(TupleTest, ForwardAsTuple) {
  int x = 42;
  double y = 3.14;
  auto t = mystl::forward_as_tuple(x, y);

  // 检查类型是引用
  static_assert(std::is_same<typename std::tuple_element<0, decltype(t)>::type,
                             int&>::value,
                "Should be int&");
  static_assert(std::is_same<typename std::tuple_element<1, decltype(t)>::type,
                             double&>::value,
                "Should be double&");
  // 验证元素值
  EXPECT_EQ(mystl::get<0>(t), 42);
  EXPECT_DOUBLE_EQ(mystl::get<1>(t), 3.14);
  // 修改 tuple 元素，原值也应改变
  mystl::get<0>(t) = 100;
  mystl::get<1>(t) = 6.28;
  EXPECT_EQ(x, 100);
  EXPECT_DOUBLE_EQ(y, 6.28);

  auto t1 = mystl::forward_as_tuple(123, 4.125);

  // 检查类型是右值引用
  static_assert(std::is_same<typename std::tuple_element<0, decltype(t1)>::type,
                             int&&>::value,
                "Should be int&&");
  static_assert(std::is_same<typename std::tuple_element<1, decltype(t1)>::type,
                             double&&>::value,
                "Should be double&&");
  // 检查值
  EXPECT_EQ(mystl::get<0>(t1), 123);
  EXPECT_DOUBLE_EQ(mystl::get<1>(t1), 4.125);

  int a = 10;
  auto t2 = std::forward_as_tuple(a, 20);
  // 第一个是左值引用，第二个是右值引用
  static_assert(std::is_same<typename std::tuple_element<0, decltype(t2)>::type,
                             int&>::value,
                "Should be int&");
  static_assert(std::is_same<typename std::tuple_element<1, decltype(t2)>::type,
                             int&&>::value,
                "Should be int&&");
  // 修改第一个
  std::get<0>(t2) = 99;
  EXPECT_EQ(a, 99);
  // 第二个是右值，不能直接用来修改原值（原值已经是临时对象）
  // 未定义行为， 20 这个临时对象在构造的时候就已经被销毁了
  // forward_as_tuple 构造的时候 20 的右值引用
  // 20 这个临时对象在 forward_as_tuple 返回后就被销毁了
  // 再去 && 引用这个临时对象是未定义行为
  // EXPECT_EQ(std::get<1>(t2), 20);
}

// ---------- tuple_cat ----------
TEST(TupleTest, TupleCat) {
  mystl::tuple<int, char> t1(1, 'a');
  mystl::tuple<std::string, double> t2("hello", 3.14);
  mystl::tuple<> t3;  // 空 tuple
  mystl::tuple<long> t4(100L);

  // 拼接两个非空 tuple
  auto cat1 = mystl::tuple_cat(t1, t2);
  static_assert(
      std::is_same<decltype(cat1),
                   mystl::tuple<int, char, std::string, double>>::value,
      "Type mismatch in tuple_cat");
  EXPECT_EQ(mystl::get<0>(cat1), 1);
  EXPECT_EQ(mystl::get<1>(cat1), 'a');
  EXPECT_EQ(mystl::get<2>(cat1), "hello");
  EXPECT_DOUBLE_EQ(mystl::get<3>(cat1), 3.14);

  // 拼接包含空 tuple
  auto cat2 = mystl::tuple_cat(t1, t3, t4);
  static_assert(
      std::is_same<decltype(cat2), mystl::tuple<int, char, long>>::value,
      "Type mismatch in tuple_cat with empty tuple");
  EXPECT_EQ(mystl::get<0>(cat2), 1);
  EXPECT_EQ(mystl::get<1>(cat2), 'a');
  EXPECT_EQ(mystl::get<2>(cat2), 100L);

  // 拼接多个 tuple
  auto cat3 = mystl::tuple_cat(t3, t1, t2, t4);
  static_assert(
      std::is_same<decltype(cat3),
                   mystl::tuple<int, char, std::string, double, long>>::value,
      "Type mismatch in tuple_cat with multiple tuples");
  EXPECT_EQ(mystl::get<0>(cat3), 1);
  EXPECT_EQ(mystl::get<1>(cat3), 'a');
  EXPECT_EQ(mystl::get<2>(cat3), "hello");
  EXPECT_DOUBLE_EQ(mystl::get<3>(cat3), 3.14);
  EXPECT_EQ(mystl::get<4>(cat3), 100L);
}

TEST(TupleTest, StdSwap) {
  mystl::tuple<int, double, char> t1(1, 2.2, 'a');
  mystl::tuple<int, double, char> t2(99, 8.8, 'z');

  const auto t1_original = t1;
  const auto t2_original = t2;

  ASSERT_NE(t1, t2);

  std::swap(t1, t2);

  EXPECT_EQ(t1, t2_original);
  EXPECT_EQ(t2, t1_original);

  mystl::tuple<> t3;
  mystl::tuple<> t4;

  const auto t3_original = t3;
  const auto t4_original = t4;

  ASSERT_EQ(t3, t4);

  // 交换空元组应该是一个无操作，且不会导致编译或运行时错误
  std::swap(t3, t4);

  // 状态应该保持不变
  EXPECT_EQ(t3, t3_original);
  EXPECT_EQ(t4, t4_original);
  EXPECT_EQ(t3, t4);
}

// from cpprefernece
TEST(TupleTest, ExampleTest) {
  // 1. Test Default Constructor (Value Initialization)
  // Corresponds to: std::tuple<int, std::string, double> t1;
  {
    mystl::tuple<int, std::string, double> t1;
    EXPECT_EQ(mystl::get<0>(t1), 0);
    EXPECT_EQ(mystl::get<1>(t1), "");
    EXPECT_EQ(mystl::get<2>(t1), 0.0);
  }

  // 2. Test Value Constructor
  // Corresponds to: std::tuple<int, std::string, double> t2{42, "Test", -3.14};
  {
    mystl::tuple<int, std::string, double> t2(42, "Test", -3.14);
    EXPECT_EQ(mystl::get<0>(t2), 42);
    EXPECT_EQ(mystl::get<1>(t2), "Test");
    EXPECT_DOUBLE_EQ(mystl::get<2>(t2),
                     -3.14);  // Use DOUBLE_EQ for floating point
  }

  // 3. Test Converting Constructor
  // Corresponds to: std::tuple<char, std::string, int> t3{t2};
  {
    mystl::tuple<int, const char*, double> t_source(65, "Test",
                                                    -3.14);  // 65 is 'A'
    mystl::tuple<char, std::string, int> t3(t_source);
    // Test implicit conversions: int->char, const char*->string, double->int
    EXPECT_EQ(mystl::get<0>(t3), 'A');
    EXPECT_EQ(mystl::get<1>(t3), "Test");
    EXPECT_EQ(mystl::get<2>(t3), -3);  // double to int truncation
  }

  // 4. Test Pair Constructor
  // Corresponds to: std::tuple<int, double> t4{std::make_pair(42, 3.14)};
  {
    mystl::pair<int, double> p = {42, 3.14};
    mystl::tuple<int, double> t4(p);
    EXPECT_EQ(mystl::get<0>(t4), 42);
    EXPECT_DOUBLE_EQ(mystl::get<1>(t4), 3.14);
  }

  // 6. Test Normal Copy Assignment
  {
    mystl::tuple<int, std::string, std::vector<int>> t1{1, "alpha", {1, 2, 3}};
    mystl::tuple<int, std::string, std::vector<int>> t2{2, "beta", {4, 5}};

    t1 = t2;

    // t1 should now be a copy of t2
    EXPECT_EQ(mystl::get<0>(t1), 2);
    EXPECT_EQ(mystl::get<1>(t1), "beta");
    EXPECT_EQ(mystl::get<2>(t1), (std::vector<int>{4, 5}));
    // t2 should be unchanged
    EXPECT_EQ(mystl::get<0>(t2), 2);
    EXPECT_EQ(mystl::get<1>(t2), "beta");
    EXPECT_EQ(mystl::get<2>(t2), (std::vector<int>{4, 5}));
  }

  // 7. Test Normal Move Assignment
  {
    mystl::tuple<int, std::string, std::vector<int>> t1{1, "alpha", {1, 2, 3}};
    mystl::tuple<int, std::string, std::vector<int>> t2{2, "beta", {4, 5}};

    t1 = std::move(t2);

    // t1 should now have the values of t2
    EXPECT_EQ(mystl::get<0>(t1), 2);
    EXPECT_EQ(mystl::get<1>(t1), "beta");
    EXPECT_EQ(mystl::get<2>(t1), (std::vector<int>{4, 5}));
    // t2 is in a valid but unspecified state. For std::string and std::vector,
    // this typically means empty. The int is trivially copyable, so it's unchanged.
    EXPECT_EQ(mystl::get<0>(t2), 2);
    EXPECT_EQ(mystl::get<1>(t2), "");
    EXPECT_TRUE(mystl::get<2>(t2).empty());
  }

  // 8. Test Converting Copy Assignment
  {
    mystl::tuple<int, std::string, std::vector<int>> t1;
    mystl::tuple<short, const char*, std::vector<int>> t3{
        3, "gamma", {6, 7, 8}};

    t1 = t3;

    EXPECT_EQ(mystl::get<0>(t1), 3);
    EXPECT_EQ(mystl::get<1>(t1), "gamma");
    EXPECT_EQ(mystl::get<2>(t1), (std::vector<int>{6, 7, 8}));
    // Source t3 should be unchanged
    EXPECT_EQ(mystl::get<0>(t3), 3);
    EXPECT_STREQ(mystl::get<1>(t3), "gamma");  // Use STREQ for const char*
  }

  // 9. Test Converting Move Assignment
  {
    mystl::tuple<int, std::string, std::vector<int>> t1;
    mystl::tuple<short, const char*, std::vector<int>> t3{
        3, "gamma", {6, 7, 8}};

    t1 = std::move(t3);

    EXPECT_EQ(mystl::get<0>(t1), 3);
    EXPECT_EQ(mystl::get<1>(t1), "gamma");
    EXPECT_EQ(mystl::get<2>(t1), (std::vector<int>{6, 7, 8}));
  }

  // 10. Test Copy Assignment from Pair
  {
    mystl::tuple<std::string, std::vector<int>> t4{"delta", {10, 11, 12}};
    mystl::pair<const char*, std::vector<int>> p1{"epsilon", {14, 15}};

    t4 = p1;

    EXPECT_EQ(mystl::get<0>(t4), "epsilon");
    EXPECT_EQ(mystl::get<1>(t4), (std::vector<int>{14, 15}));
    // Source pair p1 should be unchanged
    EXPECT_STREQ(p1.first, "epsilon");
    EXPECT_EQ(p1.second, (std::vector<int>{14, 15}));
  }

  // 11. Test Move Assignment from Pair
  {
    mystl::tuple<std::string, std::vector<int>> t4{"delta", {10, 11, 12}};
    mystl::pair<const char*, std::vector<int>> p1{"epsilon", {14, 15}};

    t4 = std::move(p1);

    EXPECT_EQ(mystl::get<0>(t4), "epsilon");
    EXPECT_EQ(mystl::get<1>(t4), (std::vector<int>{14, 15}));
    // The vector in p1 should be moved from (empty)
    EXPECT_TRUE(p1.second.empty());
  }

  {
    // --- Test make_tuple and std::ref ---
    int n = 1;
    auto t = mystl::make_tuple(10, "Test", 3.14, std::ref(n), n);

    // Initial check
    EXPECT_EQ(mystl::get<0>(t), 10);
    EXPECT_STREQ(mystl::get<1>(t), "Test");
    EXPECT_DOUBLE_EQ(mystl::get<2>(t), 3.14);
    EXPECT_EQ(mystl::get<3>(t), 1);
    EXPECT_EQ(mystl::get<4>(t), 1);

    // Modify n and check if the reference in the tuple reflects the change
    n = 7;
    EXPECT_EQ(mystl::get<3>(t), 7);  // The reference should see the new value
    EXPECT_EQ(mystl::get<4>(t), 1);  // The copy should not change

    // --- Test tie with a function return ---
    auto f = []() -> mystl::tuple<int, int> {
      return mystl::make_tuple(5, 7);
    };
    int a, b;
    mystl::tie(a, b) = f();
    EXPECT_EQ(a, 5);
    EXPECT_EQ(b, 7);
  }

  {
    // --- Test tie for lexicographical comparison ---
    struct S {
      int n;
      std::string s;
      float d;
      // Comparison operator will be defined outside the struct
    };
    // Define operator< for S at function scope
    auto s_less = [](const S& lhs, const S& rhs) noexcept {
      return mystl::tie(lhs.n, lhs.s, lhs.d) < mystl::tie(rhs.n, rhs.s, rhs.d);
    };
    std::set<S, decltype(s_less)> set_of_s(s_less);
    S value{42, "Test", 3.14};
    auto [iter, is_inserted] = set_of_s.insert(value);
    ASSERT_TRUE(is_inserted);

    // --- Test tie for unpacking and structured bindings ---
    auto position = [](int w) {
      return mystl::make_tuple(1 * w, 2 * w);
    };
    auto [x, y] = position(1);
    ASSERT_EQ(x, 1);
    ASSERT_EQ(y, 2);

    mystl::tie(x, y) = position(2);  // Reuse variables with tie
    ASSERT_EQ(x, 2);
    ASSERT_EQ(y, 4);

    // --- Test tie with implicit conversion ---
    mystl::tuple<char, short> coordinates('a', 9);
    mystl::tie(x, y) = coordinates;
    ASSERT_EQ(x, 'a');
    ASSERT_EQ(y, 9);

    // --- Test tie with std::ignore ---
    std::string z;
    mystl::tie(x, std::ignore, z) = mystl::make_tuple(1, 2.0, "Test");
    ASSERT_EQ(x, 1);
    ASSERT_EQ(z, "Test");
  }

  {
    std::map<int, std::string> m;

    // This relies on the map's emplace correctly using the tuples we provide.
    m.emplace(std::piecewise_construct, mystl::forward_as_tuple(6),
              mystl::forward_as_tuple(9, 'g'));

    ASSERT_EQ(m.size(), 1);
    EXPECT_EQ(m.at(6), std::string(9, 'g'));  // "ggggggggg"
  }

  {
    mystl::tuple<int, std::string, float> t1(10, "Test", 3.14);
    int n = 7;

    // Concatenate multiple tuples, including one created from a reference via tie
    auto t2 = mystl::tuple_cat(t1, mystl::make_tuple("Foo", "bar"), t1,
                               mystl::tie(n));

    // Change n AFTER concatenation. The reference in t2 should update.
    n = 42;

    // Check the structure and values of the concatenated tuple
    // t1 + ("Foo", "bar") + t1 + tie(n)
    // (int, string, float, const char*, const char*, int, string, float, int&)
    EXPECT_EQ(mystl::get<0>(t2), 10);
    EXPECT_EQ(mystl::get<1>(t2), "Test");
    EXPECT_FLOAT_EQ(mystl::get<2>(t2), 3.14f);
    EXPECT_STREQ(mystl::get<3>(t2), "Foo");
    EXPECT_STREQ(mystl::get<4>(t2), "bar");
    EXPECT_EQ(mystl::get<5>(t2), 10);
    EXPECT_EQ(mystl::get<6>(t2), "Test");
    EXPECT_FLOAT_EQ(mystl::get<7>(t2), 3.14f);
    EXPECT_EQ(mystl::get<8>(t2), 42);  // Check the reference
  }

  {
    auto x = mystl::make_tuple(1, "Foo", 3.14);

    // Index-based access
    EXPECT_EQ(mystl::get<0>(x), 1);
    EXPECT_STREQ(mystl::get<1>(x), "Foo");
    EXPECT_DOUBLE_EQ(mystl::get<2>(x), 3.14);

    // Type-based access (C++14 feature)
    EXPECT_EQ(mystl::get<int>(x), 1);
    EXPECT_STREQ(mystl::get<const char*>(x), "Foo");
    EXPECT_DOUBLE_EQ(mystl::get<double>(x), 3.14);

    // The ambiguity check is a compile-time error, so we can't test it directly.
    // We can only note it in a comment.
    // const mystl::tuple<int, double, double> y(1, 6.9, 9.6);
    // const double& d = mystl::get<double>(y); // This line should fail to compile.
  }

  {
    std::vector<mystl::tuple<int, std::string, float>> v{
        {2, "baz", -0.1f},
        {2, "bar", 3.14f},
        {1, "foo", 10.1f},
        {2, "baz", -1.1f},
    };

    // This call requires mystl::tuple::operator< to be correctly implemented.
    std::sort(v.begin(), v.end());

    std::vector<mystl::tuple<int, std::string, float>> expected_sorted_v{
        {1, "foo", 10.1f},
        {2, "bar", 3.14f},
        {2, "baz", -1.1f},  // -1.1 comes before -0.1
        {2, "baz", -0.1f},
    };

    // This check requires mystl::tuple::operator== to be correctly implemented.
    ASSERT_EQ(v.size(), expected_sorted_v.size());
    for (size_t i = 0; i < v.size(); ++i) {
      EXPECT_EQ(mystl::get<0>(v[i]), mystl::get<0>(expected_sorted_v[i]));
      EXPECT_EQ(mystl::get<1>(v[i]), mystl::get<1>(expected_sorted_v[i]));
      EXPECT_FLOAT_EQ(mystl::get<2>(v[i]), mystl::get<2>(expected_sorted_v[i]));
    }
  }

  {
    mystl::tuple<int, std::string, float> p1{42, "ABCD", 2.71f};
    mystl::tuple<int, std::string, float> p2{10, "1234", 3.14f};

    auto p1_original = p1;
    auto p2_original = p2;

    // Test member swap
    p1.swap(p2);
    EXPECT_EQ(mystl::get<0>(p1), mystl::get<0>(p2_original));
    EXPECT_EQ(mystl::get<0>(p2), mystl::get<0>(p1_original));

    // Test non-member swap (should swap them back)
    swap(p1, p2);  // ADL will find mystl::swap
    EXPECT_EQ(mystl::get<0>(p1), mystl::get<0>(p1_original));
    EXPECT_EQ(mystl::get<0>(p2), mystl::get<0>(p2_original));
  }
}