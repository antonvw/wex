////////////////////////////////////////////////////////////////////////////////
// Name:      stc/blame-revision.cpp
// Purpose:   Implementation of class wex::stc::blame_revision
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/stc/stc.h>
#include <wex/ui/frame.h>

void wex::stc::blame_revision(const std::string& offset)
{
  if (const auto& renamed(margin_get_revision_renamed()); !renamed.empty())
  {
    m_renamed = renamed;
  }

  m_frame->vcs_blame_revision(this, m_renamed, offset);
}
