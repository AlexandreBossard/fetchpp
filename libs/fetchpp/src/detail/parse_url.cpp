#include "detail/parse_url.hpp"
#include <exception>
#include <regex>

namespace fetchpp
{
namespace detail
{
url parse_url(std::string const& url)
{
  using namespace std::string_literals;
  // https://regex101.com/r/qrDzR5/6
  static auto const rg = std::regex(
      R"(^(https?)://(?:.*?@)?(.+(?:\..+?)+?)(?::(\d+))?([/?].*?)?$)",
      std::regex::optimize);
  std::smatch match;
  if (!std::regex_match(url, match, rg))
    throw std::runtime_error("bad url");
  auto port = match[3].matched ?
                  match[3].str() :
                  (match[1].compare("https") == 0 ? "443"s : "80"s);
  if (!match[2].matched)
    throw std::runtime_error("no domain!");
  return {match[1], match[2], port, match[4]};
}
}
}
