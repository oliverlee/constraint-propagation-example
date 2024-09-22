#pragma once

#include "expression.hpp"

#include <tuple>
#include <type_traits>

namespace sym::op {

/// specifies the resulting `expression` type from applying `Op` to `Args...`
///
template <class Op, class... Args>
using op_invoke_result_t = expression<
    Op,
    std::tuple<sym_expr_t<Args>...>,
    std::invoke_result_t<
        typename Op::constraint,
        typename std::remove_cvref_t<Args>::constraint_type...>>;

/// function object to simplify operation application
///
/// handles promotion from `symbol` to `expression` and determins the resulting
/// aggregate constraint type from the operation.
///
inline constexpr struct
{
  template <class Op, class... Args>
  static constexpr auto
  operator()(Op, Args&&... args) -> op_invoke_result_t<Op, Args&&...>
  {
    return op_invoke_result_t<Op, Args&&...>{
        std::tuple{expr(std::forward<Args>(args))...}};
  }
} op_invoke{};

}  // namespace sym::op
