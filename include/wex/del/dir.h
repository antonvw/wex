////////////////////////////////////////////////////////////////////////////////
// Name:      dir.h
// Purpose:   Include file for wex::del::dir
//            and wex::del::tool_dir classes
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/dir.h>
#include <wex/listview.h>
#include <wex/stream-statistics.h>
#include <wex/tool.h>

namespace wex::del
{
  /// Offers a dir with reporting to a listview.
  /// All matching files and folders are added as listitem to the listview.
  /// This one is used e.g. by listview_file to add items.
  class dir : public wex::dir
  {
  public:
    /// Constructor, provide your listview and a path.
    dir(
      wex::listview*   listview,
      const path&      fullpath,
      const data::dir& data = data::dir());

  private:
    bool on_dir(const path& dir) final;
    bool on_file(const path& file) final;

    wex::listview* m_listview;
  };

  /// Offers a dir with tool support.
  /// run_tool is find_files invoked on all matching files.
  /// This one is used e.g. by grep / sed to find in files,
  /// in combination with wex::stream.
  class tool_dir : public wex::dir
  {
  public:
    /// Constructor, provide your tool and a path.
    /// setup_tool should already be called.
    tool_dir(
      const tool&      tool,
      const path&      fullpath,
      const data::dir& data = data::dir());

    /// Returns the statistics.
    auto& get_statistics() const { return m_statistics; };

  private:
    bool on_file(const path& file) final;

    stream_statistics m_statistics;
    const tool        m_tool;
  };
}; // namespace wex::del
