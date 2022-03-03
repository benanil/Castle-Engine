#include <type_traits>
#include "compile_time/constants.hpp"

namespace compile_time {
namespace detail {

template <typename Real>
constexpr bool is_close(Real x, Real y) {
  return abs(x - y) <= 1e-7;
}

template <typename Real>
constexpr Real sqrt_newton(Real x, Real guess) {
  return is_close(guess * guess, x)
             ? guess
             : sqrt_newton(x, (guess + x / guess) / static_cast<Real>(2));
}

template <typename Integer>
constexpr Integer range_product(Integer start, Integer end) {
  return (start > end) ? 1 : start * range_product(start + 1, end);
}

}  // namespace detail

template <typename Integer>
constexpr Integer factorial(Integer n) {
  static_assert(std::is_integral<Integer>::value,
                "factorial is defined only for non-negative integers");
  // clang-format off
  return (n < 0) ? throw std::domain_error("n >= 0") :
         (n > 0) ? n * factorial(n - 1) : 1;
  // clang-format on
}

template <typename Real>
constexpr Real abs(Real num) {
  return (num < 0) ? -num : num;
}

template <typename Integer>
constexpr bool is_even(Integer num) {
  static_assert(std::is_integral<Integer>::value,
                "is_even is defined only for integer types");
  return num % 2 == 0;
}

template <typename Integer>
constexpr bool is_odd(Integer num) {
  static_assert(std::is_integral<Integer>::value,
                "is_odd is defined only for integer types");
  return num % 2 != 0;
}

template <typename Real>
constexpr bool is_negative(Real num) {
  return num < 0;
}

template <typename Real, typename Integer>
constexpr Real pow(Real a, Integer n) {
  static_assert(std::is_integral<Integer>::value,
                "pow supports only integral powers");
  // clang-format off
  return (  n <  0  ) ? 1 / pow(a, -n)              :
         (  n == 0  ) ? 1                           :
         (  n == 1  ) ? a                           :
         (  a == 2  ) ? 1LL << n                    :
         (is_even(n)) ? pow(a * a, n / 2)           :
         (   true   ) ? a * pow(a * a, (n - 1) / 2) : 0;
  // clang-format on
}

template <typename Real>
constexpr Real max(Real a, Real b) {
  return (a < b) ? b : a;
}

template <typename Real, class Compare>
constexpr Real max(Real a, Real b, Compare comp) {
  return comp(a, b) ? b : a;
}

template <typename Real>
constexpr Real min(Real a, Real b) {
  return (a < b) ? a : b;
}

template <typename Real, class Compare>
constexpr Real min(Real a, Real b, Compare comp) {
  return comp(a, b) ? a : b;
}

template <typename Real>
constexpr Real sqrt(Real x) {
  return (x < 0) ? throw std::domain_error("x >= 0")
                 : detail::sqrt_newton(x, x);
}

template <typename Real>
constexpr Real hypot(Real a, Real b) {
  return detail::sqrt_newton(a * a + b * b, abs(a) + abs(b));
}

template <typename Real>
constexpr Real sqr(Real x) {
  return x * x;
}

template <typename Integer>
constexpr Integer choose(Integer n, Integer k) {
  static_assert(std::is_integral<Integer>::value,
                "choose works only for integral arguments");
  // clang-format off
  return (n < 0) ? throw std::domain_error("n >= 0") :
         (k < 0) ? throw std::domain_error("k >= 0") :
         (k > n) ? throw std::domain_error("k <= n") :
                   detail::range_product(max(n - k, k) + 1, n) / factorial(min(n - k, k));
  // clang-format on
}

template <typename Integer>
constexpr Integer fibonacci(Integer iter, Integer a, Integer b) {
  static_assert(std::is_integral<Integer>::value,
                "fibonacci is defined only for integral inputs");
  return (iter <= 0) ? throw std::domain_error("n > 0")
                     : (iter == 1) ? b : fibonacci(iter - 1, b, a + b);
}

template <typename Real>
constexpr Real deg2rad(Real deg) {
  return deg * (constants::pi / 180);
}

template <typename Real>
constexpr Real rad2deg(Real rad) {
  return rad * (180 / constants::pi);
}

template <typename Floating, typename Integer>
constexpr Integer floor(Floating x) {
  static_assert(std::is_floating_point<Floating>::value,
                "floor accepts only floating point inputs");
  return static_cast<Integer>(x) - (static_cast<Integer>(x) > x);
}

template <typename Floating, typename Integer>
constexpr Integer ceil(Floating x) {
  static_assert(std::is_floating_point<Floating>::value,
                "floor accepts only floating point inputs");
  return static_cast<Integer>(x) + (static_cast<Integer>(x) < x);
}

}  // namespace compile_time
