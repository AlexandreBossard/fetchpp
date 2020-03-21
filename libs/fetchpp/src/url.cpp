#include <fetchpp/url.hpp>

#include <exception>
#include <regex>

namespace fetchpp
{
url::url(std::string const& url)
{
  using namespace std::string_literals;
  // https://regex101.com/r/qrDzR5/6
  static auto const rg = std::regex(
      R"(^(https?)://(?:.*?@)?(.+(?:\..+?)+?)(?::(\d+))?([/?].*?)?$)",
      std::regex::optimize);
  std::smatch match;
  if (!std::regex_match(url, match, rg))
    throw std::runtime_error("bad url");
  _port = std::stoi(match[3].matched ?
                        match[3].str() :
                        (match[1].compare("https") == 0 ? "443"s : "80"s));
  if (!match[2].matched)
    throw std::runtime_error("no domain!");

  _scheme = match[1];
  _domain = match[2];
  _target = match[4];
}

std::string const& url::scheme() const
{
  return _scheme;
}

std::string const& url::domain() const
{
  return _domain;
}

uint16_t url::port() const
{
  return _port;
}

std::string const& url::target() const
{
  return _target;
}

void url::set_scheme(std::string const& scheme)
{
  _scheme = scheme;
}

void url::set_domain(std::string const& domain)
{
  _domain = domain;
}

void url::set_port(uint16_t p)
{
  _port = p;
}

void url::set_target(std::string const& target)
{
  _target = target;
}
}
