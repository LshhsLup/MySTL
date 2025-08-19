#ifndef __MYSTL_ALLOCATOR_H__
#define __MYSTL_ALLOCATOR_H__

#include <stdint.h>  // for uintptr_t
#include <cstddef>   // for std::size_t std::ptrdiff_t
#include <exception>
#include <type_traits>  // for std::true_type

namespace mystl {
/*
* The default allocator is stateless, that is, 
* all instances of the given allocator are interchangeable, 
* compare equal and can deallocate memory allocated by any 
* other instance of the same allocator type.
*/
template <class T>
struct allocator {
  // member type
  using value_type = T;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  // what is it ? determine move allocator when container assigment
  using propagate_on_container_move_assignment = std::true_type;
  using is_always_equal = std::true_type;

  // defalut allocator is stateless，
  // constructors ans deconstructor nothing to do.
  allocator() noexcept {}
  allocator(const allocator&) noexcept {}
  template <class U>
  allocator(const allocator<U>&) noexcept {}
  ~allocator() noexcept {}

  T* address(T& x) const noexcept { return static_cast<T*>(&x); }

  const T* address(const T& x) const noexcept {
    return static_cast<const T*>(&x);
  }

  size_type max_size() const noexcept { return std::size_t(-1) / sizeof(T); }

  // c++17 deprecated
  // the new memory block as new as hint
  T* allocate(std::size_t n, const void* hint = static_cast<const void*>(0)) {
    if (n > this->max_size()) {
      throw std::bad_alloc();
    }
    // for some SIMD instruction types, align > 16(long double)
    constexpr std::size_t alignment = alignof(T);
    constexpr std::size_t max_align = alignof(std::max_align_t);

#if defined(__cpp_aligned_new)
    //c++17
    if (alignment > max_align) {
      std::align_val_t __al = std::align_val_t(alignof(_Tp));
      return static_cast<_Tp*>(::operator new(__n * sizeof(_Tp), __al));
    }
#endif
    // not support c++17, select platform API
#if defined(__GUNC__) || defined(__clang__)
    // linux
    if (alignment > max_align) {
      void* p = nullptr;
      if (posix_memalign(&p, alignment, n * sizeof(T)) != 0) {
        throw std::bad_alloc();
      }
      return static_cast<T*>(p);
    }
#endif

#if defined(__MSC_VAR)
    // windows
    if (alignment > max_align) {
      return static_cast<T*>(_aligned_malloc(n * sizeof(T), alignment));
    }
#endif

    // simulate
    if (alignment > max_align) {
      // x + [0, alignment-1] can be aligned by alignment
      // eg. alignment = 16, x = 25, so padding_size = 32 - 25 = 7
      // alignment = 16, x = 33, so padding_size = 48 - 33 = 15
      // set padding_size is the maxinum to reduce computing complexity
      constexpr std::size_t padding_size = alignment - 1;
      // memory pos for raw_ptr, which is thr original alloccate ptr
      constexpr std::size_t raw_ptr_size = sizeof(void*);
      // total size
      const std::size_t total_size =
          n * sizeof(T) + padding_size + raw_ptr_size;
      char* raw_memory = static_cast<char*>(::operator new(total_size));
      /*
       ** |----- padding_size -----|----- raw_ptr -----|----- user_data -----|
       ** ^                           ^             
       ** ^raw_memory                 ^aligned_ptr
       **  (分配内存起始位置)             (找到一个对齐 alignment 的地址)                       
       **/
      // 需要将 raw_memory 地址存起来，方便 destroy
      // 先留出来存放的内存，64位机器 8 字节
      void* user_block_start = raw_memory + raw_ptr_size;
      uintptr_t aligned_addr_val =
          reinterpret_cast<uintptr_t>(user_block_start);
      // 这里运算向上对齐 alignment
      aligned_addr_val =
          aligned_addr_val +
          (alignment - (aligned_addr_val % alignment)) % alignment;

      T* aligned_ptr = reinterpret_cast<T*>(aligned_addr_val);

      // 在 aligned_ptr 前面存放 raw_memory
      void** raw_ptr = reinterpret_cast<void**>(aligned_ptr) - 1;
      *raw_ptr = raw_memory;

      return aligned_ptr;
    }
    return static_cast<T*>(::operator new(n * sizeof(T)));
  }
};
}  // namespace mystl

#endif  // __MYSTL_ALLOCATOR_H__