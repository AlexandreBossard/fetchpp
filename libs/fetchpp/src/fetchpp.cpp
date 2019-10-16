#include <fetchpp/fetchpp.hpp>
#include <fmt/format.h>

namespace fetchpp
{
std::future<std::string> fetch(std::string const &url)
{
  fmt::print("the given url: {}", url);
  auto task = std::packaged_task<std::string(std::string)>(
      [](auto param) { return fmt::format("the return content"); });
  return task.get_future();
}
}
