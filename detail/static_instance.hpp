#pragma once

namespace sym::detail {

template <class T>
inline constexpr auto static_instance = T{};

}
