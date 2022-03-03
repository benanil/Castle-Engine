#include <stdexcept>
#include "compile_time/math.hpp"

namespace compile_time {
namespace detail {

// [exp_frac_helper] and [exp_frac] are based on eqn 2.2.1 of
// link.springer.com/content/pdf/bbm%3A978-94-91216-37-4%2F1.pdf
template <typename Real>
constexpr Real exp_frac_helper(Real x2, int iter = 5, int k = 6) {
  return (iter > 0) ? k + x2 / exp_frac_helper(x2, iter - 1, k + 4)
                    : k + x2 / (k + 4);
}

template <typename Real>
constexpr Real exp_frac(Real x) {
  // compute exp(x) for -1 <= x <= 1
  return (x != 0) ? 1 + 2 * x / (2 - x + (x * x) / exp_frac_helper(x * x)) : 1;
}
}  // namespace detail

template <typename Real>
constexpr Real exp(Real x) {
  return pow(constants::e, floor(x)) * detail::exp_frac(x - floor(x));
}

template <typename Real, typename Integer>
constexpr Integer ilog(Real x, Real b) {
  // clang-format off
  return (b == 1) ? throw std::domain_error("base != 1") :
         (b <= 0) ? throw std::domain_error("base > 0")  :
         (x <= 0) ? throw std::domain_error("x > 0")     :
         (x >= b) ? ilog(x / b, b) + 1                   :
         (x < 1 ) ? ilog(b * x, b) - 1                   :
         ( true ) ? 0                                  :0;
  // clang-format on
}
}  // compile_time