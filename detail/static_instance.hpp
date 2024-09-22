#pragma once

#include <concepts>

namespace sym::detail {

template <std::default_initializable T>
inline constexpr auto static_instance = T{};

}
