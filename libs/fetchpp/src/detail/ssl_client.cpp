#include "detail/ssl_client.hpp"

#include <boost/beast/ssl.hpp>

namespace fetchpp
{
namespace detail
{

ssl_client::ssl_client(net::io_context& ioc)
  : ctx(net::ssl::context::tlsv12_client), stream(ioc, ctx)
{
  // FIXME
  // ctx.set_default_verify_paths();
  // ctx.set_verify_mode(ssl::verify_peer);
}

void ssl_client::start(tcp::resolver::results_type const& results)
{
  beast::get_lowest_layer(stream).connect(results);
  stream.handshake(net::ssl::stream_base::client);
}

void ssl_client::stop()
{
  boost::system::error_code ec;
  stream.shutdown(ec);
  if (ec && ec != beast::errc::not_connected)
    throw beast::system_error{ec};
}
}
}