#ifndef __MYSTL_VECTOR_H__
#define __MYSTL_VECTOR_H__

#include <memory>
#include <type_traits>
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
  /*============================================
  * State 1: An Empty Vector
  * size() = 0, capacity() = 0
  *
  * +-----------------------------------------+
  * |             (No Memory Allocated)       |
  * +-----------------------------------------+
  *   ^
  *   |
  * start
  * finish
  * end_of_storage
  *   (All pointers are equal, often nullptr)
  *============================================= 
  *State 2: A vector with some elements and spare capacity
  * size() = 3, capacity() = 5
  *
  * +==================================================================+
  * |                          llocated Memory                         |
  * +------------+------------+------------+-------------+-------------+
  * |  Element A |  Element B |  Element C | <uninit>    | <uninit>    |
  * +------------+------------+------------+-------------+-------------+
  * ^                                      ^                           ^
  * |                                      |                           |
  * start                                 finish                  end_of_storage
  *
  *
  *[start, finish)  => Range containing valid elements (the vector's content)
  *                       Distance: finish - start == 3
  *
  *[finish, end_of_storage) => Uninitialized but available memory for future pushes
  *
  *[start, end_of_storage) => The entire range of allocated memory (the capacity)
  *                              Distance: end_of_storage - start == 5
  *======================================================================
  * State 3: A full vector (size == capacity)
  * size() = 5, capacity() = 5
  *
  * +================================================================+
  * |                          Allocated Memory                      |
  * +------------+------------+------------+------------+------------+
  * |  Element A |  Element B |  Element C |  Element D |  Element E |
  * +------------+------------+------------+------------+------------+
  * ^                                                                ^
  * |                                                                |
  * start                                                          finish
  *                                                                end_of_storage
  * finish and end_of_storage now point to the same location.
  * Any call to push_back() will cause a reallocation.
  */
  pointer start{nullptr};
  pointer finish{nullptr};
  pointer end_of_storage{nullptr};
  Alloc allocator;

 private:
  void M_crate_storage(size_type n) {
    start = allocator.allocate(n);
    finish = start;
    end_of_storage = start + n;
  }

  template <class... Args>
  void M_construct(Args&&... args) {
    try {
      for (; finish != end_of_storage; ++finish) {
        // 调用 ::new((void*)p) T(args)...
        allocator.construct(finish, mystl::forward<Args>(args)...);
      }
    } catch (...) {
      for (auto p = start; p != finish; ++p) {
        allocator.destroy(p);
      }
      allocator.deallocate(start, size_type(end_of_storage - start));
      start = finish = end_of_storage = nullptr;
      throw;
    }
  }

  template <class InputIterator>
  void M_construct_ranges(InputIterator first, InputIterator last) {
    try {
      for (; first != last; ++first) {
        allocator.construct(finish++, *first);
      }
    } catch (...) {
      for (auto p = start; p != finish; ++p) {
        allocator.destroy(p);
      }
      allocator.deallocate(start, size_type(end_of_storage - start));
      start = finish = end_of_storage = nullptr;
      throw;
    }
  }

 public:
  //===================================================================
  //======================== constructors =============================
  //===================================================================
  // 默认构造函数，委托给 vector(const Alloc&);
  vector() noexcept(noexcept(allocator_type())) : vector(allocator_type()) {}
  explicit vector(const allocator_type& alloc) : allocator(alloc) {}

  explicit vector(size_type n, const allocator_type& alloc = allocator_type())
      : start(nullptr),
        finish(nullptr),
        end_of_storage(nullptr),
        allocator(alloc) {
    if (n == 0) {
      return;
    }
    // 先分配内存
    M_crate_storage(n);
    M_construct();
  }

  explicit vector(size_type n, const T& value,
                  const allocator_type& alloc = allocator_type())
      : start(nullptr),
        finish(nullptr),
        end_of_storage(nullptr),
        allocator(alloc) {
    if (n == 0) {
      return;
    }
    M_crate_storage(n);
    M_construct(value);
  }

  template <class InputIterator,
            typename = std::enable_if_t<std::is_convertible<
                typename std::iterator_traits<InputIterator>::iterator_category,
                std::input_iterator_tag>::value>>
  vector(InputIterator first, InputIterator last,
         const allocator_type& alloc = allocator_type())
      : allocator(alloc) {
    size_type n = std::distance(first, last);
    M_crate_storage(n);
    M_construct_ranges(first, last);
  }

  vector(const vector& other) {
    if (this != &other) {
      allocator =
          std::allocator_traits<Alloc>::select_on_container_copy_construction(
              other.get_allocator());
      size_type n = other.size();
      this->M_crate_storage(n);
      this->M_construct_ranges(other.begin(), other.end());
    }
  }

  vector(vector&& other) noexcept : allocator(mystl::move(other.allocator)) {
    this->start = other.start;
    this->finish = other.finish;
    this->end_of_storage = other.end_of_storage;
    other.start = nullptr;
    other.finish = nullptr;
    other.end_of_storage = nullptr;
  }

  vector(const vector& other, const allocator_type& alloc) : allocator(alloc) {
    if (this != &other) {
      size_type n = other.size();
      this->M_crate_storage(n);
      this->M_construct_ranges(other.begin(), other.end());
    }
  }

  vector(vector&& other, const allocator_type& alloc)
      : allocator(mystl::move(alloc)) {
    this->start = other.start;
    this->finish = other.finish;
    this->end_of_storage = other.end_of_storage;
    other.start = nullptr;
    other.finish = nullptr;
    other.end_of_storage = nullptr;
  }

  vector(std::initializer_list<T> init,
         const allocator_type& alloc = allocator_type())
      : allocator(alloc) {
    size_type n = init.size();
    this->M_crate_storage(n);
    this->M_construct_ranges(init.begin(), init.end());
  }

  //===================================================================
  //======================= elements access ===========================
  //===================================================================
  reference at(size_type pos) {
    if(pos >= this->size()) {
      throw std::out_of_range("at: pos is out of range!") 
    }
    return *(start + pos);
  }
  const_reference at(size_type pos) const {
    if(pos >= this->size()) {
      throw std::out_of_range("at: pos is out of range!") 
    }
    return *(start + pos);
  }

  reference operator[](size_type pos) {
    // if pos >= size(), ub
    return *(start + pos);
  }
  const_reference operator[](size_type pos) const {
    return *(start + pos);
  }

  reference front() {
    return *start;
  }
  const_reference front() const {
    return *start;
  }

  reference back() {
    return *(finish - 1);
  }
  const_reference back() const {
    return *(finish - 1);
  }

  T* data() {}

  const T* data() {}

  size_type size() const { return finish - start; }

  allocator_type get_allocator() const { return allocator; }
};
}  // namespace mystl

#endif