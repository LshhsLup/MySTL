#ifndef __MYSTL_VECTOR_H__
#define __MYSTL_VECTOR_H__

#include <memory>
#include "mystl/allocator.h"

namespace mystl {

template <class T, class Alloc = mystl::allocator<T>>
class vector {
 public:
  // member type
  using value_type = T;
  using allocator_type = Alloc;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using reference = value_type&;
  using const_reference = const value_type&;
  using pointer = typename std::allocator_traits<Alloc>::pointer;
  using const_pointer = typename std::allocator_traits<Alloc>::pointer;
  using iterator = T*;
  using const_iterator = const T*;
  using revrese_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

 private:
  pointer start;
  pointer finish;
  pointer end_of_storage;
};
}  // namespace mystl

#endif