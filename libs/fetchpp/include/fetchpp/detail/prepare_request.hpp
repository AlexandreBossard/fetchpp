#pragma once

#include <fetchpp/url.hpp>

#include <fetchpp/message.hpp>
#include <fetchpp/options.hpp>
#include <fetchpp/version.hpp>

namespace fetchpp
{
namespace detail
{
template <typename BodyRequest>
auto prepare_request(url const& url, options<BodyRequest> const& opt)
    -> http::request<BodyRequest>
{
  auto req = http::request<BodyRequest>(
      opt.method, url.target(), 11, std::move(opt.body));
  req.set(http::field::host, url.domain());
  req.set(http::field::user_agent, fetchpp::USER_AGENT);
  for (auto const& field : opt.headers)
    req.insert(field.field, field.field_name, field.value);
  req.prepare_payload();
  return req;
}
}
}
