////////////////////////////////////////////////////////////////////////////////
// Name:      unified-diff.cpp
// Purpose:   Implementation of class wex::unified_diff (for git)
//            https://git-scm.com/docs/diff-format
// Author:    Anton van Wezenbeek
// Copyright: (c) 2024-2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/log.h>
#include <wex/factory/frame.h>
#include <wex/vcs/unified-diff.h>
#include <wex/vcs/vcs-entry.h>
#include <wex/vcs/vcs.h>

wex::unified_diff::unified_diff(
  const path&      p,
  const vcs_entry* e,
  factory::frame*  f)
  : factory::unified_diff(e->std_out(), f)
  , m_path_vcs(p)
  , m_frame(f)
  , m_path_toplevel(vcs().toplevel())
  , m_vcs_entry(e)
{
}

bool wex::unified_diff::report_diff()
{
  m_path_vcs = path(m_path_toplevel).append(m_path[0]);

  if (!m_path_vcs.dir_exists())
  {
    if (!m_path_vcs.file_exists())
    {
      log("unified_diff") << m_path_vcs.string() << "does not exist";
      return false;
    }

    if (!m_frame->report_unified_diff(this))
    {
      return false;
    }
  }

  return true;
}

std::string wex::unified_diff::token_from() const
{
  return "a/";
}

std::string wex::unified_diff::token_to() const
{
  return "b/";
}
