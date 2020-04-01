#include <catch2/catch.hpp>

#include <fetchpp/connect.hpp>
#include <fetchpp/process_one.hpp>

#include <fetchpp/field.hpp>
#include <fetchpp/message.hpp>
#include <fetchpp/version.hpp>

#include <boost/asio/use_future.hpp>
#include <boost/beast/ssl/ssl_stream.hpp>

#include "helpers.hpp"

namespace fetchpp
{
namespace ssl = net::ssl;
}

TEST_CASE_METHOD(ioc_fixture,
                 "process one get request",
                 "[https][process_one][async]")
{
  fetchpp::ssl::context context(fetchpp::ssl::context::tlsv12_client);
  fetchpp::beast::ssl_stream<fetchpp::beast::tcp_stream> stream(ioc, context);

  fetchpp::request<fetchpp::empty_body> request(
      fetchpp::http::verb::get, "/get", 11);
  request.set(fetchpp::field::host, TEST_URL);
  request.set(fetchpp::field::user_agent, fetchpp::USER_AGENT);
  fetchpp::response<fetchpp::string_body> response;

  http_ssl_connect(ioc, stream, TEST_URL);

  auto fut = fetchpp::async_process_one(
      stream, request, response, boost::asio::use_future);
  fut.get();
  REQUIRE(response.result_int() == 200);
  REQUIRE(response.at(fetchpp::field::content_type) == "application/json");
}

TEST_CASE_METHOD(ioc_fixture, "connect", "[http][connect][async]")
{
  fetchpp::ssl::context context(fetchpp::ssl::context::tlsv12_client);
  fetchpp::beast::ssl_stream<fetchpp::beast::tcp_stream> stream(ioc, context);

  fetchpp::request<fetchpp::empty_body> request(
      fetchpp::http::verb::get, "/get", 11);
  request.set(fetchpp::field::host, TEST_URL);
  request.set(fetchpp::field::user_agent, fetchpp::USER_AGENT);
  fetchpp::response<fetchpp::string_body> response;

  auto results = http_resolve_domain(ioc, TEST_URL);
  auto fut = fetchpp::async_connect(stream, results, boost::asio::use_future);
  fut.get();

  auto fut2 = fetchpp::async_process_one(
      stream, request, response, boost::asio::use_future);
  fut2.get();
  REQUIRE(response.result_int() == 200);
  REQUIRE(response.at(fetchpp::field::content_type) == "application/json");
}
