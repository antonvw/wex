////////////////////////////////////////////////////////////////////////////////
// Name:      unified-diff.cpp
// Purpose:   Implementation of class wex::unified_diff
//            https://www.gnu.org/software/diffutils/manual/html_node/Detailed-Unified.html
// Author:    Anton van Wezenbeek
// Copyright: (c) 2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/log.h>
#include <wex/factory/frame.h>
#include <wex/vcs/unified-diff.h>
#include <wex/vcs/vcs-entry.h>
#include <wex/vcs/vcs.h>

namespace wex
{
int stoi(const std::string& i)
{
  return i.empty() ? 1 : std::stoi(i);
}
} // namespace wex

wex::unified_diff::unified_diff(
  const path&      p,
  const vcs_entry* e,
  factory::frame*  f)
  : factory::unified_diff(e->std_out())
  , m_path_vcs(p)
  , m_frame(f)
  , m_path_toplevel(vcs().toplevel())
  , m_vcs_entry(e)
{
  m_frame->page_save();
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

    if (!m_frame->vcs_unified_diff(m_vcs_entry, this))
    {
      return false;
    }
  }

  return true;
}

void wex::unified_diff::report_diff_finish()
{
  m_frame->page_restore();
}
