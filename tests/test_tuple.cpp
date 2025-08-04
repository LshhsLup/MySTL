#include <gtest/gtest.h>
#include <string>
#include <vector>
#include "mystl/tuple.h"
#include "utils/test_types.h"

TEST(TupleTest, Constructors) {
  mystl::tuple<int, std::string, std::vector<int>> t1;
  std::vector<int> v1{};
  EXPECT_EQ(mystl::get<0>(t1), 0);
  EXPECT_EQ(mystl::get<1>(t1), "");
  EXPECT_EQ(mystl::get<2>(t1), v1);
  EXPECT_EQ(mystl::get<int>(t1), 0);
  EXPECT_EQ(mystl::get<std::string>(t1), "");
  EXPECT_EQ(mystl::get<std::vector<int>>(t1), v1);
}