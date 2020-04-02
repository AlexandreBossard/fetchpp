#include <catch2/catch.hpp>

#include "helpers.hpp"

#include <boost/beast/core/buffers_to_string.hpp>

#include <fetchpp/fetchpp.hpp>
#include <fetchpp/field.hpp>
#include <fetchpp/message.hpp>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/use_future.hpp>

#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/ssl/ssl_stream.hpp>

#include <fmt/format.h>
#include <fmt/ostream.h>
#include <nlohmann/json.hpp>

namespace boost
{
fmt::string_view to_string_view(boost::string_view const& v)
{
  return fmt::string_view{v.data(), v.length()};
}
}

// namespace fmt
// {
// template <>
// struct formatter<boost::beast::multi_buffer>
// {
//   template <typename ParseContext>
//   constexpr auto parse(ParseContext& ctx)
//   {
//     return ctx.begin();
//   }

//   template <typename FormatContext>
//   auto format(boost::beast::multi_buffer const& b, FormatContext& ctx)
//   {
//     auto const v = boost::beast::buffers_to_string(b.data());
//     return format_to(ctx.out(), "{}", v);
//   }
// };
// }

TEST_CASE_METHOD(ioc_fixture, "http async fetch", "[https][fetch][get][async]")
{
  auto request =
      make_request(fetchpp::http::verb::get, fetchpp::url::parse("get"_https));
  request.set(fetchpp::field::content_type, "text/html; charset=UTF8");
  auto response =
      fetchpp::async_fetch(ioc, std::move(request), boost::asio::use_future)
          .get();
  REQUIRE(response.result_int() == 200);
  REQUIRE(response.at(fetchpp::field::content_type) == "application/json");
  auto json = nlohmann::json::parse(response.body());
  auto const content_type = json.at("headers").at(
      std::string{to_string(fetchpp::field::content_type)});
  REQUIRE(content_type == "text/html; charset=UTF8");
}

TEST_CASE_METHOD(ioc_fixture, "http async get", "[https][get][async]")
{
  auto response =
      fetchpp::async_get(
          ioc,
          "get"_https,
          {{fetchpp::field::content_type, "text/html; charset=UTF8"}},
          boost::asio::use_future)
          .get();
  REQUIRE(response.result_int() == 200);
  REQUIRE(response.at(fetchpp::field::content_type) == "application/json");
  auto const& body = response.body();
  auto json = nlohmann::json::parse(body.begin(), body.end());
  auto ct = std::string{to_string(fetchpp::field::content_type)};
  REQUIRE(json.at("headers").at(ct) == "text/html; charset=UTF8");
}

// TEST_CASE_METHOD(ioc_fixture, "http async post string",
// "[https][post][async]")
// {
//   auto const data = std::string("this is my data");
//   auto response = fetchpp::async_post(ioc,
//                                       "post"_https,
//                                       data,
//                                       {{"X-corp-header", "corp value"}},
//                                       boost::asio::use_future)
//                       .get();
//   REQUIRE(response.result_int() == 200);
//   REQUIRE(response.at(fetchpp::field::content_type) == "application/json");
//   auto const& body = response.body();
//   auto json = nlohmann::json::parse(body.begin(), body.end());
//   fmt::print("{}\n", response);
//   REQUIRE(json.at("headers").at("X-Corp-Header") == "corp value");
//   REQUIRE(json.at("data") == data);
// }
