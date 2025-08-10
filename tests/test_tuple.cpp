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
    EXPECT_EQ(b, 20); // b 未被修改

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