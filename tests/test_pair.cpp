#include <gtest/gtest.h>
#include <algorithm>
#include <string>
#include <vector>
#include "mystl/utility.h"
#include "utils/test_types.h"

class PairTest : public ::testing::Test {
 protected:
  // 你可以在这里定义所有测试用例都能访问的对象
  // mystl::pair<int, std::string> p1{1, "hello"};
  // mystl::pair<int, std::string> p2{2, "world"};
};

using namespace mystl::test;
// ---- 测试默认构造函数 ----
// TEST(TestSuiteName, TestName)
TEST(PairTest, DefaultConstruction) {
  // --- 子测试 1: 基本类型和标准库类型 ---
  {
    mystl::pair<int, double> p_pod;
    EXPECT_EQ(p_pod.first, 0);
    EXPECT_EQ(p_pod.second, 0.0);

    mystl::pair<std::string, std::string> p_str;
    EXPECT_EQ(p_str.first, "");
    EXPECT_EQ(p_str.second, "");
  }

  // --- 子测试 2: 两个成员都是隐式默认构造 ---
  {
    Perfect::counter.reset();
    mystl::pair<Perfect, Perfect> p;
    EXPECT_EQ(Perfect::counter.default_constructor, 2);

    // 验证这种 pair 可以被隐式构造
    auto test_implicit = []() {
      mystl::pair<Perfect, RuleOfZero> p = {};  // 拷贝列表初始化
      (void)p;                                  // nothing to do
    };
    EXPECT_NO_THROW(test_implicit());

    // 编译期验证
    static_assert(
        std::is_default_constructible_v<mystl::pair<Perfect, Perfect>>,
        "Pair of two implicit should be default constructible");
    static_assert(
        mystl::is_implicitly_default_constructible_v<
            mystl::pair<Perfect, Perfect>>,
        "Pair of two implicit should be IMPLICITLY default constructible");
  }

  // --- 子测试 3: 至少一个成员是显式默认构造 ---
  {
    // 测试 case: <implicit, explicit>
    Perfect::counter.reset();
    ExplicitDefault::counter.reset();
    mystl::pair<Perfect, ExplicitDefault> p1;  // 直接初始化
    EXPECT_EQ(Perfect::counter.default_constructor, 1);
    EXPECT_EQ(ExplicitDefault::counter.default_constructor, 1);

    // 测试 case: <explicit, explicit>
    ExplicitDefault::counter.reset();
    mystl::pair<ExplicitDefault, ExplicitDefault> p2;  // 直接初始化
    EXPECT_EQ(ExplicitDefault::counter.default_constructor, 2);

    // 编译期验证：这些 pair 应该是可默认构造的，但必须是 explicit 的
    static_assert(
        std::is_default_constructible_v<mystl::pair<Perfect, ExplicitDefault>>,
        "Pair with one explicit should be default constructible");
    static_assert(!mystl::is_implicitly_default_constructible_v<
                      mystl::pair<Perfect, ExplicitDefault>>,
                  "Pair with one explicit should NOT be implicitly default "
                  "constructible");

    static_assert(std::is_default_constructible_v<
                      mystl::pair<ExplicitDefault, ExplicitDefault>>,
                  "Pair with two explicit should be default constructible");
    static_assert(!mystl::is_implicitly_default_constructible_v<
                      mystl::pair<ExplicitDefault, ExplicitDefault>>,
                  "Pair with two explicit should NOT be implicitly default "
                  "constructible");
  }

  // --- 子测试 4: 构造函数被正确禁用 ---
  {
    // 这个测试完全是编译期的，所以我们只需要 static_assert 即可。
    // 将它们放在这里是为了逻辑上的完整性。

    static_assert(
        !std::is_default_constructible_v<mystl::pair<NoDefault, int>>,
        "pair with NoDefault member should not be default constructible");

    static_assert(
        !std::is_default_constructible_v<mystl::pair<int, NoDefault>>,
        "pair with NoDefault member should not be default constructible");

    static_assert(
        !std::is_default_constructible_v<mystl::pair<NoDefault, NoDefault>>,
        "pair with two NoDefault members should not be default constructible");

    static_assert(
        !std::is_default_constructible_v<mystl::pair<ReferenceMember, int>>,
        "pair with ReferenceMember should not be default constructible");
  }
}

// pair(const T1&, const T2&)
TEST(PairTest, ValueConstructor) {
  // test1: 两个支持隐式拷贝构造函数类型
  {
    Perfect::counter.reset();
    static_assert(std::is_convertible<const Perfect&, Perfect>::value,
                  "Should support implicit copy");
    Perfect a(10);
    Perfect b(20);
    mystl::pair<Perfect, Perfect> p1 = {a, b};
    EXPECT_EQ(p1.first.value(), 10);
    EXPECT_EQ(p1.second.value(), 20);
    EXPECT_EQ(Perfect::counter.copy_constructor, 2);
  }
  // test2: 有显示拷贝构造函数类型
  {
    ExplicitCopy::counter.reset();
    Perfect::counter.reset();
    static_assert(!std::is_convertible<ExplicitCopy, ExplicitCopy>::value,
                  "ExplicitCpoy does not support implicit copy");
    ExplicitCopy a(30);
    ExplicitCopy b(40);
    Perfect c(79);
    // mystl::pair<ExplicitCopy, ExplicitCopy> p2 = {a, b}; // 编译失败
    mystl::pair<ExplicitCopy, ExplicitCopy> p2{a, b};
    // mystl::pair<Perfect, ExplicitCopy> p3 = {c , b}; // 编译失败
    mystl::pair<Perfect, ExplicitCopy> p3{c, b};
    EXPECT_EQ(p2.first.value(), 30);
    EXPECT_EQ(p2.second.value(), 40);
    EXPECT_EQ(p3.first.value(), 79);
    EXPECT_EQ(p3.second.value(), 40);
    EXPECT_EQ(ExplicitCopy::counter.value_constructor, 2);
    EXPECT_EQ(ExplicitCopy::counter.copy_constructor, 3);
    EXPECT_EQ(Perfect::counter.copy_constructor, 1);
  }
  // test3: 内置类型
  {
    mystl::pair<int, std::string> p4 = {10, "hello, world"};
    mystl::pair<std::string, double> p5{"hello, world", 100.11};
    EXPECT_EQ(p4.first, 10);
    EXPECT_EQ(p4.second, "hello, world");
    EXPECT_EQ(p5.first, "hello, world");
    EXPECT_EQ(p5.second, 100.11);
  }
  // 应禁用不可拷贝类型的构造函数
  {
    static_assert(!std::is_constructible<
                      mystl::pair<NonMovableNonCopyable, NoDefault>>::value,
                  "NonCopyable should not be constructible");
  }
}

//
TEST(PairTest, AssignOperator) {
  mystl::pair<int, std::vector<int>> p{1, {2}}, q{2, {5, 6}};
  p = q;
  EXPECT_EQ(p.first, 2);
  std::vector<int> v1{5, 6};
  EXPECT_EQ(p.second, v1);
  mystl::pair<short, std::vector<int>> r{4, {7, 8, 9}};
  p = r;
  EXPECT_EQ(p.first, 4);
  std::vector<int> v2{7, 8, 9};
  EXPECT_EQ(p.second, v2);
  p = mystl::pair<int, std::vector<int>>{3, {4}};
  p = std::move(q);
  EXPECT_EQ(p.first, 2);
  EXPECT_EQ(p.second, v1);
  EXPECT_EQ(q.first, 2);
  std::vector<int> v3{};
  EXPECT_EQ(q.second, v3);
  p = mystl::pair<int, std::vector<int>>{5, {6}};
  p = std::move(r);
  EXPECT_EQ(p.first, 4);
  EXPECT_EQ(p.second, v2);
  EXPECT_EQ(r.first, 4);
  EXPECT_EQ(r.second, v3);
  // std::cout << "hello world\n";
}

TEST(PairTest, PairSwapAndGet) {
  mystl::pair<int, std::string> p1(10, "test"), p2;
  p2.swap(p1);
  EXPECT_EQ(p2.first, 10);
  EXPECT_EQ(p2.second, "test");
  int i1 = 10, i2{};
  std::string s1("test"), s2;
  mystl::pair<int&, std::string&> r1(i1, s1), r2(i2, s2);
  r2.swap(r1);
  EXPECT_EQ(r2.first, 10);
  EXPECT_EQ(r2.second, "test");

  auto p3 = mystl::make_pair(10, 3.14);
  auto p4 = mystl::pair(12, 1.23);
  EXPECT_EQ(mystl::get<0>(p3), 10);
  EXPECT_EQ(mystl::get<1>(p3), 3.14);
  EXPECT_EQ(mystl::get<0>(p4), 12);
  EXPECT_EQ(mystl::get<1>(p4), 1.23);
  p3.swap(p4);
  EXPECT_EQ(mystl::get<0>(p3), 12);
  EXPECT_EQ(mystl::get<1>(p3), 1.23);
  EXPECT_EQ(mystl::get<0>(p4), 10);
  EXPECT_EQ(mystl::get<1>(p4), 3.14);
  mystl::swap(p3, p4);
  EXPECT_EQ(mystl::get<0>(p3), 10);
  EXPECT_EQ(mystl::get<1>(p3), 3.14);
  EXPECT_EQ(mystl::get<0>(p4), 12);
  EXPECT_EQ(mystl::get<1>(p4), 1.23);

  auto p5 = mystl::make_pair(1, 3.14);
  auto p6 =
      mystl::make_pair(std::vector<int>{1, 2, 3}, std::string("hello world"));
  std::vector<int> vec{1, 2, 3};
  EXPECT_EQ(mystl::get<0>(p5), 1);
  EXPECT_EQ(mystl::get<1>(p5), 3.14);
  EXPECT_EQ(mystl::get<int>(p5), 1);
  EXPECT_EQ(mystl::get<double>(p5), 3.14);
  EXPECT_EQ(mystl::get<0>(p6), vec);
  EXPECT_EQ(mystl::get<1>(p6), "hello world");
  EXPECT_EQ(mystl::get<std::vector<int>>(p6), vec);
  EXPECT_EQ(mystl::get<std::string>(p6), "hello world");
}

TEST(PairTest, MakePairAndCompare) {
  int n = 1;
  int a[5] = {1, 2, 3, 4, 5};
  auto p1 = mystl::make_pair(n, a[1]);
  EXPECT_EQ(p1.first, 1);
  EXPECT_EQ(p1.second, 2);
  n = 10;
  EXPECT_EQ(p1.first, 1);
  auto p2 = mystl::make_pair(std::ref(n), a);
  EXPECT_EQ(p2.first, 10);
  n = 7;
  EXPECT_EQ(p2.first, 7);
  EXPECT_EQ(*(p2.second + 2), 3);

  std::vector<mystl::pair<int, std::string>> v = {
      {2, "baz"}, {2, "bar"}, {1, "foo"}};
  std::sort(v.begin(), v.end());
  std::vector<mystl::pair<int, std::string>> v1 = {
      {1, "foo"}, {2, "bar"}, {2, "baz"}};
  EXPECT_EQ(v, v1);
}