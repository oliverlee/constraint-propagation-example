#pragma once

#include <functional>
#include <type_traits>
#include <utility>
namespace sym::op {

/// idenitty op implementation
///
/// defines:
/// 1. indentity of one value (via inheritance of `std::indentity`)
/// 2. propagated constraint from identity (i.e. the same constrait)
///
struct identity : std::identity
{
  struct constraint
  {
    template <class T>
    [[nodiscard]]
    static constexpr auto operator()(T&& t) -> std::remove_cvref_t<T>
    {
      return std::forward<T>(t);
    }
  };
};

}  // namespace sym::op
