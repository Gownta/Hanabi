#pragma once

#include <cassert>
#include <cstring>
#include <memory>

template <typename T, int N>
class small_vector {
 public:
  small_vector() {}

  void push_back(T t) {
    assert(sz_ < N);
    new (begin() + sz_++) T(std::move(t));
  }
  T pop_back() {
    assert(sz_ > 0);
    T ret = (*this)[sz_ - 1];
    sz_--;
    return ret;
  }

  int size() const {
    return sz_;
  }
  bool empty() const {
    return sz_ == 0;
  }

  void erase(int pos) {
    assert(pos >= 0);
    assert(pos < sz_);
    for (int i = pos + 1; i < sz_; ++i) {
      std::swap((*this)[i], (*this)[i - 1]);
    }
    sz_--;
    std::memset(data_ + sz_ * sizeof(T), 0, sizeof(T));
  }

  T& operator[](int i) {
    return *(begin() + i);
  }
  const T& operator[](int i) const {
    return *(begin() + i);
  }

  // Iterators
  T* begin() {
    return (T*)data_;
  }
  const T* begin() const {
    return (const T*)data_;
  }
  T* end() {
    return begin() + sz_;
  }
  const T* end() const {
    return begin() + sz_;
  }

 private:
  char data_[N * sizeof(T)];
  int sz_ = 0;
};
