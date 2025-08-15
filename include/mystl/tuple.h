#ifndef __MYSTL_TUPLE_H__
#define __MYSTL_TUPLE_H__

#include <initializer_list>
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
  template <class Dummy = void,
            typename = typename std::enable_if<check_for_default_constructor<
                Dummy>::isDefaultConstructible()>::type>
  constexpr tuple() : Base() {}

  template <class Dummy>
  struct check_for_direct_constructor {
    static constexpr bool isConstructible() {
      return mystl::is_all_true_general_v<std::is_constructible,
                                          mystl::TypeLists<Types...>,
                                          mystl::TypeLists<const Types&...>>;
    }
    static constexpr bool isNonEmpty() { return sizeof...(Types) >= 1; }
  };
  template <class Dummy = void,
            typename std::enable_if<
                check_for_direct_constructor<Dummy>::isConstructible() &&
                    check_for_direct_constructor<Dummy>::isNonEmpty(),
                bool>::type = true>
  constexpr tuple(const Types&... args) : Base(args...) {}

  template <
      class... UTypes,
      typename std::enable_if<
          // 检查参数数量是否匹配
          sizeof...(UTypes) >= 1 && sizeof...(UTypes) == sizeof...(Types) &&
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

  // assignment operators
  /*
   * 下面的写法一个关键的边界情况下会失败：当一个 tuple 的元素类型既不可拷贝赋值，也不可移动赋值时。
   * 一个典型的例子是 mystl::tie 创建的对 const 变量的引用元组，例如 mystl::tuple<const int&>。
   *
   * 在这种情况下：
   * 1. 对于拷贝赋值运算符：
   *    - std::is_copy_assignable<const int&>::value 为 false。
   *    - std::conditional 的结果是 mystl::nonsuch。
   *    - 运算符的签名变为：tuple& operator=(const mystl::nonsuch&);
   *
   * 2. 对于移动赋值运算符：
   *    - std::is_move_assignable<const int&>::value 也为 false。
   *    - std::conditional 的结果同样是 mystl::nonsuch。
   *    - 运算符的签名也变为：tuple& operator=(const mystl::nonsuch&);
   *
   * 结果：
   * 两个运算符都被 SFINAE 机制转换成了具有完全相同的函数签名。这在 C++ 中是不允许的，
   * 导致了编译器报告“函数重载冲突 (cannot be overloaded)”的错误。
  */
  // tuple& operator=(typename std::conditional<
  //                  mystl::is_all_true_v<std::is_copy_assignable, Types...>,
  //                  const tuple&, const nonsuch&>::type other) {
  //   assign_from(other, std::make_index_sequence<sizeof...(Types)>{});
  //   return *this;
  // }

  // tuple& operator=(
  //     typename std::conditional<
  //         mystl::is_all_true_v<std::is_move_assignable, Types...>, tuple&&,
  //         const nonsuch&>::type
  //         other) noexcept(mystl::is_all_true_v<std::is_nothrow_move_assignable,
  //                                              Types...>) {
  //   assign_from(std::move(other), std::make_index_sequence<sizeof...(Types)>{});
  //   return *this;
  // }
  template <bool B = mystl::is_all_true_v<std::is_copy_assignable, Types...>,
            typename std::enable_if_t<B, int> = 0>
  tuple& operator=(const tuple& other) {
    assign_from(other, std::make_index_sequence<sizeof...(Types)>{});
    return *this;
  }

  template <bool B = mystl::is_all_true_v<std::is_copy_assignable, Types...>,
            typename std::enable_if_t<!B, int> = 0>
  tuple& operator=(const tuple& other) = delete;

  template <bool B = mystl::is_all_true_v<std::is_move_assignable, Types...>,
            typename std::enable_if_t<B, int> = 0>
  tuple& operator=(tuple&& other) {
    assign_from(std::move(other), std::make_index_sequence<sizeof...(Types)>{});
    return *this;
  }

  template <bool B = mystl::is_all_true_v<std::is_move_assignable, Types...>,
            typename std::enable_if_t<!B, int> = 0>
  tuple& operator=(tuple&& other) = delete;

  template <class... UTypes,
            typename std::enable_if<
                sizeof...(UTypes) == sizeof...(Types) &&
                    mystl::is_all_true_general_v<
                        std::is_assignable, mystl::TypeLists<const Types&...>,
                        mystl::TypeLists<const UTypes&...>>,
                int>::type = 0>
  tuple& operator=(const tuple<UTypes...>& other) {
    assign_from(other, std::make_index_sequence<sizeof...(Types)>{});
    return *this;
  }

  template <class... UTypes,
            typename std::enable_if<
                sizeof...(UTypes) == sizeof...(Types) &&
                    mystl::is_all_true_general_v<std::is_assignable,
                                                 mystl::TypeLists<Types&...>,
                                                 mystl::TypeLists<UTypes...>>,
                int>::type = 0>
  tuple& operator=(tuple<UTypes...>&& other) {
    assign_from(std::move(other), std::make_index_sequence<sizeof...(Types)>{});
    return *this;
  }

  template <class E1, class E2,
            typename std::enable_if<
                sizeof...(Types) == 2 &&
                    mystl::is_all_true_general_v<
                        std::is_assignable, mystl::TypeLists<Types&...>,
                        mystl::TypeLists<const E1&, const E2&>>,
                int>::type = 0>
  tuple& operator=(const mystl::pair<E1, E2>& p) {
    mystl::get<0>(*this) = p.first;
    mystl::get<1>(*this) = p.second;
    return *this;
  }

  template <class E1, class E2,
            typename std::enable_if<
                sizeof...(Types) == 2 &&
                    mystl::is_all_true_general_v<std::is_assignable,
                                                 mystl::TypeLists<Types&...>,
                                                 mystl::TypeLists<E1, E2>>,
                int>::type = 0>
  tuple& operator=(mystl::pair<E1, E2>&& p) {
    mystl::get<0>(*this) = std::move(p.first);
    mystl::get<1>(*this) = std::move(p.second);
    return *this;
  }

  // swap
  void swap(tuple& other) noexcept {
    swap_elements(other, std::make_index_sequence<sizeof...(Types)>{});
  }

 private:
  template <class OtherTuple, std::size_t... Is>
  constexpr tuple(OtherTuple&& other, std::index_sequence<Is...>)
      : Base(mystl::get<Is>(std::forward<OtherTuple>(other))...) {}

  template <class OtherTuple, std::size_t... Is>
  void assign_from(OtherTuple&& other, std::index_sequence<Is...>) {
    // C++11/14 not support fold expression
    (void)(std::initializer_list<int>{(
        mystl::get<Is>(*this) = mystl::get<Is>(std::forward<OtherTuple>(other)),
        0)...});
    // C++17 可用折叠表达式代替
    // (mystl::get<Is>(*this) = mystl::get<Is>(std::forward<OtherTuple>(other)), ...);
  }

  template <std::size_t... Is>
  void swap_elements(tuple& other, std::index_sequence<Is...>) noexcept {
    using std::swap;
    // C++11/14 not support fold expression
    (void)(std::initializer_list<int>{
        (swap(mystl::get<Is>(*this), mystl::get<Is>(other)), 0)...});
    // C++17 可用折叠表达式代替
    // (std::swap(mystl::get<Is>(*this), mystl::get<Is>(other)), ...);
  }
};

template <template <class> class Func, class Tuple>
struct transform_tuple_types;

template <template <class> class Func, class... Types>
struct transform_tuple_types<Func, mystl::tuple<Types...>> {
  using type = mystl::tuple<typename Func<Types>::type...>;
};

template <template <class> class Func, class Tuple>
using transform_tuple_types_t =
    typename transform_tuple_types<Func, Tuple>::type;

template <class... Types>
using decay_tuple = transform_tuple_types_t<std::decay, mystl::tuple<Types...>>;

template <class... Types>
using return_types_tuple =
    transform_tuple_types_t<mystl::unwrap_reference_wrapper,
                            decay_tuple<Types...>>;
// make_tuple
template <class... Types>
return_types_tuple<Types...> make_tuple(Types&&... args) {
  return return_types_tuple<Types...>(std::forward<Types>(args)...);
}

// ignore
struct ignore_t {
  template <class T>
  constexpr const ignore_t& operator=(T&&) const noexcept {
    return *this;
  }
};
inline constexpr ignore_t ignore{};

// tie
template <class... Types>
mystl::tuple<Types&...> tie(Types&... args) {
  return mystl::tuple<Types&...>(args...);
}

//  compare operators
template <std::size_t I, class... Types, class... UTypes>
constexpr typename std::enable_if<I == sizeof...(Types), bool>::type
is_equal_impl(const mystl::tuple<Types...>& lhs,
              const mystl::tuple<UTypes...>& rhs) {
  return true;  // 所有元素都已比较完
}

template <std::size_t I, class... Types, class... UTypes>
    constexpr typename std::enable_if <
    I<sizeof...(Types), bool>::type is_equal_impl(
        const mystl::tuple<Types...>& lhs, const mystl::tuple<UTypes...>& rhs) {
  if (mystl::get<I>(lhs) != mystl::get<I>(rhs)) {
    return false;  // 当前元素不相等，返回 false
  }
  return is_equal_impl<I + 1>(lhs, rhs);  // 递归比较
}

template <class... TTypes, class... UTypes>
bool operator==(const mystl::tuple<TTypes...>& lhs,
                const mystl::tuple<UTypes...>& rhs) {
  return is_equal_impl<0>(lhs, rhs);
}

template <class... TTypes, class... UTypes>
bool operator!=(const mystl::tuple<TTypes...>& lhs,
                const mystl::tuple<UTypes...>& rhs) {
  return !(lhs == rhs);
}

template <std::size_t I, class... Types, class... UTypes>
constexpr typename std::enable_if<I == sizeof...(Types), bool>::type
is_less_impl(const mystl::tuple<Types...>& lhs,
             const mystl::tuple<UTypes...>& rhs) {
  return false;  // 所有元素都已比较完，lhs 不小于 rhs
}

template <std::size_t I, class... Types, class... UTypes>
    constexpr typename std::enable_if <
    I<sizeof...(Types), bool>::type is_less_impl(
        const mystl::tuple<Types...>& lhs, const mystl::tuple<UTypes...>& rhs) {
  if (mystl::get<I>(lhs) < mystl::get<I>(rhs)) {
    return true;  // 当前元素小于 rhs，返回 true
  } else if (mystl::get<I>(rhs) < mystl::get<I>(lhs)) {
    return false;  // 当前元素大于 lhs，返回 false
  }
  return is_less_impl<I + 1>(lhs, rhs);  // 递归比较
}

template <class... TTypes, class... UTypes>
bool operator<(const mystl::tuple<TTypes...>& lhs,
               const mystl::tuple<UTypes...>& rhs) {
  if constexpr (sizeof...(TTypes) == 0) {
    return false;  // 空 tuple 不小于任何 tuple
  }
  return is_less_impl<0>(lhs, rhs);
}

template <class... TTypes, class... UTypes>
bool operator<=(const mystl::tuple<TTypes...>& lhs,
                const mystl::tuple<UTypes...>& rhs) {
  return !(rhs < lhs);
}

template <class... TTypes, class... UTypes>
bool operator>(const mystl::tuple<TTypes...>& lhs,
               const mystl::tuple<UTypes...>& rhs) {
  return rhs < lhs;
}

template <class... TTypes, class... UTypes>
bool operator>=(const mystl::tuple<TTypes...>& lhs,
                const mystl::tuple<UTypes...>& rhs) {
  return !(lhs < rhs);
}

// forward_as_tuple
template <class... Types>
mystl::tuple<Types&&...> forward_as_tuple(Types&&... args) noexcept {
  return mystl::tuple<Types&&...>(std::forward<Types>(args)...);
}

// 合并两个 tuple
template <class T1, class T2>
struct TupleCatHelper;

template <class... Ts1, class... Ts2>
struct TupleCatHelper<mystl::tuple<Ts1...>, mystl::tuple<Ts2...>> {
  using type = mystl::tuple<Ts1..., Ts2...>;
};

// 递归的合并
template <class... Tuples>
struct TupleCatRValue;

template <class Tuple>
struct TupleCatRValue<Tuple> {
  using type = Tuple;
};

template <class Tuple1, class Tuple2, class... Rest>
struct TupleCatRValue<Tuple1, Tuple2, Rest...> {
  using type =
      typename TupleCatRValue<typename TupleCatHelper<Tuple1, Tuple2>::type,
                              Rest...>::type;
};

template <class... Tuples>
using TupleCatRValue_t = typename TupleCatRValue<Tuples...>::type;

// 合并两个 tuple
template <class Tuple1, class Tuple2, std::size_t... Is1, std::size_t... Is2>
auto tuple_cat_two_impl(Tuple1&& t1, Tuple2&& t2, std::index_sequence<Is1...>,
                        std::index_sequence<Is2...>) {
  using result_type =
      TupleCatRValue_t<std::decay_t<Tuple1>, std::decay_t<Tuple2>>;
  return result_type(mystl::get<Is1>(std::forward<Tuple1>(t1))...,
                     mystl::get<Is2>(std::forward<Tuple2>(t2))...);
}

template <class Tuple1, class Tuple2>
auto tuple_cat_two_impl(Tuple1&& t1, Tuple2&& t2) {
  return tuple_cat_two_impl(
      std::forward<Tuple1>(t1), std::forward<Tuple2>(t2),
      std::make_index_sequence<std::tuple_size<std::decay_t<Tuple1>>::value>{},
      std::make_index_sequence<std::tuple_size<std::decay_t<Tuple2>>::value>{});
}

template <class Tuple>
auto tuple_cat_impl(Tuple&& tuple) {
  return std::forward<Tuple>(tuple);
}

// 递归合并多个 tuple
template <class Tuple1, class Tuple2, class... Rest>
auto tuple_cat_impl(Tuple1&& t1, Tuple2&& t2, Rest&&... rest) {
  auto first_two_cat =
      tuple_cat_two_impl(std::forward<Tuple1>(t1), std::forward<Tuple2>(t2));
  if constexpr (sizeof...(Rest) == 0) {
    return first_two_cat;
  } else {
    return tuple_cat_impl(std::move(first_two_cat),
                          std::forward<Rest>(rest)...);
  }
}

// tuple_cat
template <class... Tuples>
TupleCatRValue_t<std::decay_t<Tuples>...> tuple_cat(Tuples&&... tuples) {
  return tuple_cat_impl(std::forward<Tuples>(tuples)...);
}

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

// std::swap 特化
template <class... Types>
void swap(mystl::tuple<Types...>& lhs, mystl::tuple<Types...>& rhs) noexcept {
  lhs.swap(rhs);
}
}  // namespace std
#endif