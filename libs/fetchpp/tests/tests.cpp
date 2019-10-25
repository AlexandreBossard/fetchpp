#include <catch2/catch.hpp>

#include <boost/beast/core/buffers_to_string.hpp>
#include <fetchpp/fetchpp.hpp>

#include <fmt/core.h>
#include <fmt/format.h>
#include <iostream>

namespace boost
{
fmt::string_view to_string_view(boost::string_view const& v)
{
  return fmt::string_view{v.data(), v.length()};
}
}
namespace fmt
{
template <>
struct formatter<boost::beast::multi_buffer>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext& ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(boost::beast::multi_buffer const& b, FormatContext& ctx)
  {
    auto const v = boost::beast::buffers_to_string(b.data());
    return format_to(ctx.out(), "{}", v);
  }
};
}

auto const TEST_URL = "httpbin.org";

std::string operator""_http(const char* target, std::size_t)
{
  return fmt::format("http://{}/{}", TEST_URL, target);
}
std::string operator""_https(const char* target, std::size_t)
{
  return fmt::format("https://{}/{}", TEST_URL, target);
}

// TEST_CASE("http get", "[https][sync]")
// {
//   auto const response = fetchpp::fetch("get"_https);
//   REQUIRE(response.result_int() == 200);
//   REQUIRE(response.at(fetchpp::field::content_type) == "application/json");
//   for (auto const& field : response)
//     fmt::print("{}: {}\n", field.name_string(), field.value());
//   fmt::print("{}\n", response.body());
// }

// TEST_CASE("http get with headers", "[https][sync]")
// {
//   auto const response =
//       fetchpp::fetch("get"_https,
//                      fetchpp::verb::get,
//                      {{"x-special-header", "a value worth reading"},
//                       {fetchpp::field::topic, "http by the book"}});
//   REQUIRE(response.result_int() == 200);
//   REQUIRE(response.at(fetchpp::field::content_type) == "application/json");
//   for (auto const& field : response)
//     fmt::print("{}: {}\n", field.name_string(), field.value());
//   fmt::print("{}\n", response.body());
// }

TEST_CASE("http async get", "[https][sync]")
{
  auto fut = fetchpp::async_fetch(""_https, fetchpp::verb::get);
  auto response = fut.get();
  REQUIRE(response.result_int() == 200);
}