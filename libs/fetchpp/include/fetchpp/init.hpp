#pragma once

#include <fetchpp/alias/http.hpp>

#include <fetchpp/field_arg.hpp>

namespace fetchpp
{
enum class cache_mode : int
{
  no_store,
  cached,
  reload,
  force_cache,
  only_if_cached,
};

enum class redirect_handling : int
{
  manual,
  abort,
  follow,
};

template <typename BodyRequest = http::string_body>
struct options
{
  http::verb method = http::verb::get;
  cache_mode cache = cache_mode::no_store;
  redirect_handling redirect = redirect_handling::manual;
  bool keep_alive = false;
  std::initializer_list<field_arg> headers;
  typename BodyRequest::value_type body;
};
}