#pragma once
#include <fetchpp/alias/beast.hpp>
#include <fetchpp/alias/error_code.hpp>
#include <fetchpp/alias/http.hpp>
#include <fetchpp/alias/net.hpp>

#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/ssl/ssl_stream.hpp>
#include <fmt/format.h>

#include <future>

namespace fetchpp
{
namespace ssl = net::ssl;
namespace detail
{
template <typename Request,
          typename Response = http::response<http::string_body>>
class ssl_client : public std::enable_shared_from_this<ssl_client<Request, Response>>
{
public:
  using stream_type = beast::ssl_stream<beast::tcp_stream>;

private:
  net::ssl::context _ctx;
  tcp::resolver _resolver;
  stream_type _stream;
  beast::flat_buffer _buffer;

  Request _req;
  Response _res;
  std::promise<Response> _p;

public:
  using Ptr = std::shared_ptr<ssl_client>;

  ssl_client(Request req, net::io_context& ioc)
    : _ctx(net::ssl::context::tlsv12_client),
      _resolver(ioc),
      _stream(ioc, _ctx),
      _req(std::move(req))
  {
    // FIXME
    // ctx.set_default_verify_paths();
    // ctx.set_verify_mode(ssl::verify_peer);
  }

  std::future<Response> execute(std::string domain, std::string service)
  {
    auto that = this->shared_from_this();
    _resolver.async_resolve(
        domain, service, [that](error_code ec, auto results) {
          that->run(ec, results);
        });

    return _p.get_future();
  }

  void run(error_code ec, tcp::resolver::results_type results)
  {
    if (ec)
      return fail(ec, "resolve");

    beast::get_lowest_layer(_stream).async_connect(
        results, [a = this->shared_from_this()](error_code ec, auto) {
          a->handshake(ec);
        });
  }

private:
  void handshake(error_code ec)
  {
    if (ec)
      return fail(ec, "connect");

    _stream.async_handshake(
        ssl::stream_base::client,
        [a = this->shared_from_this()](error_code ec) { a->send_request(ec); });
  }

  void send_request(error_code ec)
  {
    if (ec)
      return fail(ec, "handshake");
    http::async_write(
        _stream,
        _req,
        [a = this->shared_from_this()](error_code ec, std::size_t) {
          a->recv_response(ec);
        });
  }

  void recv_response(error_code ec)
  {
    if (ec)
      return fail(ec, "send_request");

    http::async_read(
        _stream, _buffer, _res, [a = this->shared_from_this()](error_code ec, auto) {
          a->finish(ec);
        });
  }

  void finish(error_code ec)
  {
    if (ec)
      return fail(ec, "recv_response");
    _p.set_value(_res);
    _stream.async_shutdown(
        [a = this->shared_from_this()](error_code ec) { a->stopped(ec); });
    if (ec && ec != beast::errc::not_connected)
      throw beast::system_error{ec};
  }

  void stopped(error_code ec)
  {
    // if (ec == net::error::eof)
    //   ec = {};
  }

  void fail(error_code const& ec, fmt::string_view subject)
  {
    _p.set_exception(std::make_exception_ptr(beast::system_error(ec)));
    fmt::print("failure {}: {}\n", subject, ec.message());
  }

  // template <typename BodyType, typename ResponseBody = http::dynamic_body>
  // http::response<ResponseBody> execute(http::request<BodyType> const& req)
  // {
  //   http::write(stream, req);
  //   auto buffer = beast::flat_buffer{};
  //   auto res = http::response<ResponseBody>{};
  //   http::read(stream, buffer, res);
  //   return res;
  // }
};

template <typename Request,
          typename Response = http::response<http::string_body>>
auto create_client(Request req, net::io_context& ioc) -> typename ssl_client<Request,Response>::Ptr
{
  return std::make_shared<ssl_client<Request, Response>>(std::move(req), ioc);
}
}
}