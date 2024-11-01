////////////////////////////////////////////////////////////////////////////////
// Name:      beautify.cpp
// Purpose:   Implementation of wex::factory::beautify class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/factory/beautify.h>
#include <wex/factory/process.h>
#include <wx/translation.h>

wex::factory::beautify::beautify(beautify_t t)
  : m_type(t)
{
}

wex::factory::beautify::beautify(const path& p)
{
  check(p);
}

bool wex::factory::beautify::check(const path& p)
{
  for (beautify_t t : {SOURCE, CMAKE, ROBOTFRAMEWORK})
  {
    if (beautify(t).is_supported(p))
    {
      m_type = t;
      return true;
    }
  }

  return false;
}

bool wex::factory::beautify::file(const path& p) const
{
  if (!is_auto() || !is_active() || !is_supported(p))
  {
    return false;
  }

  switch (m_type)
  {
    case CMAKE:
      return factory::process().system(name() + " -i " + p.string()) == 0;

    case ROBOTFRAMEWORK:
      return factory::process().system(
               name() + " --separator tab " + p.string()) == 0;

    case SOURCE:
      return factory::process().system(
               name() + " -i " + p.string() +
               " --style=file --fallback-style=none") == 0;

    default:
      return false;
  }
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
  switch (m_type)
  {
    case CMAKE:
      return p.filename() == "CMakeLists.txt" || p.extension() == ".cmake";

    case ROBOTFRAMEWORK:
      return p.extension() == ".robot";

    case SOURCE:
      return p.extension() == ".c" || p.extension() == ".cpp" ||
             p.extension() == ".h" || p.extension() == ".hpp" ||
             p.extension() == ".java" || p.extension() == ".javascript";

    default:
      return false;
  }
}

wex::config::strings_t wex::factory::beautify::list() const
{
  return config::strings_t{{""}, {"clang-format"}, {"gersemi"}, {"robotidy"}};
}

const std::string wex::factory::beautify::name() const
{
  switch (m_type)
  {
    case CMAKE:
      return config("stc.beautifier.cmake").get(list()).front();

    case ROBOTFRAMEWORK:
      return config("stc.beautifier.robotframework").get(list()).front();

    case SOURCE:
      return config("stc.beautifier.sources").get(list()).front();

    default:
      return std::string();
  }
}
