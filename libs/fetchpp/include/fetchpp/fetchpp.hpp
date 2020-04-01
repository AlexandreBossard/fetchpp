#pragma once

#include <fetchpp/cache_mode.hpp>
#include <fetchpp/connect.hpp>
#include <fetchpp/field_arg.hpp>
#include <fetchpp/options.hpp>
#include <fetchpp/process_one.hpp>
#include <fetchpp/redirect_handling.hpp>
#include <fetchpp/url.hpp>

#include <fetchpp/detail/prepare_request.hpp>

#include <fetchpp/alias/error_code.hpp>
#include <fetchpp/alias/http.hpp>
#include <fetchpp/alias/net.hpp>

#include <boost/beast/core/async_base.hpp>
#include <boost/beast/http/empty_body.hpp>
#include <boost/beast/http/string_body.hpp>

namespace fetchpp
{

template <typename CompletionToken, typename BodyResponse>
using async_http_result =
    typename net::async_result<typename std::decay_t<CompletionToken>,
                               void(error_code, http::response<BodyResponse>)>;

template <typename CompletionToken, typename BodyResponse>
using async_http_return_t =
    typename async_http_result<CompletionToken, BodyResponse>::return_type;

template <typename GetHandler, typename BodyResponse = http::string_body>
auto async_get(net::io_context& ioc,
               std::string const& url_str,
               std::initializer_list<field_arg> fields,
               GetHandler&& handler)
    -> async_http_return_t<GetHandler, BodyResponse>
{
  using BodyRequest = http::empty_body;
  using Request = http::request<BodyRequest>;
  using Response = http::response<BodyResponse>;
  using async_completion_t =
      net::async_completion<GetHandler,
                            void(error_code, http::response<BodyResponse>)>;
  using handler_type = typename async_completion_t::completion_handler_type;
  using AsyncStream = beast::ssl_stream<beast::tcp_stream>;

  net::ssl::context sslc(net::ssl::context::tlsv12_client);
  AsyncStream stream(ioc, sslc);

  using base_type =
      beast::stable_async_base<handler_type,
                               typename AsyncStream::executor_type>;

  struct op : base_type
  {
    enum status : int
    {
      starting,
      resolving,
      connecting,
      processing,
    };

    struct temporary_data
    {
      AsyncStream stream;
      Request req;
      Response res;
      beast::flat_buffer buffer;
      tcp::resolver resolver;
      temporary_data(AsyncStream pstream, Request req)
        : stream(std::move(pstream)),
          req(req),
          res(),
          buffer(),
          resolver(stream.get_executor())
      {
      }
    };

    url _url;
    temporary_data& data;
    status state = status::starting;

    op(AsyncStream stream, Request preq, url purl, handler_type& handler)
      : base_type(std::move(handler), stream.get_executor()),
        _url(purl),
        data(beast::allocate_stable<temporary_data>(
            *this, std::move(stream), std::move(preq)))
    {
      (*this)();
    }

    void operator()(error_code ec = {})
    {
      if (!ec)
      {
        switch (state)
        {
        case starting:
          state = status::resolving;
          data.resolver.async_resolve(
              _url.domain(), _url.scheme(), std::move(*this));
          return;
        case connecting:
          state = status::processing;
          async_process_one(data.stream, data.req, data.res, std::move(*this));
          return;
        default:
          assert(0);
          break;
        case processing:
          break;
        }
      }
      this->complete_now(ec, std::move(std::exchange(data.res, Response{})));
    }

    void operator()(error_code ec, tcp::resolver::results_type results)
    {
      if (ec)
      {
        this->complete_now(ec, std::move(std::exchange(data.res, Response{})));
        return;
      }
      state = status::connecting;
      async_connect(data.stream, results, std::move(*this));
    }
  };

  auto const curl = url::parse(url_str);
  auto request =
      detail::prepare_request(curl,
                              options<BodyRequest>{http::verb::get,
                                                   cache_mode::no_store,
                                                   redirect_handling::manual,
                                                   fields});
  async_completion_t async_comp{handler};
  op(std::move(stream),
     std::move(request),
     std::move(curl),
     async_comp.completion_handler);
  return async_comp.result.get();
}
// template <typename CompletionToken,
//           typename AsyncStream,
//           typename BodyRequest = http::string_body,
//           typename BodyResponse = http::string_body>
// auto async_post(AsyncStream& stream,
//                 std::string const& purl,
//                 typename BodyRequest::value_type data,
//                 std::initializer_list<field_arg> fields,
//                 CompletionToken&& handler)
//     -> async_http_return_t<CompletionToken, BodyResponse>

// {
//   static_assert(beast::is_async_stream<AsyncStream>::value,
//                 "AsyncStream requirements not met");

//   auto const url = detail::parse_url(purl);
//   if (url.scheme != "https")
//     throw std::runtime_error("protocol not handled");

//   auto req = http::request<BodyRequest>(
//       http::verb::post, url.target, 11, std::move(data));

//   req.set(http::field::host, url.domain);
//   req.set(http::field::user_agent, fetchpp::USER_AGENT);
//   for (auto const& field : fields)
//     req.insert(field.field, field.field_name, field.value);
//   req.prepare_payload();

//   auto state =
//       std::make_unique<detail::state<AsyncStream, BodyRequest,
//       BodyResponse>>(
//           std::move(req), stream);
//   return net::async_compose<CompletionToken,
//                             void(error_code, http::response<BodyResponse>)>(
//       detail::ssl_composer<decltype(state)>{std::move(state)}, handler,
//       stream);
// }
}
