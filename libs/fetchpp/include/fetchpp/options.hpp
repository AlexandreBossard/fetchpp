#pragma once

#include <fetchpp/message.hpp>
#include <fetchpp/field_arg.hpp>
#include <fetchpp/redirect_handling.hpp>

#include <vector>

namespace fetchpp
{
template <typename BodyRequest = http::string_body>
struct options
{
  http::verb method = http::verb::get;
  cache_mode cache = cache_mode::no_store;
  redirect_handling redirect = redirect_handling::manual;
  std::vector<field_arg> headers;
  typename BodyRequest::value_type body;
  bool keep_alive = false;
};
}
