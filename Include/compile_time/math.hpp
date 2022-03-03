#pragma once
#include <type_traits>
#include "compile_time/constants.hpp"

namespace compile_time {

template <typename Integer>
constexpr Integer factorial(Integer n);

template <typename Real>
constexpr Real abs(Real num);

template <typename Integer>
constexpr bool is_even(Integer num);

template <typename Integer>
constexpr bool is_odd(Integer num);

template <typename Real>
constexpr bool is_negative(Real num);

template <typename Real, typename Integer>
constexpr Real pow(Real a, Integer n);

template <typename Real>
constexpr Real max(Real a, Real b);

template <typename Real, class Compare>
constexpr Real max(Real a, Real b, Compare comp);

template <typename Real>
constexpr Real min(Real a, Real b);

template <typename Real, class Compare>
constexpr Real min(Real a, Real b, Compare comp);

template <typename Real>
constexpr Real sqrt(Real x);

template <typename Real>
constexpr Real hypot(Real a, Real b);

template <typename Real>
constexpr Real sqr(Real x);

template <typename Integer>
constexpr Integer choose(Integer n, Integer k);

template <typename Integer>
constexpr Integer fibonacci(Integer iter, Integer a = 0, Integer b = 1);

template <typename Real>
constexpr Real deg2rad(Real deg);

template <typename Real>
constexpr Real rad2deg(Real rad);

template <typename Floating, typename Integer = long long>
constexpr Integer floor(Floating x);

template <typename Floating, typename Integer = long long>
constexpr Integer ceil(Floating x);

template <typename Real>
constexpr Real exp(Real x);

template <typename Real>
constexpr Real sin(Real x);

template <typename Real>
constexpr Real cos(Real x);

template <typename Real>
constexpr Real tan(Real x);

template <typename Real>
constexpr Real csc(Real x);

template <typename Real>
constexpr Real sec(Real x);

template <typename Real>
constexpr Real cot(Real x);

template <typename Real>
constexpr Real fast_atan(Real x);

template <typename Real, typename Integer = long long>
constexpr Integer ilog(Real x, Real b = constants::e);
} // compile_time

#include "detail/math/math.inl"
#include "detail/math/exp.inl"
#include "detail/math/trig.inl"