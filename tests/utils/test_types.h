#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>

namespace mystl {
namespace test {
// =================================================================================
// 辅助工具：一个简单的计数器来追踪构造、析构和赋值的调用次数(SMF: special member function)
// =================================================================================
struct SMF_Counter {
  int default_constructor = 0;
  int value_constructor = 0;
  int copy_constructor = 0;
  int move_constructor = 0;
  int copy_assign = 0;
  int move_assign = 0;
  int deconstructor = 0;

  void reset() { *this = {}; }
};

// =================================================================================
// ============== A. 显式定义/删除的类型 =============
// =================================================================================

//  "完美类型" - 拥有所有特殊成员函数，且行为正常
class Perfect {
 public:
  static inline SMF_Counter counter;

  Perfect() : data(0) { counter.default_constructor++; }
  explicit Perfect(int v) : data(v) { counter.value_constructor++; }
  Perfect(const Perfect& other) : data(other.data) {
    counter.copy_constructor++;
  }

  Perfect(Perfect&& other) noexcept : data(std::exchange(other.data, 0)) {
    counter.move_constructor++;
  }

  Perfect& operator=(const Perfect& other) {
    data = other.data;
    counter.copy_assign++;
    return *this;
  }

  Perfect& operator=(Perfect&& other) noexcept {
    data = std::exchange(other.data, 0);
    counter.move_assign++;
    return *this;
  }

  ~Perfect() { counter.deconstructor++; }

  int value() const { return data; }
  bool operator==(const Perfect& other) const { return data == other.data; }

 private:
  int data;
};

//  "仅可移动类型" - 删除了拷贝操作 (e.g., std::unique_ptr)
class MoveOnly {
 public:
  static inline SMF_Counter counter;

  MoveOnly() : data(new int(0)) { counter.default_constructor++; }
  explicit MoveOnly(int v) : data(new int(v)) { counter.value_constructor++; }

  MoveOnly(const MoveOnly&) = delete;
  MoveOnly& operator=(const MoveOnly&) = delete;

  MoveOnly(MoveOnly&& other) noexcept
      : data(std::exchange(other.data, nullptr)) {
    counter.move_constructor++;
  }

  MoveOnly& operator=(MoveOnly&& other) noexcept {
    delete data;
    data = std::exchange(other.data, nullptr);
    counter.move_assign++;
    return *this;
  }

  ~MoveOnly() {
    delete data;
    counter.deconstructor++;
  }

  int value() const { return data ? *data : -1; }

 private:
  int* data;
};

//  "仅可拷贝类型" - 无移动操作，移动请求会退化为拷贝
class CopyOnly {
 public:
  static inline SMF_Counter counter;

  CopyOnly() : data(0) { counter.default_constructor++; }
  explicit CopyOnly(int v) : data(v) { counter.value_constructor++; }

  CopyOnly(const CopyOnly& other) : data(other.data) {
    counter.copy_constructor++;
  }

  CopyOnly& operator=(const CopyOnly& other) {
    data = other.data;
    counter.copy_assign++;
    return *this;
  }

  ~CopyOnly() { counter.deconstructor++; }

  int value() const { return data; }

 private:
  int data;
};

//  "不可移动也不可拷贝类型" (e.g., std::mutex)
class NonMovableNonCopyable {
 public:
  NonMovableNonCopyable() = default;
  NonMovableNonCopyable(const NonMovableNonCopyable&) = delete;
  NonMovableNonCopyable& operator=(const NonMovableNonCopyable&) = delete;
  NonMovableNonCopyable(NonMovableNonCopyable&&) = delete;
  NonMovableNonCopyable& operator=(NonMovableNonCopyable&&) = delete;
  ~NonMovableNonCopyable() = default;
};

//  "无默认构造函数"
class NoDefault {
 public:
  static inline SMF_Counter counter;

  NoDefault() = delete;
  explicit NoDefault(int v) : data(v) { counter.value_constructor++; }

  NoDefault(const NoDefault& other) : data(other.data) {
    counter.copy_constructor++;
  }
  NoDefault& operator=(const NoDefault& other) {
    data = other.data;
    counter.copy_assign++;
    return *this;
  }
  ~NoDefault() { counter.deconstructor++; }

 private:
  int data;
};

// "显式默认构造函数"
class ExplicitDefault {
 public:
  static inline SMF_Counter counter;

  explicit ExplicitDefault() { counter.default_constructor++; }
  explicit ExplicitDefault(int v) { counter.value_constructor++; }

  ExplicitDefault(const ExplicitDefault&) { counter.copy_constructor++; }
  ExplicitDefault& operator=(const ExplicitDefault&) {
    counter.copy_assign++;
    return *this;
  }
  ~ExplicitDefault() { counter.deconstructor++; }

  int value() const { return data; }

 private:
  int data{};
};

// "显示拷贝构造函数"
class ExplicitCopy {
 public:
  static inline SMF_Counter counter;
  ExplicitCopy() = default;
  ExplicitCopy(int v) : data(v) { counter.value_constructor++; }
  explicit ExplicitCopy(const ExplicitCopy& other) {
    data = other.data;
    counter.copy_constructor++;
  }

  int value() const { return data; }

 private:
  int data;
};

//  "会抛异常的拷贝构造函数"
class ThrowOnCopy {
 public:
  static inline bool throw_on_copy_constructor = false;

  ThrowOnCopy() = default;
  explicit ThrowOnCopy(int) {}

  ThrowOnCopy(const ThrowOnCopy&) {
    if (throw_on_copy_constructor) {
      throw std::runtime_error("Throwing on copy construction");
    }
  }
  ThrowOnCopy(ThrowOnCopy&&) noexcept {}
  ThrowOnCopy& operator=(const ThrowOnCopy&) { return *this; }
  ThrowOnCopy& operator=(ThrowOnCopy&&) noexcept { return *this; }
};

//  "非 noexcept 的移动操作"
class NotNoexceptMove {
 public:
  NotNoexceptMove() = default;
  NotNoexceptMove(const NotNoexceptMove&) = default;
  NotNoexceptMove& operator=(const NotNoexceptMove&) = default;

  NotNoexceptMove(NotNoexceptMove&&) { /* might throw */ }
  NotNoexceptMove& operator=(NotNoexceptMove&&) { /* might throw */
    return *this;
  }
};

// =================================================================================
// ============              B. 编译器隐式生成/删除的类型                    ===========
// =================================================================================

//  "聚合体" - 简单、无用户声明的构造函数，所有成员公开
struct Aggregate {
  int i;
  double d;
};  // 编译器生成所有 SMF。可聚合初始化 T t = {1, 3.14};

// 1 "Rule of Zero 类" - 带有私有成员和构造函数
class RuleOfZero {
 public:
  RuleOfZero(int v = 0) : data(v) {}  // 提供一个默认构造
  int value() const { return data; }

 private:
  int data;
};  // 编译器生成: 拷贝/移动构造, 拷贝/移动赋值, 析构

//  "Rule of Three: 用户定义的析构函数" - 抑制移动操作生成
class CustomDestructor {
 public:
  CustomDestructor(int v = 0) : data(new int(v)) {}
  ~CustomDestructor() { delete data; }  // 用户定义的析构函数
  // 拷贝操作由编译器生成，但移动操作被抑制
 private:
  int* data;
};

//  "Rule of Five: 用户定义的拷贝操作" - 抑制移动操作生成
class CustomCopy {
 public:
  CustomCopy(int v = 0) : data(v) {}
  CustomCopy(const CustomCopy& other) : data(other.data) {}
  CustomCopy& operator=(const CustomCopy& other) {
    data = other.data;
    return *this;
  }
  // 移动操作被抑制
 private:
  int data;
};

// "带有 const 成员" - 隐式删除赋值操作
class ConstMember {
 public:
  const int data;
  ConstMember(int v = 0) : data(v) {}
};  // 编译器删除: 拷贝/移动赋值

// "带有引用成员" - 隐式删除默认构造和赋值操作
class ReferenceMember {
 public:
  int& ref;
  ReferenceMember(int& r) : ref(r) {}
};  // 编译器删除: 默认构造, 拷贝/移动赋值

// "带有 non-trivial 成员" - 属性“继承”自成员
class NonTrivialMember {
 public:
  std::string s;
};  // 所有 SMF 都会被编译器生成，但它们会调用 std::string 对应的 SMF

// "带有不可移动成员" - 移动回退到拷贝
class MemberWithNoMove {
 public:
  CopyOnly member;
};  // 移动操作会调用 CopyOnly 的拷贝操作
}  // namespace test
}  // namespace mystl