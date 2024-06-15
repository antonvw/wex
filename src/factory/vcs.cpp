////////////////////////////////////////////////////////////////////////////////
// Name:      vcs.cpp
// Purpose:   Declaration of class wex::vcs_admin
// Author:    Anton van Wezenbeek
// Copyright: (c) 2008-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/factory/vcs.h>

wex::factory::vcs_admin::vcs_admin(const std::string& dir, const path& p)
  : m_dir(dir)
  , m_path(p)
{
}

bool wex::factory::vcs_admin::exists() const
{
  return !m_dir.empty() && !m_path.empty() &&
         path(m_path).append(path(m_dir)).dir_exists();
}

bool wex::factory::vcs_admin::is_toplevel() const
{
  return !toplevel().empty();
}

wex::path wex::factory::vcs_admin::toplevel() const
{
  if (m_dir.empty() || m_path.empty())
  {
    return path();
  }

  // .git
  // /home/user/wex/src/src/vi.cpp
  // should return -> /home/user/wex
  for (path root; const auto& part : m_path.data())
  {
    if (vcs_admin(m_dir, root.append(part)).exists())
    {
      return root;
    }
  }

  return path();
}
