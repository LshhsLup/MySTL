#include <gtest/gtest.h>
#include <string>
#include <vector>
#include "mystl/tuple.h"
#include "utils/test_types.h"

TEST(TupleTest, Constructors) {
  mystl::tuple<int, std::string, std::vector<int>> t1;
}