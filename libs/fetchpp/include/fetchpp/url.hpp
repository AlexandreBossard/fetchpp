#pragma once

#include <string>

namespace fetchpp
{
class url
{
public:
  explicit url(std::string const& url);

  // [[nodiscard]] std::string str() const;

  std::string const& scheme() const;
  std::string const& domain() const;
  uint16_t port() const;
  std::string const& target() const;

  void set_scheme(std::string const&);
  void set_domain(std::string const&);
  void set_port(uint16_t);
  void set_target(std::string const&);

private:
  std::string _scheme;
  std::string _domain;
  uint16_t _port;
  std::string _target;
};
}
