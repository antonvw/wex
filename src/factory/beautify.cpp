////////////////////////////////////////////////////////////////////////////////
// Name:      beautify.cpp
// Purpose:   Implementation of wex::factory::beautify class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/factory/beautify.h>
#include <wex/factory/process.h>
#include <wx/translation.h>

bool wex::factory::beautify::file(const path& p) const
{
  return is_auto() && is_active() && is_supported(p) &&
         factory::process().system(
           name() + " -i " + p.string() +
           " --style=file --fallback-style=none") == 0;
}

bool wex::factory::beautify::is_active() const
{
  return !name().empty();
}

bool wex::factory::beautify::is_auto() const
{
  return config(_("stc.Auto beautify")).get(false);
}

bool wex::factory::beautify::is_supported(const path& p) const
{
  return p.extension() == ".cpp" || p.extension() == ".java" ||
         p.extension() == ".javascript";
}

wex::config::strings_t wex::factory::beautify::list() const
{
  return config::strings_t{{""}, {"clang-format"}};
}

const std::string wex::factory::beautify::name() const
{
  return config(_("stc.Beautifier")).get(list()).front();
}
