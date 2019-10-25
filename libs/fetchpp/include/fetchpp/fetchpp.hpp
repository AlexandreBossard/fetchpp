#pragma once

#include <fetchpp/field_arg.hpp>

#include <fetchpp/alias/http.hpp>

#include <future>
#include <string>
#include <vector>

namespace fetchpp
{
using http::verb;
template <typename BodyType = http::dynamic_body>
using response = http::response<BodyType>;

response<> fetch(std::string const& url,
                 fetchpp::verb method = fetchpp::verb::get,
                 std::initializer_list<field_arg> fields = {});

std::future<response<>> async_fetch(
    std::string const& purl,
    fetchpp::verb method = fetchpp::verb::get,
    std::initializer_list<field_arg> fields = {});
}
