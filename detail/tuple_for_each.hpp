#pragma once

#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>
namespace sym::detail {

/// invokes a function object with every element of a tuple
///
inline constexpr struct
{
  template <class Tuple, class F>
  static constexpr auto operator()(Tuple&& tup, F f)
  {
    const auto seq = std::make_index_sequence<
        std::tuple_size_v<std::remove_cvref_t<Tuple>>>{};

    []<std::size_t... Is, class T>(
        std::index_sequence<Is...>, T&& tup, auto f) {
      std::ignore = ((f(std::get<Is>(std::forward<T>(tup))), 0) + ...);
    }(seq, std::forward<Tuple>(tup), f);
  }
} tuple_for_each{};

}  // namespace sym::detail
