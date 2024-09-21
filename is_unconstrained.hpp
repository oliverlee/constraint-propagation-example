#pragma once

#include <type_traits>

namespace sym {

/// determine if types are unconstrained
///
template <class... Ts>
struct is_unconstrained
    : std::bool_constant<(is_unconstrained<Ts>::value and ...)>
{};

/// undefined boolean value is used with zero types
template <>
struct is_unconstrained<>
{};

}  // namespace sym
