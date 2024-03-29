////////////////////////////////////////////////////////////////////////////////
// Name:      path-lexer.cpp
// Purpose:   Implementation of class wex::path_lexer
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/config.h>
#include <wex/core/log.h>
#include <wex/factory/frame.h>
#include <wex/factory/process.h>
#include <wex/syntax/lexers.h>
#include <wex/syntax/path-lexer.h>
#include <wx/app.h>

namespace fs = std::filesystem;

namespace wex
{
enum class build_system_t
{
  MAKE,
  NINJA,
  OTHER,
};

std::map<build_system_t, std::string> binary{
  {build_system_t::MAKE, "make"},
  {build_system_t::NINJA, "ninja"}};

std::map<build_system_t, std::string> switches{
  {build_system_t::MAKE, "-f"},
  {build_system_t::NINJA, ""}};

build_system_t check_build_system(const path_lexer& p)
{
  if (p.lexer().scintilla_lexer() == "makefile")
  {
    return build_system_t::MAKE;
  }
  else if (p.lexer().display_lexer() == "ninja")
  {
    return build_system_t::NINJA;
  }
  else
  {
    log("unsupported build system") << p.lexer().scintilla_lexer();
    return build_system_t::OTHER;
  }
}

bool build(const path_lexer& p)
{
  if (auto* frame =
        dynamic_cast<wex::factory::frame*>(wxTheApp->GetTopWindow());
      frame != nullptr)
  {
    if (const auto t(check_build_system(p)); t != build_system_t::OTHER)
    {
      const auto& sw(config("build." + binary[t] + ".switch").get(switches[t]));
      return frame->process_async_system(
        process_data(
          config("build." + binary[t] + ".bin").get(binary[t]) + " " +
          (!sw.empty() ? sw + " " + p.filename() : std::string()))
          .start_dir(p.parent_path()));
    }
  }

  return false;
}

const std::string lexer_string(const std::string& filename)
{
  auto* l = lexers::get(false);
  return l != nullptr && !l->get_lexers().empty() ?
           l->find_by_filename(filename).display_lexer() :
           std::string();
}
}; // namespace wex

wex::path_lexer::path_lexer(const std::string& p)
  : path(p)
  , m_lexer(lexer_string(p))
{
}

wex::path_lexer::path_lexer(const path& p)
  : path(p)
  , m_lexer(lexer_string(p.filename()))
{
}

bool wex::path_lexer::is_build() const
{
  return m_lexer.scintilla_lexer() == "makefile" ||
         m_lexer.display_lexer() == "ninja";
}
