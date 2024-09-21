#pragma once

#include "detail/type_name.hpp"

#include <array>
#include <cassert>
#include <limits>
#include <ostream>
#include <type_traits>
#include <utility>

namespace sym {

/// compile time constant value
///
template <auto Value>
struct [[nodiscard]]
constant
{
  using value_type = decltype(Value);

  [[nodiscard]]
  static constexpr auto value() -> value_type
  {
    return Value;
  }

  [[nodiscard]]
  constexpr operator value_type() const
  {
    return Value;
  }
};

namespace constraint {

/// crtp helper providing common `Ordered` operations dervied from basis
/// operations
///
template <class Ordered>
class ordered_base
{
  template <class Self>
    requires std::is_same_v<Ordered, std::remove_cvref_t<Self>>
  friend auto operator<<(std::ostream& os, const Self& self) -> auto&
  {
    os << detail::type_name<typename Ordered::value_type>() << ": ["
       << self.min() << ", " << self.max() << "]";
    return os;
  }
};

/// constraint describing an ordered set (e.g. Reals)
/// Min and Max are inclusive
///
template <class Min, class Max>
class [[nodiscard]] ordered : Min, Max, ordered_base<ordered<Min, Max>>
{
public:
  using value_type =
      std::common_type_t<typename Min::value_type, typename Max::value_type>;

  ordered() = default;

  constexpr ordered(Min min, Max max) : Min{min}, Max{max}
  {
    assert(min <= max);
  }

  [[nodiscard]]
  constexpr auto min() const -> value_type
  {
    return Min::value();
  }
  [[nodiscard]]
  constexpr auto max() const -> value_type
  {
    return Max::value();
  }
};

/// `double` is used an an archetype to represent `Reals`
///
/// This may need to be modified depending on the desired fidelity of the
/// constraint system. This probably would not need to match the evaluation
/// type of an expression.
///
/// @{

using real_type = double;

/// convenience value for specifying a symbol is real
///
inline constexpr auto real = ordered{
    constant<-std::numeric_limits<real_type>::infinity()>{},
    constant<+std::numeric_limits<real_type>::infinity()>{}};

/// convenience value for specifying a symbol is negative
///
inline constexpr auto negative = ordered{
    constant<-std::numeric_limits<real_type>::infinity()>{},
    constant<real_type{} - std::numeric_limits<real_type>::denorm_min()>{},
};

/// convenience value for specifying a symbol is positive
///
inline constexpr auto positive = ordered{
    constant<real_type{} + std::numeric_limits<real_type>::denorm_min()>{},
    constant<+std::numeric_limits<real_type>::infinity()>{},
};

/// type-erased ordered constraint
///
class [[nodiscard]] any_ordered : ordered_base<any_ordered>
{
  std::array<real_type, 2> values_;

public:
  template <class Ordered>
  constexpr explicit any_ordered(const Ordered& c) : values_{c.min(), c.max()}
  {}

  [[nodiscard]]
  constexpr auto min() const -> real_type
  {
    return std::get<0>(values_);
  }
  [[nodiscard]]
  constexpr auto max() const -> real_type
  {
    return std::get<1>(values_);
  }

  [[nodiscard]]
  constexpr friend auto
  operator==(const any_ordered& lhs, const any_ordered& rhs) -> bool
  {
    return lhs.min() == rhs.min() and lhs.max() == rhs.max();
  }
};

/// @}

}  // namespace constraint
}  // namespace sym
