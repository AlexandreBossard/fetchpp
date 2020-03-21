#pragma once

#include <fetchpp/connect.hpp>
#include <fetchpp/process_one.hpp>

#include <fetchpp/alias/error_code.hpp>
#include <fetchpp/alias/http.hpp>
#include <fetchpp/alias/net.hpp>

#include <fetchpp/field_arg.hpp>
#include <fetchpp/cache_mode.hpp>
#include <fetchpp/redirect_handling.hpp>
#include <fetchpp/options.hpp>
#include <fetchpp/version.hpp>

#include <fetchpp/detail/parse_url.hpp>

#include <boost/beast/core/async_base.hpp>
#include <boost/beast/http/empty_body.hpp>
#include <boost/beast/http/string_body.hpp>

#include <fmt/format.h>

namespace fetchpp
{

template <typename BodyRequest>
auto prepare_request(detail::url const& url, options<BodyRequest> const& opt)
    -> http::request<BodyRequest>
{
  auto req = http::request<BodyRequest>(
      opt.method, url.target, 11, std::move(opt.body));
  req.set(http::field::host, url.domain);
  req.set(http::field::user_agent, fetchpp::VERSION);
  for (auto const& field : opt.headers)
    req.insert(field.field, field.field_name, field.value);
  // auto it = req.find(http::field::connection);
  // if (it != req.end())
  //   opt.keep_alive = it == beast::string_view("keep-alive");
  // else if (opt.keep_alive)
  //   req.set(http::field::connection, "keep-alive");
  req.prepare_payload();
  return req;
}

template <typename CompletionToken, typename BodyResponse>
using async_http_result =
    typename net::async_result<typename std::decay_t<CompletionToken>,
                               void(error_code, http::response<BodyResponse>)>;

template <typename CompletionToken, typename BodyResponse>
using async_http_return_t =
    typename async_http_result<CompletionToken, BodyResponse>::return_type;

template <typename GetHandler,
          typename AsyncStream,
          typename BodyResponse = http::string_body>
auto async_get(AsyncStream& stream,
               std::string const& purl,
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
      Request req;
      Response res;
      beast::flat_buffer buffer;
      tcp::resolver resolver;
      temporary_data(AsyncStream& stream, Request req)
        : req(req), res(), buffer(), resolver(stream.get_executor())
      {
      }
    };

    AsyncStream& stream;
    detail::url url;
    temporary_data& data;
    status state = status::starting;

    op(AsyncStream& stream,
       Request preq,
       detail::url purl,
       handler_type& handler)
      : base_type(std::move(handler), stream.get_executor()),
        stream(stream),
        url(purl),
        data(beast::allocate_stable<temporary_data>(
            *this, stream, std::move(preq)))
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
          data.resolver.async_resolve(url.domain, url.scheme, std::move(*this));
          return;
        case connecting:
          state = status::processing;
          async_process_one(stream, data.req, data.res, std::move(*this));
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
      async_connect(stream, results, std::move(*this));
    }
  };

  auto url = detail::parse_url(purl);
  auto request = prepare_request(url,
                                 options<BodyRequest>{http::verb::get,
                                                      cache_mode::no_store,
                                                      redirect_handling::manual,
                                                      fields});
  async_completion_t async_comp{handler};
  op(stream, std::move(request), std::move(url), async_comp.completion_handler);
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
//   req.set(http::field::user_agent, fetchpp::VERSION);
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
