#include "sym.hpp"

#include <iostream>

using namespace sym;

auto main() -> int
{
  // construct a symbol with a compile-time known name
  {
    constexpr auto x = "x"_symbol;
    std::cout << x << "\n";

    static_assert(1 == sizeof(x));
  }

  // construct a symbol with a run-time known name
  {
    const auto x = symbol{"x"};
    std::cout << x << "\n";
  }

  // manually promote a symbol to an expression
  {
    constexpr auto x = expr("x"_symbol);
    std::cout << x << "\n";
  }

  // construct an expression from two symbols
  {
    constexpr auto x = "x"_symbol;
    constexpr auto two_x = x + x;

    std::cout << two_x << "\n";
  }

  // construct an expression from two symbols with compile-time known
  // constraints (same)
  {
    constexpr auto x = "x"_symbol[constraint::negative];
    constexpr auto y = "y"_symbol[constraint::negative];
    constexpr auto x_plus_y = x + y;

    std::cout << x_plus_y << "\n";
  }

  // construct an expression from two symbols with compile-time known
  // constraints (different)
  {
    constexpr auto x = "x"_symbol[constraint::positive];
    constexpr auto y = "y"_symbol[constraint::negative];
    constexpr auto x_plus_y = x + y;

    std::cout << x_plus_y << "\n";
  }

#if 0
  // constraint application on a symbol must be a refinement
  {
    // runtime error - `constexpr auto x` results in a compile-time error
    const auto x = "x"_symbol[constraint::positive][constraint::negative];
    constexpr auto y = "y"_symbol[constraint::negative];
    const auto x_plus_y = x + y;

    std::cout << x_plus_y << "\n";
  }
#endif

#if 1
  // expression creation validates contraint consistency
  {
    // runtime error - `constexpr auto two_x` results in a compile-time error
    const auto two_x =
        "x"_symbol[constraint::positive] + "x"_symbol[constraint::negative];

    std::cout << two_x << "\n";
  }
#endif
}
