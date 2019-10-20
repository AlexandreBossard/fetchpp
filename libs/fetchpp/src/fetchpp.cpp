#include "detail/parse_url.hpp"

#include <boost/asio.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <exception>
#include <fetchpp/fetchpp.hpp>
#include <fmt/format.h>
#include <fmt/ostream.h>

namespace beast = boost::beast;
namespace http = boost::beast::http;
namespace net = boost::asio;
namespace ssl = net::ssl;
using tcp = net::ip::tcp;

namespace fetchpp
{
class ssl_fecther
{
public:
  using stream_type = beast::ssl_stream<beast::tcp_stream>;

private:
  ssl::context ctx;
  stream_type stream;

public:
  ssl_fecther(net::io_context& ioc)
    : ctx(ssl::context::tlsv12_client), stream(ioc, ctx)
  {
    // FIXME
    // ctx.set_default_verify_paths();
    // ctx.set_verify_mode(ssl::verify_peer);
  }

  void start(tcp::resolver::results_type const& results)
  {
    beast::get_lowest_layer(stream).connect(results);
    stream.handshake(ssl::stream_base::client);
  }
  void stop()
  {
    boost::system::error_code ec;
    stream.shutdown(ec);
    if (ec && ec != beast::errc::not_connected)
      throw beast::system_error{ec};
  }

  template <typename BodyType, typename ResponseBody = http::dynamic_body>
  http::response<ResponseBody> fetch(http::request<BodyType> const& req)
  {
    http::write(stream, req);
    auto buffer = beast::flat_buffer{};
    auto res = http::response<http::dynamic_body>{};
    http::read(stream, buffer, res);
    return res;
  }
};
response<> fetch(std::string const& purl)
{
  fmt::print("the given url: {}\n", purl);
  net::io_context ioc;

  tcp::resolver resolver(ioc);

  auto const url = detail::parse_url(purl);
  fmt::print(
      "domain: {}, service {} target{}\n", url.domain, url.scheme, url.target);

  auto const results = resolver.resolve(url.domain, url.scheme);
  if (url.scheme != "https")
    throw std::runtime_error("protocol not handled");
  ssl_fecther fetcher(ioc);

  fetcher.start(results);

  auto req = http::request<http::empty_body>(http::verb::get, url.target, 11);
  req.set(http::field::host, url.domain);
  req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

  auto response = fetcher.fetch(req);
  fetcher.stop();
  return response;
}
}
