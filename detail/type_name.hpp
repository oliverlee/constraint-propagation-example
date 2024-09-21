#pragma once

// https://stackoverflow.com/questions/81870/is-it-possible-to-print-a-variables-type-in-standard-c/64490578#64490578

// This next line assumes C++17; otherwise, replace it with
// your own string view implementation
#include <cstddef>
#include <source_location>
#include <string_view>

namespace sym::detail {

template <class T>
consteval auto type_name() -> std::string_view;

template <>
consteval auto type_name<void>() -> std::string_view
{
  return "void";
}

namespace detail {

using type_name_prober = void;

template <class T>
consteval auto wrapped_type_name() -> std::string_view
{
  return std::source_location::current().function_name();
}

consteval auto wrapped_type_name_prefix_length() -> std::size_t
{
  return wrapped_type_name<type_name_prober>().find(
      type_name<type_name_prober>());
}

consteval auto wrapped_type_name_suffix_length() -> std::size_t
{
  return wrapped_type_name<type_name_prober>().length() -
         wrapped_type_name_prefix_length() -
         type_name<type_name_prober>().length();
}

}  // namespace detail

template <typename T>
consteval auto type_name() -> std::string_view
{
  constexpr auto wrapped_name = detail::wrapped_type_name<T>();
  constexpr auto prefix_length = detail::wrapped_type_name_prefix_length();
  constexpr auto suffix_length = detail::wrapped_type_name_suffix_length();
  constexpr auto type_name_length =
      wrapped_name.length() - prefix_length - suffix_length;
  return wrapped_name.substr(prefix_length, type_name_length);
}

}  // namespace sym::detail
