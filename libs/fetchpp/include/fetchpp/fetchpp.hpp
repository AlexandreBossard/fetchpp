#pragma once

#include <boost/beast/http.hpp>

#include <string>
#include <vector>

namespace fetchpp
{
namespace http = boost::beast::http;
using field = http::field;
struct field_arg
{
  http::field field;
  std::string field_name;
  std::string value;

  field_arg() = delete;
  field_arg(field_arg const&) = default;
  field_arg(field_arg&&) = default;
  field_arg& operator=(field_arg&&) = default;
  field_arg& operator=(field_arg const&) = default;

  field_arg(http::field field, std::string value);
  field_arg(std::string field_name, std::string value);
};

template <typename BodyType = http::dynamic_body>
using response = http::response<BodyType>;

response<> fetch(std::string const& url,
                 std::initializer_list<field_arg> fields = {});
}
