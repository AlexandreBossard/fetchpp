#pragma once

#include <future>
#include <string>
#include <vector>

namespace fetchpp
{
std::future<std::string> fetch(std::string const& url);
}
