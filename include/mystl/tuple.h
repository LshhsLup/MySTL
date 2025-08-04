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

template <std::size_t I, class T>
struct TupleLeaf {
  T value{};
  constexpr TupleLeaf() = default;

  template <class U>
  constexpr TupleLeaf(U&& x) : value(std::forward<U>(x)) {}

  // get(), 方便特化 tuple 的 get
  constexpr T& get() & noexcept { return value; }

  constexpr const T& get() const& noexcept { return value; }

  constexpr T&& get() && noexcept { return std::move(value); }

  constexpr const T&& get() const&& noexcept { return std::move(value); }
};

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

// 获取 index 对应的类型
template <std::size_t Index, class... Types>
struct NthType;

template <class Head, class... Tail>
struct NthType<0, Head, Tail...> {
  using type = Head;
};

template <std::size_t Index, class Head, class... Tail>
struct NthType<Index, Head, Tail...> {
  using type = typename NthType<Index - 1, Tail...>::type;
};

template <std::size_t Index, class... Types>
using NthType_t = typename NthType<Index, Types...>::type;

// 根据类型获取 Types 中对应的索引
template <class T, class... Types>
struct TypeIndex;

template <class T, class... Tail>
struct TypeIndex<T, T, Tail...> {
  static constexpr std::size_t value = 0;
};

template <class T, class Head, class... Tail>
struct TypeIndex<T, Head, Tail...> {
  static constexpr std::size_t value = TypeIndex<T, Tail...>::value + 1;
};

template <class T, class... Types>
constexpr inline std::size_t TypeIndex_v = TypeIndex<T, Types...>::value;

// 计算 Types 中 T 出现的次数
template <class T, class... Types>
struct CountType;

template <class T>
struct CountType<T> {
  static constexpr std::size_t value = 0;
};

template <class T, class Head, class... Tail>
struct CountType<T, Head, Tail...> {
  static constexpr std::size_t value =
      (std::is_same<T, Head>::value ? 1 : 0) + CountType<T, Tail...>::value;
};

template <class T, class... Types>
constexpr inline std::size_t CountType_v = CountType<T, Types...>::value;

template <std::size_t I, class... Types>
typename std::tuple_element<I, mystl::tuple<Types...>>::type& get(
    mystl::tuple<Types...>& t) noexcept {
  using LeafType =
      TupleLeaf<I,
                typename std::tuple_element<I, mystl::tuple<Types...>>::type>;
  return static_cast<LeafType&>(t).get();
}

template <std::size_t I, class... Types>
typename std::tuple_element<I, mystl::tuple<Types...>>::type&& get(
    mystl::tuple<Types...>&& t) noexcept {
  using LeafType =
      TupleLeaf<I,
                typename std::tuple_element<I, mystl::tuple<Types...>>::type>;
  return static_cast<LeafType&&>(t).get();
}

template <std::size_t I, class... Types>
const typename std::tuple_element<I, mystl::tuple<Types...>>::type& get(
    const mystl::tuple<Types...>& t) noexcept {
  using LeafType =
      TupleLeaf<I,
                typename std::tuple_element<I, mystl::tuple<Types...>>::type>;
  return static_cast<const LeafType&>(t).get();
}

template <std::size_t I, class... Types>
const typename std::tuple_element<I, mystl::tuple<Types...>>::type&& get(
    const mystl::tuple<Types...>&& t) noexcept {
  using LeafType =
      TupleLeaf<I,
                typename std::tuple_element<I, mystl::tuple<Types...>>::type>;
  return static_cast<const LeafType&&>(t).get();
}

template <class T, class... Types>
constexpr T& get(mystl::tuple<Types...>& t) noexcept {
  static_assert(CountType_v<T, Types...> == 1,
                "get<T>(tuple): T must occur exactly once in the tuple");
  constexpr std::size_t index = TypeIndex_v<T, Types...>;
  using LeafType = TupleLeaf<
      index, typename std::tuple_element<index, mystl::tuple<Types...>>::type>;
  return static_cast<LeafType&>(t).get();
}

template <class T, class... Types>
constexpr T&& get(mystl::tuple<Types...>&& t) noexcept {
  static_assert(CountType_v<T, Types...> == 1,
                "get<T>(tuple): T must occur exactly once in the tuple");
  constexpr std::size_t index = TypeIndex_v<T, Types...>;
  using LeafType = TupleLeaf<
      index, typename std::tuple_element<index, mystl::tuple<Types...>>::type>;
  return static_cast<LeafType&&>(t).get();
}

template <class T, class... Types>
constexpr const T& get(const mystl::tuple<Types...>& t) noexcept {
  static_assert(CountType_v<T, Types...> == 1,
                "get<T>(tuple): T must occur exactly once in the tuple");
  constexpr std::size_t index = TypeIndex_v<T, Types...>;
  using LeafType = TupleLeaf<
      index, typename std::tuple_element<index, mystl::tuple<Types...>>::type>;
  return static_cast<const LeafType&>(t).get();
}

template <class T, class... Types>
constexpr const T&& get(const mystl::tuple<Types...>&& t) noexcept {
  static_assert(CountType_v<T, Types...> == 1,
                "get<T>(tuple): T must occur exactly once in the tuple");
  constexpr std::size_t index = TypeIndex_v<T, Types...>;
  using LeafType = TupleLeaf<
      index, typename std::tuple_element<index, mystl::tuple<Types...>>::type>;
  return static_cast<const LeafType&&>(t).get();
}
}  // namespace mystl

namespace std {
template <std::size_t I, class... Types>
struct tuple_element<I, mystl::tuple<Types...>> {
  using type = mystl::NthType_t<I, Types...>;
};
}  // namespace std
#endif