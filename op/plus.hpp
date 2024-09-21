#pragma once
#include "constraint.hpp"
#include "op/op_util.hpp"

#include <algorithm>
#include <functional>
#include <utility>

namespace sym {
namespace op {

/// plus op implementation
///
/// defines:
/// 1. addition of two values (via inheritance of `std::plus<>`)
/// 2. aggregate constraint from addition
///
struct plus : std::plus<>
{
  struct constraint
  {
    template <class Min1, class Max1, class Min2, class Max2>
    [[nodiscard]]
    static constexpr auto operator()(
        ::sym::constraint::ordered<Min1, Max1>,
        ::sym::constraint::ordered<Min2, Max2>)
        -> ::sym::constraint::ordered<
            constant<std::min(Min1::value(), Min2::value())>,
            constant<std::max(Max1::value(), Max2::value())>>
    {
      return {};
    }
  };
};

}  // namespace op

/// plus function object
///
/// example:
///
/// ~~~{.cpp}
/// plus("a"_symbol,  "b"_symbol);
/// plus(expr, "b"_symbol);
/// ~~~
///
/// note, could handle 2+ args
///
inline constexpr struct
{
  template <
      class T1,
      class T2,
      class R = op::op_invoke_result_t<op::plus, T1&&, T2&&>>
  static constexpr auto operator()(T1&& t1, T2&& t2) -> R
  {
    return op::op_invoke(
        op::plus{}, std::forward<T1>(t1), std::forward<T2>(t2));
  }
} plus{};

/// operator+ overload
///
/// example:
///
/// ~~~{.cpp}
/// "a"_symbol + "b"_symbol;
/// expr + "b"_symbol;
/// ~~~
///
template <class T1, class T2>
constexpr auto operator+(T1&& t1, T2&& t2)
    -> decltype(plus(std::forward<T1>(t1), std::forward<T2>(t2)))
{
  return plus(std::forward<T1>(t1), std::forward<T2>(t2));
}

}  // namespace sym
