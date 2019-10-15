#pragma once

#include <future>
#include <string>
#include <vector>

namespace fetchpp
{
std::future<std::vector<uint8_t>> fetch(std::string const& url);
}
