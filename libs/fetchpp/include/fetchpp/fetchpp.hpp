#pragma once

#include <boost/beast/http.hpp>

#include <string>
#include <vector>

namespace fetchpp
{
namespace http = boost::beast::http;
using field = http::field;
template <typename BodyType = http::dynamic_body>
using response = http::response<BodyType>;

response<> fetch(std::string const& url);
}
