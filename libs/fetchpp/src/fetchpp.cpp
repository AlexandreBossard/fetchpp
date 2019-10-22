#include <fetchpp/fetchpp.hpp>

#include <fetchpp/alias/net.hpp>
#include <fetchpp/version.hpp>

#include "detail/parse_url.hpp"
#include "detail/ssl_client.hpp"

#include <exception>
#include <fmt/format.h>
#include <fmt/ostream.h>

namespace fetchpp
{

response<> fetch(std::string const& purl,
                 verb method,
                 std::initializer_list<field_arg> fields)
{
  fmt::print("the given url: {}\n", purl);
  net::io_context ioc;

  tcp::resolver resolver(ioc);

  auto const url = detail::parse_url(purl);
  auto const results = resolver.resolve(url.domain, url.scheme);
  if (url.scheme != "https")
    throw std::runtime_error("protocol not handled");

  detail::ssl_client fetcher(ioc);

  fetcher.start(results);

  auto req = http::request<http::string_body>(method, url.target, 11);
  req.set(http::field::host, url.domain);
  req.set(http::field::user_agent, fetchpp::VERSION);
  for (auto const& field : fields)
    req.insert(field.field, field.field_name, field.value);

  auto response = fetcher.execute(req);
  fetcher.stop();
  return response;
}
}
