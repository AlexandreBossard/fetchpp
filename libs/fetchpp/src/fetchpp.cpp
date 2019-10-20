#include "detail/parse_url.hpp"

#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <exception>
#include <fetchpp/fetchpp.hpp>
#include <fmt/format.h>
#include <fmt/ostream.h>

namespace beast = boost::beast;
namespace http = boost::beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

namespace fetchpp
{
response<> fetch(std::string const& purl)
{
  fmt::print("the given url: {}\n", purl);
  net::io_context ioc;

  tcp::resolver resolver(ioc);
  // beast::tcp_stream stream(ioc);
  tcp::socket socket(ioc);

  auto const url = detail::parse_url(purl);
  fmt::print(
      "domain: {}, service {} target{}\n", url.domain, url.scheme, url.target);
  auto const results = resolver.resolve(url.domain, url.scheme);
  net::connect(socket, results.begin(), results.end());
  auto req = http::request<http::empty_body>(http::verb::get, url.target, 11);
  req.set(http::field::host, url.domain);
  req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

  http::write(socket, req);

  auto buffer = beast::flat_buffer{};
  auto res = http::response<http::dynamic_body>{};
  http::read(socket, buffer, res);

  boost::system::error_code ec;
  socket.shutdown(tcp::socket::shutdown_both, ec);
  return res;
}
}
