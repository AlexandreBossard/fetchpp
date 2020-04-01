#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/core.hpp>

#include <fetchpp/alias/beast.hpp>
#include <fetchpp/alias/net.hpp>
#include <fetchpp/alias/tcp.hpp>

#include <fmt/format.h>

#include <string>
#include <thread>

static auto constexpr TEST_URL = "httpbin.org";

inline std::string operator""_http(const char* target, std::size_t)
{
  return fmt::format("http://{}/{}", TEST_URL, target);
}

inline std::string operator""_https(const char* target, std::size_t)
{
  return fmt::format("https://{}/{}", TEST_URL, target);
}

inline auto http_resolve_domain(fetchpp::net::io_context& ioc,
                                std::string const& domain)
{
  fetchpp::tcp::resolver resolver(ioc);
  return resolver.resolve(domain, "https");
}

template <typename Stream>
void http_ssl_connect(fetchpp::net::io_context& ioc,
                      Stream& stream,
                      std::string const& domain)
{
  auto results = http_resolve_domain(ioc, domain);
  fetchpp::beast::get_lowest_layer(stream).connect(results);
  stream.handshake(fetchpp::net::ssl::stream_base::client);
}

struct ioc_fixture
{
  fetchpp::net::io_context ioc;

  using work_guard = fetchpp::net::executor_work_guard<
      fetchpp::net::io_context::executor_type>;

  ioc_fixture()
    : work(fetchpp::net::make_work_guard(ioc)), worker([this]() { ioc.run(); })
  {
  }
  ~ioc_fixture()
  {
    work.reset();
    worker.join();
  }

private:
  work_guard work;
  std::thread worker;
};
