#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/core.hpp>

#include <fetchpp/alias/beast.hpp>
#include <fetchpp/alias/net.hpp>
#include <fetchpp/alias/tcp.hpp>

#include <fmt/format.h>

#include <string>

static auto constexpr TEST_URL = "httpbin.org";

inline std::string operator""_http(const char* target, std::size_t)
{
  return fmt::format("http://{}/{}", TEST_URL, target);
}

inline std::string operator""_https(const char* target, std::size_t)
{
  return fmt::format("https://{}/{}", TEST_URL, target);
}


inline auto http_resolve_domain(fetchpp::net::io_context &ioc, std::string const &domain)
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
