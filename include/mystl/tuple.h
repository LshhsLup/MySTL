#ifndef __MYSTL_TUPLE_H__
#define __MYSTL_TUPLE_H__

#include <tuple>
#include <type_traits>
#include <utility>
#include "mystl/type_traits.h"
#include "mystl/utility.h"

std::tuple<int, int, int> t1;

namespace mystl {
// tuple: 变长参数模板，无法在一个类中存储可变数量和类型的成员
// 设计思路： 继承一个包含所有元素类型的、独立的基类聚合
// 基类就是一个带索引和类型的节点

// 空基类优化
template <std::size_t I, class T,
          bool = std::is_empty<T>::value && !std::is_final<T>::value>
struct TupleLeaf;

template <std::size_t I, class T>
struct TupleLeaf<I, T, false> {
  T value;
  constexpr TupleLeaf() = default;

  template <class U>
  constexpr TupleLeaf(U&& x) : value(std::forward<U>(x)) {}
};

template <std::size_t I, class T>
struct TupleLeaf<I, T, true> : public T {};

template <class IndexSeqence, class... Types>
struct TupleImpl;

template <std::size_t... Is, class... Types>
struct TupleImpl<std::index_sequence<Is...>, Types...>
    : public TupleLeaf<Is, Types>... {
  constexpr TupleImpl() : TupleLeaf<Is, Types>()... {};

  template <class... Args>
  constexpr TupleImpl(Args&&... args)
      : TupleLeaf<Is, Types>(std::forward<Args>(args))... {}
};

template <class... Types>
class tuple
    : public TupleImpl<std::make_index_sequence<sizeof...(Types)>, Types...> {
 public:
  using Base = TupleImpl<std::make_index_sequence<sizeof...(Types)>, Types...>;

  template <
      typename std::enable_if<
          mystl::is_all_true_v<std::is_default_constructible, Types...> &&
              mystl::is_all_true_v<
                  mystl::is_implicitly_default_constructible_impl, Types...>,
          int>::type = 0>
  constexpr tuple() : Base() {}

  constexpr tuple(const Types&... args);

  template <class... UTypes>
  constexpr tuple(UTypes&&... args);

  template <class... UTypes>
  constexpr tuple(const tuple<UTypes...>& other);

  template <class... UTypes>
  constexpr tuple(tuple<UTypes...>&& other);

  template <class U1, class U2>
  constexpr tuple(const mystl::pair<U1, U2>& p);

  template <class U1, class U2>
  constexpr tuple(std::pair<U1, U2>&& p);

  tuple(const tuple& other) = default;
  tuple(tuple&& other) = default;
};

}  // namespace mystl

#endif