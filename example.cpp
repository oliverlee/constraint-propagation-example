#include "sym.hpp"

#include <iostream>

using namespace sym;

auto main() -> int
{
  // construct a symbol with a compile-time known name
  {
    constexpr auto x = "x"_symbol;
    std::cout << x << "\n";
    // symbol(x) [double: [-inf, inf]]

    static_assert(sizeof(x) == 1);
  }

  // construct a symbol with a run-time known name
  {
    const auto x = symbol{"x"};
    std::cout << x << "\n";
    // symbol(x) [double: [-inf, inf]]

    static_assert(sizeof(x) == sizeof(std::string));
  }

  // manually promote a symbol to an expression
  {
    constexpr auto x = expr("x"_symbol);
    std::cout << x << "\n";
    // expression { sym::op::identity, symbol(x) [double: [-inf, inf]] } double: [-inf, inf]

    static_assert(sizeof(x) == 1);
  }

  // construct an expression from two symbols
  {
    constexpr auto x = "x"_symbol;
    constexpr auto two_x = x + x;

    std::cout << two_x << "\n";
    // expression { sym::op::plus, expression { sym::op::identity, symbol(x) [double: [-inf, inf]] } double: [-inf, inf], expression { sym::op::identity, symbol(x) [double: [-inf, inf]] } double: [-inf, inf] } double: [-inf, inf]

    static_assert(sizeof(two_x) == 1);
  }

  // construct an expression from two symbols with compile-time known
  // constraints (same)
  {
    constexpr auto x = "x"_symbol[constraint::negative];
    constexpr auto y = "y"_symbol[constraint::negative];
    constexpr auto x_plus_y = x + y;

    std::cout << x_plus_y << "\n";
    // expression { sym::op::plus, expression { sym::op::identity, symbol(x) [double: [-inf, -4.94066e-324]] } double: [-inf, -4.94066e-324], expression { sym::op::identity, symbol(y) [double: [-inf, -4.94066e-324]] } double: [-inf, -4.94066e-324] } double: [-inf, -4.94066e-324]

    static_assert(sizeof(x_plus_y) == 1);
  }

  // construct an expression from two symbols with compile-time known
  // constraints (different)
  {
    constexpr auto x = "x"_symbol[constraint::positive];
    constexpr auto y = "y"_symbol[constraint::negative];
    constexpr auto x_plus_y = x + y;

    std::cout << x_plus_y << "\n";
    // expression { sym::op::plus, expression { sym::op::identity, symbol(x) [double: [4.94066e-324, inf]] } double: [4.94066e-324, inf], expression { sym::op::identity, symbol(y) [double: [-inf, -4.94066e-324]] } double: [-inf, -4.94066e-324] } double: [-inf, inf]

    static_assert(sizeof(x_plus_y) == 1);
  }

#if 0
  // constraint application on a symbol must be a refinement
  {
    // Assertion failed: (is_narrowing() and "constraint value does not refine existing constraint on symbol."), function operator[], file symbol.hpp, line 124.
    const auto x = "x"_symbol[constraint::positive][constraint::negative];
    constexpr auto y = "y"_symbol[constraint::negative];
    const auto x_plus_y = x + y;

    std::cout << x_plus_y << "\n";
  }
#endif

#if 0
  // expression creation validates constraint consistency
  {
    // error: static assertion failed due to requirement 'check': inconsistent symbolic constraints within expression
    const auto two_x =
        "x"_symbol[constraint::positive] + "x"_symbol[constraint::negative];

    std::cout << two_x << "\n";
  }
#endif
}
