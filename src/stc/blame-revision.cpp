////////////////////////////////////////////////////////////////////////////////
// Name:      stc/blame-revision.cpp
// Purpose:   Implementation of class wex::stc::blame_revision
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/core.h>
#include <wex/core/log.h>
#include <wex/stc/stc.h>
#include <wex/stc/vcs.h>
#include <wex/ui/frame.h>

void wex::stc::blame_revision(const std::string& offset)
{
  const auto& revision(margin_get_revision_id());
  auto        renamed(margin_get_revision_renamed());
  wex::vcs    vcs({m_data.head_path().empty() ? path() : m_data.head_path()});
  const auto& extra(!is_visual() ? "-L %LINES " : std::string());

  if (!renamed.empty())
  {
    m_renamed = renamed;
  }
  else
  {
    renamed = m_renamed;
  }

  log::trace("blame revision") << vcs.entry().name();

  if (vcs.entry().name() == "git")
  {
    if (!renamed.empty())
    {
      if (revision != find_before(renamed, " "))
      {
        if (
          vcs.entry().system(
            process_data()
              .args("blame " + extra + revision + offset + " -- " + renamed)
              .start_dir(vcs.toplevel().string())) != 0)
        {
          log::status("blame") << "error";
          return;
        }
      }
      else
      {
        log::status("blame") << "at oldest: " << revision;
        log::trace("blame equal") << revision << renamed;
        return;
      }
    }
    else if (
      vcs.entry().system(
        process_data()
          .args("blame " + extra + path().string() + " " + revision + offset)
          .start_dir(path().parent_path())) != 0)
    {
      log::status("blame") << "error";
      return;
    }
  }
  else if (vcs.entry().name() == "svn")
  {
    if (
      vcs.entry().system(
        process_data()
          .args("blame " + path().string() + " -v -r " + revision)
          .start_dir(path().parent_path())) != 0)
    {
      log::status("blame") << "error";
      return;
    }
  }

  if (renamed.empty())
  {
    renamed = revision + " " + path().string();
  }

  vcs.entry().get_blame().caption(
    "blame " + revision + " " + path().filename());

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
    ->open_file(wex::path(renamed), vcs.entry(), data);
}
