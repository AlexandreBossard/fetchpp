#pragma once

#include <fetchpp/field_arg.hpp>

#include <fetchpp/alias/error_code.hpp>
#include <fetchpp/alias/http.hpp>
#include <fetchpp/alias/net.hpp>
#include <fetchpp/version.hpp>

#include <fetchpp/detail/parse_url.hpp>
#include <fetchpp/detail/ssl_client.hpp>

#include <boost/asio/compose.hpp>
#include <exception>

#include <future>
#include <string>
#include <vector>

namespace fetchpp
{
template <typename CompletionToken, typename BodyResponse>
using async_http_result =
    typename net::async_result<typename std::decay_t<CompletionToken>,
                               void(error_code, http::response<BodyResponse>)>;

template <typename CompletionToken, typename BodyResponse>
using async_http_return_t =
    typename async_http_result<CompletionToken, BodyResponse>::return_type;

template <typename CompletionToken,
          typename AsyncStream,
          typename BodyResponse = http::string_body>
auto async_get(AsyncStream& stream,
               std::string const& purl,
               std::initializer_list<field_arg> fields,
               CompletionToken&& token)
    -> async_http_return_t<CompletionToken, BodyResponse>
{
  auto const url = detail::parse_url(purl);
  if (url.scheme != "https")
    throw std::runtime_error("protocol not handled");

  using BodyRequest = http::empty_body;

  auto req = http::request<BodyRequest>(http::verb::get, url.target, 11);

  req.set(http::field::host, url.domain);
  req.set(http::field::user_agent, fetchpp::VERSION);
  for (auto const& field : fields)
    req.insert(field.field, field.field_name, field.value);

  auto state =
      std::make_unique<detail::state<AsyncStream, BodyRequest, BodyResponse>>(
          std::move(req), stream);
  return net::async_compose<CompletionToken,
                            void(error_code, http::response<BodyResponse>)>(
      detail::ssl_composer<decltype(state)>{std::move(state)}, token, stream);
}

template <typename CompletionToken,
          typename AsyncStream,
          typename BodyRequest = http::string_body,
          typename BodyResponse = http::string_body>
auto async_post(AsyncStream& stream,
                std::string const& purl,
                typename BodyRequest::value_type data,
                std::initializer_list<field_arg> fields,
                CompletionToken&& token)
    -> async_http_return_t<CompletionToken, BodyResponse>

{
  static_assert(beast::is_async_stream<AsyncStream>::value,
                "AsyncStream requirements not met");

  auto const url = detail::parse_url(purl);
  if (url.scheme != "https")
    throw std::runtime_error("protocol not handled");

  auto req = http::request<BodyRequest>(
      http::verb::post, url.target, 11, std::move(data));

  req.set(http::field::host, url.domain);
  req.set(http::field::user_agent, fetchpp::VERSION);
  for (auto const& field : fields)
    req.insert(field.field, field.field_name, field.value);
  req.prepare_payload();

  auto state =
      std::make_unique<detail::state<AsyncStream, BodyRequest, BodyResponse>>(
          std::move(req), stream);
  return net::async_compose<CompletionToken,
                            void(error_code, http::response<BodyResponse>)>(
      detail::ssl_composer<decltype(state)>{std::move(state)}, token, stream);
}
}
