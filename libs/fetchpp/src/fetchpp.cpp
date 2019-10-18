#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <exception>
#include <fetchpp/fetchpp.hpp>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <regex>

namespace beast = boost::beast;
namespace http = boost::beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

namespace fetchpp
{
namespace
{
std::pair<std::string, std::string> parse_uri(std::string const& url)
{
  using namespace std::string_literals;
  // https://regex101.com/r/qrDzR5/6
  static auto const rg = std::regex(
      R"(^(https?)://(?:.*?@)?([\w-]+(?:\.[\w-]+)+?)([/?].*?)?(?::(\d+))?$)",
      std::regex::optimize);
  std::smatch match;
  if (!std::regex_match(url, match, rg))
    throw std::runtime_error("bad url");
  auto port = match[4].matched ?
                  match[4].str() :
                  (match[1].compare("https") == 0 ? "443"s : "80"s);
  if (!match[2].matched)
    throw std::runtime_error("no domain!");
  return std::make_pair(std::move(match[2].str()), std::move(port));
}
}
std::vector<uint8_t> fetch(std::string const& url)
{
  fmt::print("the given url: {}", url);
  net::io_context ioc;

  tcp::resolver resolver(ioc);
  // beast::tcp_stream stream(ioc);
  tcp::socket socket(ioc);

  std::string domain, service;
  std::tie(domain, service) = parse_uri(url);
  fmt::print("domain: {}, service {}\n", domain, service);
  auto const results = resolver.resolve(domain, service);
  fmt::print("resolve size {}\n", results.size());
  net::connect(socket, results.begin(), results.end());
  auto req = http::request<http::string_body>(http::verb::get, "/", 11);
  req.set(http::field::host, domain);
  req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

  http::write(socket, req);

  auto buffer = beast::flat_buffer{};
  auto res = http::response<http::dynamic_body>{};
  http::read(socket, buffer, res);

  fmt::print("{}\n", res);
  boost::system::error_code ec;
  socket.shutdown(tcp::socket::shutdown_both, ec);
  return {};
}
}
