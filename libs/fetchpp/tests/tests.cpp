#include <catch2/catch.hpp>

#include <boost/beast/core/buffers_to_string.hpp>

#include <fetchpp/fetchpp.hpp>

#include <boost/asio/use_future.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/ssl/ssl_stream.hpp>

#include <fmt/core.h>
#include <fmt/format.h>
#include <iostream>
#include <nlohmann/json.hpp>

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

TEST_CASE("http async get", "[https][get][async]")
{
  fetchpp::net::io_context ioc;
  fetchpp::ssl::context context(fetchpp::ssl::context::tlsv12_client);
  fetchpp::beast::ssl_stream<fetchpp::beast::tcp_stream> stream(ioc, context);
  auto fut = fetchpp::async_get(
      stream,
      "get"_https,
      {{fetchpp::field::content_type, "text/html; charset=UTF8"}},
      boost::asio::use_future);
  ioc.run();
  auto response = fut.get();
  REQUIRE(response.result_int() == 200);
  REQUIRE(response.at(fetchpp::field::content_type) == "application/json");
  auto const& body = response.body();
  auto json = nlohmann::json::parse(body.begin(), body.end());
  auto ct = std::string{to_string(fetchpp::field::content_type)};
  REQUIRE(json.at("headers").at(ct) == "text/html; charset=UTF8");
}

TEST_CASE("http async post string", "[https][post][async]")
{
  fetchpp::net::io_context ioc;
  fetchpp::ssl::context context(fetchpp::ssl::context::tlsv12_client);
  fetchpp::beast::ssl_stream<fetchpp::beast::tcp_stream> stream(ioc, context);
  auto const data = std::string("this is my data");
  auto fut = fetchpp::async_post(stream,
                                 "post"_https,
                                 data,
                                 {{"X-corp-header", "corp value"}},
                                 boost::asio::use_future);
  ioc.run();
  auto response = fut.get();
  REQUIRE(response.result_int() == 200);
  REQUIRE(response.at(fetchpp::field::content_type) == "application/json");
  auto const& body = response.body();
  auto json = nlohmann::json::parse(body.begin(), body.end());
  fmt::print("{}\n", response);
  REQUIRE(json.at("headers").at("X-Corp-Header") == "corp value");
  REQUIRE(json.at("data") == data);
}

TEST_CASE("http async post string with init", "[https][fetch][init][async]")
{
  fetchpp::net::io_context ioc;
  fetchpp::ssl::context context(fetchpp::ssl::context::tlsv12_client);
  fetchpp::beast::ssl_stream<fetchpp::beast::tcp_stream> stream(ioc, context);
  auto const data = std::string("this is my data");
  auto init = fetchpp::options<fetchpp::http::string_body>{
      fetchpp::http::verb::post,
      fetchpp::cache_mode::no_store,
      fetchpp::redirect_handling::manual,
      false,
      {{"X-corp-header", "corp value"}},
      data};
  auto fut = fetchpp::async_fetch(
      stream, "post"_https, std::move(init), boost::asio::use_future);
  ioc.run();
  auto response = fut.get();
  REQUIRE(response.result_int() == 200);
  // REQUIRE(response.at(fetchpp::field::content_type) == "application/json");
}