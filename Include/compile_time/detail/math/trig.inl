#include "compile_time/constants.hpp"

namespace compile_time {
namespace detail {

template <typename Real>
constexpr Real wrap(Real x) {
  // standardize the angle so that -pi <= x < pi
  // clang-format off
  return (x <= -constants::pi) ? wrap(x + 2*constants::pi) :
         (x >   constants::pi) ? wrap(x - 2*constants::pi) :
         ( true              ) ? x                         : 0;
  // clang-format on
}

template <typename Real>
constexpr Real sin_cfrac(Real x2, int k = 2, int n = 40) {
  return (n == 0) ? k * (k + 1) - x2
                  : k * (k + 1) - x2 +
                        (k * (k + 1) * x2) / sin_cfrac(x2, k + 2, n - 1);
}

template <typename Real>
constexpr Real tan_cfrac(Real x2, int k = 1, int n = 40) {
  return (n == 0) ? k : k - x2 / tan_cfrac(x2, k + 2, n - 1);
}

template <typename Real>
constexpr Real fast_atan_unit(Real x) {  // -1 <= x <= 1
  return constants::pi_4 * x - x * (abs(x) - 1) * (0.2447 + 0.0663 * abs(x));
}
}  // namespace detail

template <typename Real>
constexpr Real sin(Real x) {
  return detail::wrap(x) /
         (1 + sqr(detail::wrap(x)) / detail::sin_cfrac(sqr(detail::wrap(x))));
}

template <typename Real>
constexpr Real cos(Real x) {
  return sin(constants::pi_2 - x);
}

template <typename Real>
constexpr Real tan(Real x) {
  return detail::wrap(x) / detail::tan_cfrac(sqr(detail::wrap(x)));
}

template <typename Real>
constexpr Real csc(Real x) {
  return 1.0 / sin(x);
}

template <typename Real>
constexpr Real sec(Real x) {
  return 1.0 / cos(x);
}

template <typename Real>
constexpr Real cot(Real x) {
  return 1.0 / tan(x);
}

template <typename Real>
constexpr Real fast_atan(Real x) {
  return (abs(x) <= 1) ? detail::fast_atan_unit(x)
                       : constants::pi_2 - detail::fast_atan_unit(1.0 / x);
}
}  // namespace compile_time
