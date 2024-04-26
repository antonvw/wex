////////////////////////////////////////////////////////////////////////////////
// Name:      blaming.cpp
// Purpose:   Implementation of class wex::blaming
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/core.h>
#include <wex/core/log.h>
#include <wex/data/stc.h>
#include <wex/factory/stc.h>

#include "blaming.h"

namespace wex
{
const std::vector<path> paths(factory::stc* stc)
{
  const auto& p(stc->get_data()->head_path());
  if (p.empty())
  {
    return {{stc->path()}};
  }

  return {{p}};
}
} // namespace wex

wex::blaming::blaming(factory::stc* stc, const std::string& offset)
  : m_offset(offset)
  , m_revision(stc->margin_get_revision_id())
  , m_renamed(stc->vcs_renamed())
  , m_vcs(paths(stc))
  , m_range(!stc->is_visual() ? "-L %LINES " : std::string())
{
}

bool wex::blaming::error(const std::string& msg)
{
  log::status("blame") << msg << m_vcs.entry().std_err();
  return false;
}

bool wex::blaming::execute(const path& path)
{
  log::trace("blame revision") << m_vcs.entry().name() << " id: " << m_revision;

  m_path = path;

  m_vcs.entry().get_blame().caption(
    "blame " + m_revision + " " + m_path.filename());

  return (m_vcs.entry().name() == "git" && exec_git()) ||
         (m_vcs.entry().name() == "svn" && exec_svn());
}

bool wex::blaming::exec_git()
{
  if (!m_renamed.empty())
  {
    if (m_revision != find_before(m_renamed, " "))
    {
      if (
        m_vcs.entry().system(
          process_data()
            .args(
              "blame " + m_range + m_revision + m_offset + " -- " + m_renamed)
            .start_dir(m_vcs.toplevel().string())) != 0)
      {
        return error();
      }
    }
    else
    {
      log::trace("blame equal") << m_revision << m_renamed;
      return error("at oldest: " + m_revision);
    }
  }
  else if (
    m_vcs.entry().system(
      process_data()
        .args(
          "blame " + m_range + m_path.string() + " " + m_revision + m_offset)
        .start_dir(m_path.parent_path())) != 0)
  {
    return error();
  }

  if (m_renamed.empty())
  {
    m_renamed = m_revision + " " + m_path.string();
  }

  return true;
}

bool wex::blaming::exec_svn()
{
  if (
    m_vcs.entry().system(
      process_data()
        .args("blame " + m_path.string() + " -v -r " + m_revision)
        .start_dir(m_path.parent_path())) != 0)
  {
    return error();
  }

  return true;
}
