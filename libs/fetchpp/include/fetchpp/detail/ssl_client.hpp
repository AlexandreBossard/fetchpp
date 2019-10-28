#pragma once
#include <fetchpp/alias/beast.hpp>
#include <fetchpp/alias/error_code.hpp>
#include <fetchpp/alias/http.hpp>
#include <fetchpp/alias/net.hpp>
#include <fetchpp/alias/tcp.hpp>

#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/ssl/ssl_stream.hpp>
#include <fmt/format.h>
#include <fmt/ostream.h>

namespace fetchpp
{
namespace ssl = net::ssl;
namespace detail
{
template <typename AsyncStream,
          typename BodyRequest,
          typename BodyResponse = BodyRequest>
struct state
{
  using Request = http::request<BodyRequest>;
  using Response = http::response<BodyResponse>;

  Request req;
  Response res;
  beast::flat_buffer buffer;
  AsyncStream& stream;
  tcp::resolver resolver;

  state(Request req, AsyncStream& stream)
    : req(std::move(req)), stream(stream), resolver(stream.get_executor())
  {
  }
};

template <typename StatePtr>
struct ssl_composer
{
  StatePtr _state;
  enum
  {
    starting,
    resolving,
    connecting,
    handshaking,
    sending,
    receiving,
    closing,
  } _status = starting;

  ssl_composer(ssl_composer const&) = delete;
  ssl_composer(ssl_composer&&) = default;
  ssl_composer& operator=(ssl_composer&&) = default;
  ssl_composer& operator=(ssl_composer const&) = delete;

  template <typename Self>
  void operator()(Self& self, error_code ec = error_code{}, std::size_t n = 0)
  {
    if (!ec)
    {
      switch (_status)
      {
      case starting:
        _status = resolving;
        _state->resolver.async_resolve(
            std::string((_state->req)[http::field::host]),
            "https",
            std::move(self));
        return;
      case handshaking:
        _status = sending;
        http::async_write(_state->stream, _state->req, std::move(self));
        return;
      case sending:
        _status = receiving;
        http::async_read(
            _state->stream, _state->buffer, _state->res, std::move(self));
        return;
      case receiving:
        _status = closing;
        _state->stream.async_shutdown(std::move(self));
        return;
      case connecting:
      case resolving:
        assert(0);
      case closing:
        break;
      }
    }
    self.complete(ec, _state->res);
  }

  template <typename Self>
  void operator()(Self& self,
                  error_code ec,
                  tcp::resolver::results_type results)
  {
    if (ec)
    {
      self.complete(ec, {});
      return;
    }
    _status = connecting;
    beast::get_lowest_layer(_state->stream)
        .async_connect(results, std::move(self));
  }

  template <typename Self>
  void operator()(Self& self,
                  error_code ec,
                  tcp::resolver::results_type::endpoint_type endpoint)
  {
    if (ec)
    {
      self.complete(ec, {});
      return;
    }
    _status = handshaking;
    _state->stream.async_handshake(ssl::stream_base::client, std::move(self));
  }
};
}
}
