////////////////////////////////////////////////////////////////////////////////
// Name:      stc/blame-revision.cpp
// Purpose:   Implementation of class wex::stc::blame_revision
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/stc/stc.h>
#include <wex/ui/frame.h>

#include "blaming.h"

void wex::stc::blame_revision(const std::string& offset)
{
  if (const auto& renamed(margin_get_revision_renamed()); !renamed.empty())
  {
    m_renamed = renamed;
  }

  blaming bl(
    {{m_data.head_path().empty() ? path() : m_data.head_path()}},
    offset,
    margin_get_revision_id(),
    m_renamed,
    !is_visual() ? "-L %LINES " : std::string());

  if (!bl.execute(path()))
  {
    return;
  }

  data::stc data(m_data);
  data.control().line(m_margin_text_click + 1);
  data::window window(m_data.control().window());
  window.name(std::string());
  data.control().window(window);

  if (data.head_path().empty())
  {
    data.head_path(path());
  }

  ((wex::factory::frame*)m_frame)
    ->open_file(wex::path(bl.renamed()), bl.vcs().entry(), data);
}
