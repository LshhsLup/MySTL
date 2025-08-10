#ifndef __MYSTL_TUPLE_H__
#define __MYSTL_TUPLE_H__

#include <tuple>
#include <type_traits>
#include <utility>
#include "mystl/type_traits.h"
#include "mystl/utility.h"

// std::tuple<int, int, int> t1;

namespace mystl {
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

namespace detail {
template <class Tuple, class OtherTuple,
          size_t Size =
              std::tuple_size<typename std::decay<Tuple>::type>::value>
struct is_unambiguous_conversion_impl {
  static constexpr bool value = true;  // 默认对多元素 tuple 返回 true
};

template <class Tuple, class OtherTuple>
struct is_unambiguous_conversion_impl<Tuple, OtherTuple, 1> {
  using T =
      typename std::tuple_element<0, typename std::decay<Tuple>::type>::type;
  using SourceTupleType = typename std::decay<OtherTuple>::type;

  static constexpr bool is_ambiguous = std::is_convertible_v<OtherTuple, T> ||
                                       std::is_constructible_v<T, OtherTuple>;
  static constexpr bool is_same_underlying_tuple =
      std::is_same_v<typename std::decay<Tuple>::type, SourceTupleType>;

  static constexpr bool value = !is_ambiguous && !is_same_underlying_tuple;
};
}  // namespace detail

template <class Tuple, class OtherTuple>
static constexpr bool is_unambiguous_conversion_v =
    detail::is_unambiguous_conversion_impl<Tuple, OtherTuple>::value;
// tuple: 变长参数模板，无法在一个类中存储可变数量和类型的成员
// 设计思路： 继承一个包含所有元素类型的、独立的基类聚合
// 基类就是一个带索引和类型的节点
// 前向声明
template <class... Types>
class tuple;

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

// get 前向声明
template <std::size_t I, class... Types>
typename std::tuple_element<I, mystl::tuple<Types...>>::type& get(
    mystl::tuple<Types...>& t) noexcept;

template <std::size_t I, class... Types>
typename std::tuple_element<I, mystl::tuple<Types...>>::type&& get(
    mystl::tuple<Types...>&& t) noexcept;

template <std::size_t I, class... Types>
const typename std::tuple_element<I, mystl::tuple<Types...>>::type& get(
    const mystl::tuple<Types...>& t) noexcept;

template <std::size_t I, class... Types>
const typename std::tuple_element<I, mystl::tuple<Types...>>::type&& get(
    const mystl::tuple<Types...>&& t) noexcept;
  
template <class... Types>
class tuple
    : public TupleImpl<std::make_index_sequence<sizeof...(Types)>, Types...> {
 public:
  using Base = TupleImpl<std::make_index_sequence<sizeof...(Types)>, Types...>;

  // TODO: explicit constructors
  template <class Dummy>
  struct check_for_default_constructor {
    static constexpr bool isDefaultConstructible() {
      return mystl::is_all_true_v<std::is_default_constructible, Types...>;
    }
  };
  template <class Dummy = void, typename = typename std::enable_if<
                check_for_default_constructor<Dummy>::isDefaultConstructible()>::type>
  constexpr tuple() : Base() {}

  template <class Dummy>
  struct check_for_direct_constructor {
    static constexpr bool isConstructible() {
      return mystl::is_all_true_v<std::is_constructible, mystl::TypeLists<Types...>,
               mystl::TypeLists<const Types&...>>;
    }
    static constexpr bool isNonEmpty() {
      return sizeof...(Types) >= 1;
    }
  };
  template <class Dummy = void, typename = typename std::enable_if<
                check_for_direct_constructor<Dummy>::isConstructible() &&
                check_for_direct_constructor<Dummy>::isNonEmpty()>::type>
  constexpr tuple(const Types&... args) : Base(args...) {}

  template <
      class... UTypes,
      typename std::enable_if<
          // 检查参数数量是否匹配
          sizeof...(UTypes) == sizeof...(Types) &&
              // 检查是否每个元素都可以从对应的参数构造
              mystl::is_all_true_general_v<std::is_constructible,
                                           mystl::TypeLists<Types...>,
                                           mystl::TypeLists<UTypes...>> &&
              // 防止此构造函数被误用于拷贝/移动 tuple 本身
              (sizeof...(UTypes) != 1 ||
               !std::is_same<tuple<Types...>, typename std::decay<NthType_t<
                                                  0, UTypes...>>::type>::value),
          int>::type = 0>
  constexpr tuple(UTypes&&... args) : Base(std::forward<UTypes>(args)...) {}

  template <class... UTypes,
            typename std::enable_if<
                sizeof...(UTypes) == sizeof...(Types) &&
                    mystl::is_all_true_general_v<
                        std::is_constructible, mystl::TypeLists<Types...>,
                        mystl::TypeLists<const UTypes&...>> &&
                    is_unambiguous_conversion_v<mystl::tuple<Types...>,
                                                const mystl::tuple<UTypes...>&>,
                int>::type = 0>
  constexpr tuple(const tuple<UTypes...>& other)
      : tuple(other, std::make_index_sequence<sizeof...(UTypes)>{}) {}

  template <class... UTypes,
            typename std::enable_if<
                sizeof...(UTypes) == sizeof...(Types) &&
                    mystl::is_all_true_general_v<
                        std::is_constructible, mystl::TypeLists<Types...>,
                        mystl::TypeLists<UTypes&&...>> &&
                    is_unambiguous_conversion_v<mystl::tuple<Types...>,
                                                mystl::tuple<UTypes...>&&>,
                int>::type = 0>
  constexpr tuple(tuple<UTypes...>&& other)
      : tuple(std::move(other), std::make_index_sequence<sizeof...(UTypes)>{}) {
  }

  template <class U1, class U2,
            typename std::enable_if<
                sizeof...(Types) == 2 &&
                    mystl::is_all_true_general_v<
                        std::is_constructible, mystl::TypeLists<Types...>,
                        mystl::TypeLists<const U1&, const U2&>>,
                int>::type = 0>
  constexpr tuple(const mystl::pair<U1, U2>& p) : Base(p.first, p.second) {}

  template <class U1, class U2,
            typename std::enable_if<
                sizeof...(Types) == 2 &&
                    mystl::is_all_true_general_v<std::is_constructible,
                                                 mystl::TypeLists<Types...>,
                                                 mystl::TypeLists<U1&&, U2&&>>,
                int>::type = 0>
  constexpr tuple(mystl::pair<U1, U2>&& p)
      : Base(std::move(p.first), std::move(p.second)) {}

  tuple(const tuple& other) = default;
  tuple(tuple&& other) = default;

 private:
  template <class OtherTuple, std::size_t... Is>
  constexpr tuple(OtherTuple&& other, std::index_sequence<Is...>)
      : Base(mystl::get<Is>(std::forward<OtherTuple>(other))...) {}
};

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
// std::tuple_size 特化
template <class... Types>
struct tuple_size<mystl::tuple<Types...>>
    : std::integral_constant<std::size_t, sizeof...(Types)> {};

// std::tuple_element 特化
template <std::size_t I, class... Types>
struct tuple_element<I, mystl::tuple<Types...>> {
  using type = mystl::NthType_t<I, Types...>;
};
}  // namespace std
#endif