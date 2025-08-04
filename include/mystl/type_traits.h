#ifndef __MYSTL_TYPE_TRAITS_H__
#define __MYSTL_TYPE_TRAITS_H__

#include <type_traits>

namespace mystl {
// is_implicitly_default_constructible
// impl: 如果 T 支持隐式构造，T v = {} 合法；反之不合法
// 1. 假设 T 支持隐式默认构造，那么 decltype(test_implicitly_construction<T>({}) = void
//  std::void_t<decltype(test_implicitly_construction<T>({})) 类型参数有效，也就是 void
// 第二个模版被特化成功，里边的 value 是 true
// 2. T 不支持隐式默认构造， decltype(test_implicitly_construction<T>({}) 类型无效
// 第二个模版特化匹配不上，被丢弃，SFINAE，实例化第一个模版，vlaue 是 false
template <class T>
void test_implicitly_construction(const T&);

template <class T, class = void>
struct is_implicitly_default_constructible_impl : std::false_type {};

template <class T>
struct is_implicitly_default_constructible_impl<
    T, std::void_t<decltype(test_implicitly_construction<T>({}))>>
    : std::true_type {};

template <class T>
inline constexpr bool is_implicitly_default_constructible_v =
    is_implicitly_default_constructible_impl<T>::value;

// type: nonsuch
struct nonsuch {
  nonsuch() = delete;
  nonsuch(const nonsuch&) = delete;
  nonsuch& operator=(const nonsuch&) = delete;
  ~nonsuch() = delete;
};

// is_all_true: 判断一个模板 Trait 对应的类型是否全为 true
// 主模板
template <template <class> class Trait, class... Types>
struct is_all_true;

// 特化，递归终止条件，所有类型都判断完都是正确的
template <template <class> class Trait>
struct is_all_true<Trait> : std::true_type {};

// 特化，递归地判断某个类型是否满足 Trait
template <template <class> class Trait, class Head, class... Tail>
struct is_all_true<Trait, Head, Tail...>
    : std::conditional<Trait<Head>::value, is_all_true<Trait, Tail...>,
                       std::false_type>::type {};

template <template <class> class Trait, class... Types>
inline constexpr bool is_all_true_v = is_all_true<Trait, Types...>::value;
}  // namespace mystl

#endif