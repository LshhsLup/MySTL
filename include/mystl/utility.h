#ifndef __MYSTL_UTILITY_H__
#define __MYSTL_UTILITY_H__

#include <tuple>  // for tuple_element
#include <type_traits>
#include <utility>
#include "mystl/type_traits.h"

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
  // T1 和 T2 都支持隐式拷贝构造 pair 的这个构造函数在支持隐式构造
  // std::is_convertible<const U1&, U1>::value 值为 true 说明 U1 支持隐式拷贝构造(U(const U&))
  template <
      class U1 = T1, class U2 = T2,
      typename std::enable_if<std::is_copy_constructible<U1>::value &&
                                  std::is_copy_constructible<U2>::value &&
                                  std::is_convertible<const U1&, U1>::value &&
                                  std::is_convertible<const U2&, U2>::value,
                              int>::type = 0>
  constexpr pair(const T1& x, const T2& y) : first(x), second(y) {}

  template <class U1 = T1, class U2 = T2,
            typename = typename std::enable_if<
                std::is_copy_constructible<U1>::value &&
                    std::is_copy_constructible<U2>::value &&
                    (!std::is_convertible<const U1&, U1>::value ||
                     !std::is_convertible<const U2&, U2>::value),
                char>::type>
  explicit constexpr pair(const T1& x, const T2& y) : first(x), second(y) {}

  template <class U1, class U2,
            typename = typename std::enable_if<
                std::is_constructible<T1, U1>::value &&
                    std::is_constructible<T2, U2>::value &&
                    std::is_convertible<U1, T1>::value &&
                    std::is_convertible<U2, T2>::value,
                int>::type>
  constexpr pair(U1&& x, U2&& y)
      : first(std::forward<U1>(x)), second(std::forward<U2>(y)) {}

  template <class U1, class U2,
            typename std::enable_if<std::is_constructible<T1, U1>::value &&
                                        std::is_constructible<T2, U2>::value &&
                                        (!std::is_convertible<U1, T1>::value ||
                                         !std::is_convertible<U2, T2>::value),
                                    char>::type = 0>
  explicit constexpr pair(U1&& x, U2&& y)
      : first(std::forward<U1>(x)), second(std::forward<U2>(y)) {}

  template <class U1, class U2,
            typename = typename std::enable_if<
                std::is_constructible<T1, const U1&>::value &&
                    std::is_constructible<T2, const U2&>::value &&
                    std::is_convertible<const U1&, T1>::value &&
                    std::is_convertible<const U2&, T2>::value,
                int>::type>
  constexpr pair(const pair<U1, U2>& p) : first(p.first), second(p.second) {}

  template <
      class U1, class U2,
      typename std::enable_if<std::is_constructible<T1, const U1&>::value &&
                                  std::is_constructible<T2, const U2&>::value &&
                                  (!std::is_convertible<const U1&, T1>::value ||
                                   !std::is_convertible<const U2&, T2>::value),
                              char>::type = 0>
  explicit constexpr pair(const pair<U1, U2>& p)
      : first(p.first), second(p.second) {}

  template <class U1, class U2,
            typename = typename std::enable_if<
                std::is_constructible<T1, U1>::value &&
                    std::is_constructible<T2, U2>::value &&
                    std::is_convertible<U1, T1>::value &&
                    std::is_convertible<U2, T2>::value,
                int>::type>
  constexpr pair(pair<U1, U2>&& p)
      : first(std::forward<U1>(p.first)), second(std::forward<U2>(p.second)) {}

  template <class U1, class U2,
            typename std::enable_if<std::is_constructible<T1, U1>::value &&
                                        std::is_constructible<T2, U2>::value &&
                                        (!std::is_convertible<U1, T1>::value ||
                                         !std::is_convertible<U2, T2>::value),
                                    char>::type = 0>
  explicit constexpr pair(pair<U1, U2>&& p)
      : first(std::forward<U1>(p.first)), second(std::forward<U2>(p.second)) {}

  pair(const pair&) = default;
  pair(pair&&) = default;

  //   template <
  //       class U1 = T1, class U2 = T2,
  //       typename = typename std::enable_if<std::is_copy_assignable<U1>::value &&
  //                                              std::is_copy_assignable<U2>::value,
  //                                          int>::type>
  // typename std::enable_if<std::is_copy_assignable<T1>::value &&
  //                             std::is_copy_assignable<T2>::value,
  //                         pair&>::type
  pair& operator=(
      typename std::conditional<std::is_copy_assignable<T1>::value &&
                                    std::is_copy_assignable<T2>::value,
                                const pair&, const nonsuch&>::type other) {
    first = other.first;
    second = other.second;
    return *this;
  }

  template <
      class U1, class U2,
      typename std::enable_if<std::is_assignable<T1&, const U1&>::value &&
                                  std::is_assignable<T2&, const U2&>::value,
                              int>::type = 0>
  pair& operator=(const pair<U1, U2>& other) {
    first = other.first;
    second = other.second;
    return *this;
  }

  template <class U1 = T1, class U2 = T2,
            typename = typename std::enable_if<
                std::is_move_assignable<U1>::value &&
                std::is_move_assignable<U2>::value>::type>
  pair& operator=(pair&& other) noexcept(
      std::is_nothrow_move_assignable<U1>::value&&
          std::is_nothrow_move_assignable<U2>::value) {
    first = std::forward<first_type>(other.first);
    second = std::forward<second_type>(other.second);
    return *this;
  }

  template <class U1, class U2,
            typename = typename std::enable_if<
                std::is_assignable<T1&, U1&&>::value &&
                std::is_assignable<T2&, U2&&>::value>::type>
  pair& operator=(pair<U1, U2>&& other) {
    first = std::forward<U1>(other.first);
    second = std::forward<U2>(other.second);
    return *this;
  }

  void swap(pair& other) {
    using std::swap;
    swap(first, other.first);
    swap(second, other.second);
  }

  ~pair() = default;
};

/* 
 * Given types std::decay<T1>::type as U1 and std::decay<T2>::type as U2, the types V1
 * and V2 are defined as follows: 
 * If U1 is std::reference_wrapper<X>, V1 is X&; otherwise V1 is U1.
 * If U2 is std::reference_wrapper<Y>, V2 is Y&; otherwise V2 is U2.
 */
template <class T>
struct unwrap_reference_wrapper {
  using type = T;
};

template <class T>
struct unwrap_reference_wrapper<std::reference_wrapper<T>> {
  using type = T&;
};

template <class T1, class T2>
pair<typename unwrap_reference_wrapper<typename std::decay<T1>::type>::type,
     typename unwrap_reference_wrapper<typename std::decay<T2>::type>::type>
make_pair(T1&& x, T2&& y) {
  using U1 = typename std::decay<T1>::type;
  using U2 = typename std::decay<T2>::type;
  using V1 = typename unwrap_reference_wrapper<U1>::type;
  using V2 = typename unwrap_reference_wrapper<U2>::type;
  return pair<U1, U2>(std::forward<T1>(x), std::forward<T2>(y));
}

template <class T1, class T2, class U1, class U2>
constexpr bool operator==(const mystl::pair<T1, T2>& lhs,
                          const mystl::pair<U1, U2>& rhs) {
  return (lhs.first == rhs.first) && (lhs.second == rhs.second);
}

template <class T1, class T2, class U1, class U2>
constexpr bool operator!=(const mystl::pair<T1, T2>& lhs,
                          const mystl::pair<U1, U2>& rhs) {
  return !(lhs == rhs);
}

template <class T1, class T2, class U1, class U2>
constexpr bool operator<(const mystl::pair<T1, T2>& lhs,
                         const mystl::pair<U1, U2>& rhs) {
  return lhs.first < rhs.first ||
         (!(rhs.first < lhs.first) && (lhs.second < rhs.second));
}

template <class T1, class T2, class U1, class U2>
constexpr bool operator<=(const mystl::pair<T1, T2>& lhs,
                          const mystl::pair<U1, U2>& rhs) {
  return !(rhs < lhs);
}

template <class T1, class T2, class U1, class U2>
constexpr bool operator>(const mystl::pair<T1, T2>& lhs,
                         const mystl::pair<U1, U2>& rhs) {
  return rhs < lhs;
}

template <class T1, class T2, class U1, class U2>
constexpr bool operator>=(const mystl::pair<T1, T2>& lhs,
                          const mystl::pair<U1, U2>& rhs) {
  return !(lhs < rhs);
}

template <std::size_t I>
struct pair_get;

template <>
struct pair_get<0> {
  template <class T1, class T2>
  static constexpr T1& get(pair<T1, T2>& p) noexcept {
    return p.first;
  }

  template <class T1, class T2>
  static constexpr T1&& move_get(pair<T1, T2>&& p) noexcept {
    return std::forward<T1>(p.first);
  }

  template <class T1, class T2>
  static constexpr const T1& const_get(const pair<T1, T2>& p) noexcept {
    return p.first;
  }

  template <class T1, class T2>
  static constexpr const T1&& const_move_get(const pair<T1, T2>&& p) noexcept {
    return std::forward<T1>(p.first);
  }
};

template <>
struct pair_get<1> {
  template <class T1, class T2>
  static constexpr T2& get(pair<T1, T2>& p) noexcept {
    return p.second;
  }

  template <class T1, class T2>
  static constexpr T2&& move_get(pair<T1, T2>&& p) noexcept {
    return std::forward<T2>(p.second);
  }

  template <class T1, class T2>
  static constexpr const T2& const_get(const pair<T1, T2>& p) noexcept {
    return p.second;
  }

  template <class T1, class T2>
  static constexpr const T2&& const_move_get(const pair<T1, T2>&& p) noexcept {
    return std::forward<T2>(p.second);
  }
};

template <std::size_t I, class T1, class T2>
constexpr typename std::tuple_element<I, pair<T1, T2>>::type& get(
    pair<T1, T2>& p) noexcept {
  return pair_get<I>::get(p);
}

template <std::size_t I, class T1, class T2>
constexpr const typename std::tuple_element<I, pair<T1, T2>>::type& get(
    const pair<T1, T2>& p) noexcept {
  return pair_get<I>::const_get(p);
}

template <std::size_t I, class T1, class T2>
constexpr typename std::tuple_element<I, pair<T1, T2>>::type&& get(
    pair<T1, T2>&& p) noexcept {
  // p 的类型是右值引用，但是 p 有名字，所以它本身是左值，需要重新转换为右值
  return pair_get<I>::move_get(std::move(p));
}

template <std::size_t I, class T1, class T2>
constexpr const typename std::tuple_element<I, pair<T1, T2>>::type&& get(
    const pair<T1, T2>&& p) noexcept {
  return pair_get<I>::const_move_get(std::move(p));
}

template <class T1, class T2>
constexpr T1& get(pair<T1, T2>& p) noexcept {
  return p.first;
}

template <class T1, class T2>
constexpr const T1& get(const pair<T1, T2>& p) noexcept {
  return p.first;
}

template <class T1, class T2>
constexpr T1&& get(pair<T1, T2>&& p) noexcept {
  return std::move(p.first);
}

template <class T1, class T2>
constexpr const T1&& get(const pair<T1, T2>&& p) noexcept {
  return std::move(p.first);
}

template <class T1, class T2>
constexpr T1& get(pair<T2, T1>& p) noexcept {
  return p.second;
}

template <class T1, class T2>
constexpr const T1& get(const pair<T2, T1>& p) noexcept {
  return p.second;
}

template <class T1, class T2>
constexpr T1&& get(pair<T2, T1>&& p) noexcept {
  return std::move(p.second);
}

template <class T1, class T2>
constexpr const T1&& get(const pair<T2, T1>&& p) noexcept {
  return std::move(p.second);
}

}  // namespace mystl

namespace std {

// specialize tuple_element
template <std::size_t I, class T1, class T2>
struct tuple_element<I, mystl::pair<T1, T2>> {
  static_assert(I < 2, "mystl::pair has only two elements!!");
};

template <class T1, class T2>
struct tuple_element<0, mystl::pair<T1, T2>> {
  using type = T1;
};

template <class T1, class T2>
struct tuple_element<1, mystl::pair<T1, T2>> {
  using type = T2;
};

// specialize tuple_size
template <class T1, class T2>
struct tuple_size<mystl::pair<T1, T2>>
    : std::integral_constant<std::size_t, 2> {};

// specialize std::swap
template <class T1, class T2>
void swap(pair<T1, T2>& x, pair<T1, T2>& y) noexcept(noexcept(x.swap(y))) {
  x.swap(y);
}
}  // namespace std
#endif