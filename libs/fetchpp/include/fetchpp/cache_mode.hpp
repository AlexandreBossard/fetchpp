#pragma once

namespace fetchpp
{
enum class cache_mode : int
{
  no_store,
  cached,
  reload,
  force_cache,
  only_if_cached,
};
}
