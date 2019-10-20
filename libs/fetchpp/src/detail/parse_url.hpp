#include <string>
#include <utility>

namespace fetchpp
{
namespace detail
{
struct url
{
  std::string scheme;
  std::string domain;
  std::string port;
  std::string target;
};
url parse_url(std::string const& url);
}
}
