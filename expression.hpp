#pragma once

#include "detail/static_instance.hpp"
#include "detail/tuple_for_each.hpp"
#include "is_unconstrained.hpp"
#include "op/identity.hpp"
#include "symbol.hpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <functional>
#include <iostream>
#include <ostream>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace sym {

/// expression precondition visitors
///
/// Stateful function objects that collect information when visiting all
/// symbols in an expression tree.
///
/// These visitors define two functions:
/// 1. visit function on `symbol`s
/// 2. conversion to `bool`. returns `false` on precondition violation
///
/// @{

struct skip_precondition_check
{
  constexpr auto operator()(const auto&) const -> void {}

  [[nodiscard]]
  constexpr operator bool() const
  {
    return true;
  }
};

template <class Container = std::vector<any_symbol_view>>
struct check_symbol_constraints
{
  Container symbols{};

  template <class... Ts>
  constexpr auto operator()(const symbol<Ts...>& s)
  {
    symbols.emplace_back(s);
  }

  [[nodiscard]]
  constexpr operator bool()
  {
    std::ranges::sort(symbols, std::ranges::less{}, &any_symbol_view::name);

    const auto it =
        std::ranges::adjacent_find(symbols, [](const auto& s1, const auto& s2) {
          return s1.name() == s2.name() and s1.constraint() != s2.constraint();
        });

    return it == symbols.cend();
  }
};

/// @}

/// expression type
///
template <class Op, class Args, class Constraint>
class expression
{
  [[no_unique_address]]
  Args args_;

  static_assert(
      std::tuple_size_v<Args> != 0,
      "`Args` must be a specialization of `std::tuple`");

  template <class PreconditionVisitor>
  constexpr expression(PreconditionVisitor precondition, Args args)
      : args_{std::move(args)}
  {
    visit(std::ref(precondition));
    assert(
        precondition and "inconsistent symbolic constraints within expression");
  }

public:
  using op_type = Op;
  using constraint_type = Constraint;

  constexpr explicit expression(Args args)
      : expression{
            std::conditional_t<
                is_unconstrained<expression>::value,
                skip_precondition_check,
                check_symbol_constraints<>>{},
            std::move(args)}
  {}

  [[nodiscard]]
  constexpr auto op() const -> const op_type&
  {
    return detail::static_instance<op_type>;
  }
  [[nodiscard]]
  constexpr auto args() const -> const Args&
  {
    return args_;
  }
  [[nodiscard]]
  constexpr auto constraint() const -> const constraint_type&
  {
    return detail::static_instance<constraint_type>;
  }

  template <class Visitor>
  constexpr auto visit(Visitor v) const
  {
    detail::tuple_for_each(args(), [v]<class T>(const T& element) {
      if constexpr (requires { element.visit(v); }) {
        element.visit(v);
      } else if constexpr (std::is_invocable_v<Visitor, const T&>) {
        v(element);
      } else {
        static_assert(
            false, "element must define `visit` or be visitor invocable");
      }
    });
  }

  friend auto operator<<(std::ostream& os, const expression& ex) -> auto&
  {
    os << "expression { " << detail::type_name<op_type>();

    [&]<std::size_t... Is>(std::index_sequence<Is...>) {
      auto _ = ((os << ", " << std::get<Is>(ex.args()), 0) + ...);
    }(std::make_index_sequence<std::tuple_size_v<Args>>{});

    os << " } ";
    os << ex.constraint();
    return os;
  }
};

/// determines if an expression is unconstained
///
/// true if all symbols are unconstrained, otherwise false
///
template <class Op, class... Ts, class Constraint>
struct is_unconstrained<expression<Op, std::tuple<Ts...>, Constraint>>
    : std::bool_constant<(is_unconstrained<Ts>::value and ...)>
{};

/// expression promotion
///
/// if value is an expression, return that expression
/// if value is a symbol, promote to an expression using the identity operation
///
/// @{

inline constexpr struct
{
  template <class... Ts>
  [[nodiscard]]
  static constexpr auto operator()(expression<Ts...> ex) -> expression<Ts...>
  {
    return std::move(ex);
  }
  template <
      class... Ts,
      class R = expression<
          op::identity,
          std::tuple<symbol<Ts...>>,
          typename symbol<Ts...>::constraint_type>>
  [[nodiscard]]
  static constexpr auto operator()(symbol<Ts...> s) -> R
  {
    return R{std::tuple{s}};
  }
} expr{};

/// @}

/// obtain the expression promotion type
///
template <class T>
using sym_expr_t = std::invoke_result_t<decltype(expr), T>;

}  // namespace sym
