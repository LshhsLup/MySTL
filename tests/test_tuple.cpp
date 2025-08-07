#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>
#include "mystl/tuple.h"
#include "utils/test_types.h"

TEST(TupleTest, DefaultConstructors) {
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
}

//   EXPECT_EQ(mystl::get<0>(t_dest1), 99);
//   EXPECT_EQ(mystl::get<1>(t_dest1), "test source");
//   // 源 tuple 保持不变
//   EXPECT_EQ(mystl::get<0>(t_source1), 99L);

//   // 2. 从 rvalue tuple 转换 (应为移动转换)
//   mystl::tuple<int, std::string> t_dest2(
//       mystl::tuple<long, const char*>(88L, "another test"));

//   EXPECT_EQ(mystl::get<0>(t_dest2), 88);
//   EXPECT_EQ(mystl::get<1>(t_dest2), "another test");

//   // 3. 使用 move-only 类型测试 rvalue 转换构造
//   auto source_ptr = std::make_unique<int>(100);
//   mystl::tuple<std::unique_ptr<int>> t_source3(std::move(source_ptr));
//   // 从 tuple<unique_ptr<int>> 移动构造成 tuple<unique_ptr<const int>>
//   mystl::tuple<std::unique_ptr<const int>> t_dest3(std::move(t_source3));

//   ASSERT_NE(mystl::get<0>(t_dest3), nullptr);
//   EXPECT_EQ(*mystl::get<0>(t_dest3), 100);
//   // 验证源 unique_ptr 已被移走
//   EXPECT_EQ(mystl::get<0>(t_source3), nullptr);

//   // 4. 测试单元素元组的转换规则
//   mystl::tuple<long> single_source(12345L);
//   mystl::tuple<int> single_dest(single_source);  // 转换构造
//   EXPECT_EQ(mystl::get<0>(single_dest), 12345);

//   mystl::tuple<int> single_source_same(99);
//   mystl::tuple<int> single_dest_same(
//       single_source_same);  // 应该调用拷贝构造，而不是转换构造
//   EXPECT_EQ(mystl::get<0>(single_dest_same), 99);
// }

// // 测试：从 pair 构造元组
// TEST(TupleTest, PairConstructors) {
//   // 1. 从 const lvalue pair 构造 (拷贝转换)
//   mystl::pair<int, std::string> p1(200, "from pair");
//   mystl::tuple<long, std::string> t1(p1);

//   EXPECT_EQ(mystl::get<0>(t1), 200L);
//   EXPECT_EQ(mystl::get<1>(t1), "from pair");
//   // 原始 pair 不变
//   EXPECT_EQ(p1.first, 200);

//   // 2. 从 rvalue std::pair 构造 (移动转换)
//   mystl::tuple<int, std::unique_ptr<int>> t2(
//       mystl::make_pair(300, std::make_unique<int>(400)));

//   EXPECT_EQ(mystl::get<0>(t2), 300);
//   ASSERT_NE(mystl::get<1>(t2), nullptr);
//   EXPECT_EQ(*mystl::get<1>(t2), 400);

//   // 3. 从 rvalue mystl::pair 构造 (移动转换)
//   mystl::pair<int, std::unique_ptr<int>> p3(500, std::make_unique<int>(600));
//   mystl::tuple<long, std::unique_ptr<int>> t3(std::move(p3));

//   EXPECT_EQ(mystl::get<0>(t3), 500L);
//   ASSERT_NE(mystl::get<1>(t3), nullptr);
//   EXPECT_EQ(*mystl::get<1>(t3), 600);
//   // 验证原始 pair 的资源已被移走
//   EXPECT_EQ(p3.second, nullptr);
// }