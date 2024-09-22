#pragma once

#include "constraint.hpp"
#include "detail/static_instance.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <ostream>
#include <string>
#include <string_view>
#include <type_traits>

namespace sym {
namespace detail {

/// compile-time string constant
///
template <std::size_t N>
struct [[nodiscard]]
string_constant
{
  std::array<char, N> chars{};

  consteval string_constant(const char (&str)[N])
      : chars{[&str] {
          auto tmp = std::array<char, N>{};
          std::ranges::copy(str, tmp.begin());
          return tmp;
        }()}
  {}

  [[nodiscard]]
  constexpr operator std::string_view() const
  {
    return std::string_view{chars.begin(), chars.size()};
  }
};

template <const char* begin, std::size_t size>
struct [[nodiscard]]
string_literal
{
  [[nodiscard]]
  constexpr operator std::string_view() const
  {
    return std::string_view{begin, size};
  }
};

template <class T>
struct is_string_literal : std::false_type
{};

template <const char* begin, std::size_t size>
struct is_string_literal<string_literal<begin, size>> : std::true_type
{};

template <class T>
inline constexpr auto is_string_literal_v = is_string_literal<T>::value;

}  // namespace detail

/// crtp helper providing common `Symbol` operations dervied from basis
/// operations
///
template <class Symbol>
struct symbol_base
{
  template <class Self>
    requires std::is_same_v<Symbol, std::remove_cvref_t<Self>>
  friend auto operator<<(std::ostream& os, const Self& self) -> auto&
  {
    os << "symbol(" << self.name() << ") [" << self.constraint() << "]";
    return os;
  }
};

/// symbol class template
///
template <
    class String = std::string,
    class Constraint = std::remove_cvref_t<decltype(constraint::real)>>
class [[nodiscard]] symbol : symbol_base<symbol<String, Constraint>>
{
  [[no_unique_address]]
  String s_{};

public:
  using constraint_type = Constraint;

  static constexpr auto is_unconstrained = std::is_same<
      constraint_type,
      std::remove_cvref_t<decltype(constraint::real)>>{};

  symbol()
    requires (detail::is_string_literal_v<String>)
  = default;

  constexpr explicit symbol(String name) : s_{std::move(name)} {}

  [[nodiscard]]
  constexpr auto name() const -> std::string_view
  {
    return s_;
  }
  [[nodiscard]]
  constexpr auto constraint() const -> const constraint_type&
  {
    return detail::static_instance<constraint_type>;
  }

  template <class Refined>
  [[nodiscard]]
  constexpr auto operator[](Refined c) && -> symbol<String, Refined>
  {
    const auto is_narrowing =
        [&c1 = this->constraint(), &c2 = std::as_const(c)] {
          return c1.min() <= c2.min() and c1.max() >= c2.max();
        };

    assert(is_narrowing() and  //
           "constraint value does not refine existing constraint on symbol.");
    return symbol<String, Refined>{std::move(s_)};
  }
};

template <class String>
symbol(String) -> symbol<std::string>;

// non-owning type-erased view of a symbol
class [[nodiscard]] any_symbol_view : symbol_base<any_symbol_view>
{
  std::string_view s_;
  constraint::any_ordered c_;

public:
  using constraint_type = constraint::any_ordered;

  template <class Symbol>
  constexpr explicit any_symbol_view(const Symbol& s)
      : s_{s.name()}, c_{s.constraint()}
  {}

  [[nodiscard]]
  constexpr auto name() const -> std::string_view
  {
    return s_;
  }

  [[nodiscard]]
  constexpr auto constraint() const -> const constraint_type&
  {
    return c_;
  }
};

inline namespace literals {

template <detail::string_constant s>
consteval auto operator""_symbol()
{
  static constexpr auto sv = std::string_view{s};

  return symbol<detail::string_literal<sv.begin(), sv.size()>>{{}};
}

}  // namespace literals
}  // namespace sym
