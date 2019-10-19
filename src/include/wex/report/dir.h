////////////////////////////////////////////////////////////////////////////////
// Name:      dir.h
// Purpose:   Include file for wex::report::dir and wex::tool_dir classes
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/dir.h>
#include <wex/listview.h>
#include <wex/stream-statistics.h>
#include <wex/tool.h>

namespace wex
{
  /// Offers a dir with tool support.
  /// run_tool is find_files invoked on all matching files.
  class tool_dir : public dir
  {
  public:
    /// Constructor, provide your tool and a path.
    /// setup_tool should already be called.
    tool_dir(const tool& tool,
      const path& fullpath,
      const std::string& filespec = std::string(),
      dir::type_t flags = dir::type_t().set());
      
    /// Returns the statistics.
    auto & get_statistics() {return m_statistics;};
  protected:  
    bool on_file(const path& file) override;
  private:    
    stream_statistics m_statistics;
    const tool m_tool;
  };
};

namespace wex::report
{
  /// Offers a dir with reporting to a listview.
  /// All matching files and folders are added as listitem to the listview.
  class dir : public wex::dir
  {
  public:
    /// Constructor, provide your listview and a path.
    dir(wex::listview* listview,
      const path& fullpath,
      const std::string& filespec = std::string(),
      dir::type_t flags = dir::type_t().set());
  protected:
    bool on_dir(const path& dir) override;
    bool on_file(const path& file) override;
  private:
    wex::listview* m_listview;
  };
};
