#pragma once
#include <cstddef>
#include <stdexcept>

namespace compile_time {
namespace detail {

class string {
 public:
  const char* p;
  std::size_t size;

  template <std::size_t N>
  constexpr string(const char (&a)[N]) : p(a), size(N - 1) {}

  constexpr char operator[](std::size_t n) const {
    return n <= size ? p[n]
                     : throw std::out_of_range("string index out of bounds");
  }
};
}
}