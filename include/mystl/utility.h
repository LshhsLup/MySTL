#ifndef __MYSTL_UTILITY_H__
#define __MYSTL_UTILITY_H__

#include <type_traits>
#include <utility>
#include "mystl/type_traits.h"

std::pair<int, int> p1;

namespace mystl {

//===========================================================
//======================    pair    =========================
//===========================================================
template <class T1, class T2>
struct pair {
  // member types
  using first_type = T1;
  using second_type = T2;

  // member object
  T1 first;
  T2 second;

  // default constructors
  // T1 和 T2 都可以默认构造的时候 pair 才能默认构造
  // T1 和 T2 都是隐式默认构造的时候 pair 才是隐式默认构造
  template <class U1 = T1, class U2 = T2,
            typename = typename std::enable_if<
                std::is_default_constructible<U1>::value &&
                    std::is_default_constructible<U2>::value &&
                    mystl::is_implicitly_default_constructible_v<U1> &&
                    mystl::is_implicitly_default_constructible_v<U2>,
                int>::type>
  constexpr pair() : first(), second() {}

  template <class U1 = T1, class U2 = T2,
            typename std::enable_if<
                std::is_default_constructible<U1>::value &&
                    std::is_default_constructible<U2>::value &&
                    (!mystl::is_implicitly_default_constructible_v<U1> ||
                     !mystl::is_implicitly_default_constructible_v<U2>),
                char>::type = 0>
  explicit constexpr pair() : first(), second() {}

  // value constructors
  // T1 和 T2 都能拷贝构造这个构造函数才会参与重载决议
  pair(const T1& x, const T2& y);

  ~pair() noexcept = default;
};
}  // namespace mystl

#endif