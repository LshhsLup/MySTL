#ifndef __MYSTL_TYPE_TRAITS_H__
#define __MYSTL_TYPE_TRAITS_H__

#include <type_traits>
#include "mystl/tuple.h"

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

// C++17 可用折叠表达式代替，简洁又方便
// is_all_true: 判断一个模板 Trait 对应的类型是否全为 true
// 对于一堆类型 Types, 判断 Trait<T> ?= true
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

// is_all_true 受限于 Trait 只能接受一个参数
// 对于 std::is_convertible<From, To> 就使用不了
// is_all_true_general 对其优化版本
// 对于两堆类型 Types, Utypes, 判断 Trait<T1, U1> ?= true
// 如果两堆类型个数不一样--> false
template <class... Types>
struct TypeLists {};

// 主模版
template <template <class, class> class Trait, class TypeLists1,
          class TypeLists2>
struct is_all_true_general;

// 终止条件, TypeLists 为空
template <template <class, class> class Trait>
struct is_all_true_general<Trait, TypeLists<>, TypeLists<>> : std::true_type {};

// 递归
template <template <class, class> class Trait, class Head1, class... Tail1,
          class Head2, class... Tail2>
struct is_all_true_general<Trait, TypeLists<Head1, Tail1...>,
                           TypeLists<Head2, Tail2...>>
    : std::conditional<
          Trait<Head1, Head2>::value,
          is_all_true_general<Trait, TypeLists<Tail1...>, TypeLists<Tail2...>>,
          std::false_type> {};

template <template <class, class> class Trait, class... Ts>
struct is_all_true_general<Trait, TypeLists<Ts...>, TypeLists<>>
    : std::false_type {};

template <template <class, class> class Trait, class... Us>
struct is_all_true_general<Trait, TypeLists<>, TypeLists<Us...>>
    : std::false_type {};

template <template <class, class> class Trait, class TypeLists1,
          class TypeLists2>
inline constexpr bool is_all_true_general_v =
    is_all_true_general<Trait, TypeLists1, TypeLists2>::value;
}  // namespace mystl

#endif