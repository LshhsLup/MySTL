#include <gtest/gtest.h>
#include <algorithm>
#include <string>
#include <vector>
#include "mystl/utility.h"
#include "utils/test_types.h"

class PairTest : public ::testing::Test {
 protected:
  // 可以在这里定义所有测试用例都能访问的对象
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

TEST(PairTest, MemberTypesAndTraits) {
  using MyPair = mystl::pair<int, std::string>;

  // 检查内嵌类型定义是否正确
  static_assert(std::is_same_v<MyPair::first_type, int>,
                "first_type should be int");
  static_assert(std::is_same_v<MyPair::second_type, std::string>,
                "second_type should be std::string");

  // 检查 tuple_size 和 tuple_element 特化是否生效
  // 这是让 get<I> 工作的关键
  static_assert(std::tuple_size<MyPair>::value == 2,
                "tuple_size of pair should be 2");
  static_assert(std::is_same_v<std::tuple_element_t<0, MyPair>, int>,
                "tuple_element<0> should be int");
  static_assert(std::is_same_v<std::tuple_element_t<1, MyPair>, std::string>,
                "tuple_element<1> should be std::string");
}

TEST(PairTest, Constructors) {
  // 1. 默认构造函数
  {
    mystl::pair<int, double> p;
    EXPECT_EQ(p.first, 0);
    EXPECT_EQ(p.second, 0.0);

    // 测试包含复杂类型的默认构造
    mystl::pair<std::string, std::vector<int>> p_complex;
    EXPECT_EQ(p_complex.first, "");
    EXPECT_TRUE(p_complex.second.empty());
  }

  // 2. 显式默认构造函数测试
  {
    // explicit 构造函数不能使用 {} 初始化
    // mystl::pair<int, ExplicitDefaultCtor> p = {}; // 这应该无法编译通过
    mystl::pair<int, ExplicitDefault> p;  // 这样可以
    EXPECT_EQ(p.first, 0);
    EXPECT_EQ(p.second.value(), 0);
  }

  // 3. 值初始化构造函数
  {
    mystl::pair<int, std::string> p(42, "hello");
    EXPECT_EQ(p.first, 42);
    EXPECT_EQ(p.second, "hello");
  }

  // 4. 隐式转换构造函数
  {
    // 从 int, const char* 转换到 long, std::string
    mystl::pair<long, std::string> p = {123, "world"};
    EXPECT_EQ(p.first, 123L);
    EXPECT_EQ(p.second, "world");
  }

  // 5. 显式转换构造函数
  {
    mystl::pair<ExplicitCopy, int> p1(ExplicitCopy(5), 10);
    // mystl::pair<ExplicitCopyCtor, int> p2 = p1; // 应该无法编译
    mystl::pair<ExplicitCopy, int> p2(p1);  // 必须显式调用
    EXPECT_EQ(p2.first.value(), 5);
    EXPECT_EQ(p2.second, 10);
  }

  // 6. 拷贝构造函数
  {
    mystl::pair<int, std::string> p1(1, "one");
    mystl::pair<int, std::string> p2(p1);
    EXPECT_EQ(p1.first, p2.first);
    EXPECT_EQ(p1.second, p2.second);
  }

  // 7. 移动构造函数 (使用 move-only 类型测试)
  {
    auto uptr = std::make_unique<int>(100);
    mystl::pair<std::unique_ptr<int>, int> p1(std::move(uptr), 200);

    EXPECT_NE(p1.first, nullptr);
    EXPECT_EQ(*p1.first, 100);
    EXPECT_EQ(p1.second, 200);

    mystl::pair<std::unique_ptr<int>, int> p2(std::move(p1));

    // 检查 p2 是否接管了资源
    EXPECT_NE(p2.first, nullptr);
    EXPECT_EQ(*p2.first, 100);
    EXPECT_EQ(p2.second, 200);

    // 检查 p1 的资源是否已被移走
    EXPECT_EQ(p1.first, nullptr);
  }

  // 8. 拷贝转换构造函数
  {
    mystl::pair<int, const char*> p1(10, "ten");
    mystl::pair<long, std::string> p2(p1);
    EXPECT_EQ(p2.first, 10L);
    EXPECT_EQ(p2.second, "ten");
  }

  // 9. 移动转换构造函数
  {
    mystl::pair<std::unique_ptr<int>, std::string> p1(std::make_unique<int>(10),
                                                      "ten");
    mystl::pair<std::unique_ptr<int>, std::string> p2(std::move(p1));

    EXPECT_NE(p2.first, nullptr);
    EXPECT_EQ(p2.second, "ten");
    EXPECT_EQ(p1.first, nullptr);
  }
}

TEST(PairTest, Assignment) {
  // 1. 拷贝赋值
  {
    mystl::pair<int, std::string> p1(1, "one");
    mystl::pair<int, std::string> p2;
    p2 = p1;
    EXPECT_EQ(p1.first, p2.first);
    EXPECT_EQ(p1.second, p2.second);
  }

  // 2. 移动赋值 (使用 move-only 类型)
  {
    mystl::pair<std::unique_ptr<int>, int> p1(std::make_unique<int>(10), 1);
    mystl::pair<std::unique_ptr<int>, int> p2;
    p2 = std::move(p1);

    EXPECT_NE(p2.first, nullptr);
    EXPECT_EQ(*p2.first, 10);
    EXPECT_EQ(p1.first, nullptr);
  }

  // 3. 拷贝转换赋值
  {
    mystl::pair<int, const char*> p1(99, "ninety-nine");
    mystl::pair<long, std::string> p2;
    p2 = p1;
    EXPECT_EQ(p2.first, 99L);
    EXPECT_EQ(p2.second, "ninety-nine");
  }

  // 4. 移动转换赋值
  {
    mystl::pair<std::unique_ptr<int>, std::string> p1(std::make_unique<int>(10),
                                                      "ten");
    mystl::pair<std::unique_ptr<int>, std::string> p2;
    p2 = std::move(p1);

    EXPECT_NE(p2.first, nullptr);
    EXPECT_EQ(p2.second, "ten");
    EXPECT_EQ(p1.first, nullptr);
  }
}

TEST(PairTest, ComparisonOperators) {
  mystl::pair<int, std::string> p1(1, "a");
  mystl::pair<int, std::string> p2(1, "b");
  mystl::pair<int, std::string> p3(2, "a");
  mystl::pair<int, std::string> p1_copy(1, "a");

  // operator==
  EXPECT_TRUE(p1 == p1_copy);
  EXPECT_FALSE(p1 == p2);
  EXPECT_FALSE(p1 == p3);

  // operator!=
  EXPECT_FALSE(p1 != p1_copy);
  EXPECT_TRUE(p1 != p2);

  // operator< (Lexicographical comparison)
  EXPECT_TRUE(p1 < p2);  // a < b
  EXPECT_TRUE(p1 < p3);  // 1 < 2
  EXPECT_FALSE(p3 < p1);

  // operator<=
  EXPECT_TRUE(p1 <= p1_copy);
  EXPECT_TRUE(p1 <= p2);
  EXPECT_FALSE(p3 <= p1);

  // operator>
  EXPECT_TRUE(p3 > p1);
  EXPECT_TRUE(p2 > p1);
  EXPECT_FALSE(p1 > p3);

  // operator>=
  EXPECT_TRUE(p1 >= p1_copy);
  EXPECT_TRUE(p3 >= p1);
  EXPECT_FALSE(p1 >= p3);
}

TEST(PairTest, Swap) {
  mystl::pair<int, std::string> p1(1, "one");
  mystl::pair<int, std::string> p2(2, "two");

  // Member swap
  p1.swap(p2);
  EXPECT_EQ(p1.first, 2);
  EXPECT_EQ(p1.second, "two");
  EXPECT_EQ(p2.first, 1);
  EXPECT_EQ(p2.second, "one");

  // Non-member swap
  std::swap(p1, p2);
  EXPECT_EQ(p1.first, 1);
  EXPECT_EQ(p1.second, "one");
  EXPECT_EQ(p2.first, 2);
  EXPECT_EQ(p2.second, "two");
}

TEST(PairTest, MakePair) {
  int i = 10;
  const char* c_str = "hello";

  // 1. Basic usage
  auto p1 = mystl::make_pair(i, c_str);
  static_assert(std::is_same_v<decltype(p1), mystl::pair<int, const char*>>,
                "make_pair type mismatch");
  EXPECT_EQ(p1.first, 10);
  EXPECT_STREQ(p1.second, "hello");

  // 2. With std::ref to create a pair with a reference member
  auto p2 = mystl::make_pair(std::ref(i), std::string("world"));
  static_assert(std::is_same_v<decltype(p2), mystl::pair<int&, std::string>>,
                "make_pair with ref failed");
  EXPECT_EQ(p2.first, 10);

  i = 20;                   // Modify original variable
  EXPECT_EQ(p2.first, 20);  // Check if reference reflects the change
}

TEST(PairTest, Get) {
  mystl::pair<int, std::string> p(42, "test");
  const mystl::pair<double, char> const_p(3.14, 'c');

  // 1. Indexed get (lvalue)
  EXPECT_EQ(mystl::get<0>(p), 42);
  EXPECT_EQ(mystl::get<1>(p), "test");

  // Modify through get
  mystl::get<0>(p) = 99;
  EXPECT_EQ(p.first, 99);

  // 2. Indexed get (const lvalue)
  EXPECT_EQ(mystl::get<0>(const_p), 3.14);
  EXPECT_EQ(mystl::get<1>(const_p), 'c');
  // mystl::get<0>(const_p) = 1.0; // Should fail to compile

  // 3. Indexed get (rvalue)
  auto moved_str = mystl::get<1>(mystl::pair<int, std::string>(1, "move me"));
  EXPECT_EQ(moved_str, "move me");

  // 4. Typed get (lvalue)
  EXPECT_EQ(mystl::get<int>(p), 99);
  EXPECT_EQ(mystl::get<std::string>(p), "test");

  // 5. Typed get (const lvalue)
  EXPECT_EQ(mystl::get<double>(const_p), 3.14);
  EXPECT_EQ(mystl::get<char>(const_p), 'c');

  // Note: get by type will be ambiguous if types are the same
  // mystl::pair<int, int> p_same(1,2);
  // mystl::get<int>(p_same); // Should be a compilation error
}

// all tests cases below form cppreference example of pair
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
  std::swap(p3, p4);
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