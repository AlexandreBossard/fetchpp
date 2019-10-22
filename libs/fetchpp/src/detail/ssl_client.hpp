#pragma once
#include <fetchpp/alias/beast.hpp>
#include <fetchpp/alias/http.hpp>
#include <fetchpp/alias/net.hpp>

#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/ssl/ssl_stream.hpp>

namespace fetchpp
{
namespace detail
{
class ssl_client
{
public:
  using stream_type = beast::ssl_stream<beast::tcp_stream>;

private:
  net::ssl::context ctx;
  stream_type stream;

public:
  ssl_client(net::io_context& ioc);

  void start(tcp::resolver::results_type const& results);

  void stop();

  template <typename BodyType, typename ResponseBody = http::dynamic_body>
  http::response<ResponseBody> execute(http::request<BodyType> const& req)
  {
    http::write(stream, req);
    auto buffer = beast::flat_buffer{};
    auto res = http::response<ResponseBody>{};
    http::read(stream, buffer, res);
    return res;
  }
};
}
}