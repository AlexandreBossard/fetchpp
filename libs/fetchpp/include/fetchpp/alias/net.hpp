#pragma once

#include <boost/asio/ip/tcp.hpp>

namespace fetchpp
{
namespace net = boost::asio;
using tcp = net::ip::tcp;
}