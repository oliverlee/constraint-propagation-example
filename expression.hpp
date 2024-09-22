#pragma once

#include "detail/static_instance.hpp"
#include "detail/tuple_for_each.hpp"
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

namespace detail {

template <class Args>
struct args_base
{
  static constexpr auto is_empty = false;

  [[no_unique_address]]
  Args args_;

  [[nodiscard]]
  constexpr auto args() const -> const Args&
  {
    return args_;
  }
};

template <class... Ts>
  requires (std::is_empty_v<Ts> and ...)
struct args_base<std::tuple<Ts...>>
{
  static constexpr auto is_empty = true;

  args_base() = default;

  constexpr explicit args_base(std::tuple<Ts...>) {}

  [[nodiscard]]
  constexpr auto args() const -> const std::tuple<Ts...>&
  {
    return static_instance<std::tuple<Ts...>>;
  }
};

}  // namespace detail

/// expression type
///
template <class Op, class Args, class Constraint>
class expression : detail::args_base<Args>
{
  using args_base_type = detail::args_base<Args>;

  template <template <class...> class list, class... Ts>
  static consteval auto
  determined_unconstrained(std::type_identity<list<Ts...>>)
      -> std::bool_constant<(Ts::is_unconstrained and ...)>
  {
    return {};
  }

  template <class PreconditionVisitor>
  constexpr expression(PreconditionVisitor precondition, Args args)
      : args_base_type{std::move(args)}
  {
    visit(std::ref(precondition));
    assert(
        precondition and "inconsistent symbolic constraints within expression");
  }

public:
  using op_type = Op;
  using constraint_type = Constraint;

  static constexpr auto is_unconstrained =
      determined_unconstrained(std::type_identity<Args>{});

  constexpr expression()
    requires (args_base_type::is_empty)
      : expression{
            std::conditional_t<
                is_unconstrained,
                skip_precondition_check,
                check_symbol_constraints<>>{},
            Args{}}
  {}

  constexpr explicit expression(Args args)
      : expression{
            std::conditional_t<
                is_unconstrained,
                skip_precondition_check,
                check_symbol_constraints<>>{},
            std::move(args)}
  {}

  [[nodiscard]]
  constexpr auto op() const -> const op_type&
  {
    return detail::static_instance<op_type>;
  }
  using args_base_type::args;
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
    return R{std::tuple{std::move(s)}};
  }
} expr{};

/// @}

/// obtain the expression promotion type
///
template <class T>
using sym_expr_t = std::invoke_result_t<decltype(expr), T>;

}  // namespace sym
