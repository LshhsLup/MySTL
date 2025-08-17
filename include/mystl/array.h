#include <array>
#include <utility>

std::array<int, 10> a;
namespace mystl {
template <class T, std::size_t N>
struct array {
  T m_data[N];

  // member type
  using value_type = T;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using reference = value_type&;
  using const_reference = const value_type&;
  using pointer = value_type*;
  using const_pointer = const value_type*;
  using iterator = value_type*;
  using const_iterator = const value_type*;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  constexpr bool empty() const noexcept { return size() == 0; }

  constexpr size_type size() const noexcept { return N; }

  constexpr size_type max_size() const noexcept { return N; }

  constexpr reference at(size_type pos) {
    if (pos >= size()) {
      throw std::out_of_range("mystl::array::at");
    }
    return m_data[pos];
  }

  constexpr const_reference at(size_type pos) const {
    if (pos >= size()) {
      throw std::out_of_range("mystl::array::at");
    }
    return m_data[pos];
  }

  constexpr reference operator[](size_type pos) { return m_data[pos]; }

  constexpr const_reference operator[](size_type pos) const {
    return m_data[pos];
  }

  constexpr reference front() { return m_data[0]; }

  constexpr const_reference front() const { return m_data[0]; }

  constexpr reference back() { return m_data[size() - 1]; }

  constexpr const_reference back() const { return m_data[size() - 1]; }

  // [data(), data() + size())
  constexpr pointer data() noexcept { return m_data; }

  constexpr const_pointer data() const noexcept { return m_data; }

  constexpr iterator begin() noexcept { return iterator(data()); }

  constexpr const_iterator begin() const noexcept { return iterator(data()); }

  constexpr const_iterator cbegin() const noexcept { return iterator(data()); }

  constexpr iterator end() noexcept { return iterator(data() + size()); }

  constexpr const_iterator end() const noexcept {
    return iterator(data() + size());
  }

  constexpr const_iterator cend() const noexcept {
    return iterator(data() + size());
  }

  constexpr reverse_iterator rbegin() noexcept {
    return reverse_iterator(end());
  }

  constexpr const_reverse_iterator rbegin() const noexcept {
    return const_reverse_iterator(end());
  }

  constexpr const_reverse_iterator crbegin() const noexcept {
    return const_reverse_iterator(end());
  }

  constexpr reverse_iterator rend() noexcept {
    return reverse_iterator(begin());
  }

  constexpr const_reverse_iterator rend() const noexcept {
    return const_reverse_iterator(begin());
  }

  constexpr const_reverse_iterator crend() const noexcept {
    return const_reverse_iterator(begin());
  }

  void fill(const T& value) { std::fill_n(begin(), size(), value); }

  void swap(array& other) noexcept(std::is_nothrow_swappable<T>::value) {
    std::swap_ranges(begin(), end(), other.begin());
  }
};

template <class T, std::size_t N>
bool operator==(const mystl::array<T, N>& lhs, const mystl::array<T, N>& rhs) {
  return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <class T, std::size_t N>
bool operator!=(const mystl::array<T, N>& lhs, const mystl::array<T, N>& rhs) {
  return !(lhs == rhs);
}

template <class T, std::size_t N>
bool operator<(const mystl::array<T, N>& lhs, const mystl::array<T, N>& rhs) {
  return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(),
                                      rhs.end());
}

template <class T, std::size_t N>
bool operator>(const mystl::array<T, N>& lhs, const mystl::array<T, N>& rhs) {
  return rhs < lhs;
}

template <class T, std::size_t N>
bool operator<=(const mystl::array<T, N>& lhs, const mystl::array<T, N>& rhs) {
  return !(rhs < lhs);
}

template <class T, std::size_t N>
bool operator>=(const mystl::array<T, N>& lhs, const mystl::array<T, N>& rhs) {
  return !(lhs < rhs);
}

// get
template <std::size_t I, class T, std::size_t N>
constexpr T& get(mystl::array<T, N>& arr) {
  static_assert(I < N, "Index out of bounds");
  return arr[I];
}

template <std::size_t I, class T, std::size_t N>
constexpr const T& get(const mystl::array<T, N>& arr) {
  static_assert(I < N, "Index out of bounds");
  return arr[I];
}

template <std::size_t I, class T, std::size_t N>
constexpr T&& get(mystl::array<T, N>&& arr) {
  static_assert(I < N, "Index out of bounds");
  return std::move(arr[I]);
}

template <std::size_t I, class T, std::size_t N>
constexpr const T&& get(const mystl::array<T, N>&& arr) {
  static_assert(I < N, "Index out of bounds");
  return std::move(arr[I]);
}
}  // namespace mystl

namespace std {
template <class T, std::size_t N>
struct tuple_size<mystl::array<T, N>> : integral_constant<std::size_t, N> {};

template <class T, std::size_t N, std::size_t I>
struct tuple_element<I, mystl::array<T, N>> {
  using type = T;
};

template <class T, std::size_t N>
void swap(mystl::array<T, N>& lhs,
          mystl::array<T, N>& rhs) noexcept(noexcept(lhs.swap(rhs))) {
  lhs.swap(rhs);
}
}  // namespace std
