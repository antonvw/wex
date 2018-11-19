////////////////////////////////////////////////////////////////////////////////
// Name:      dir.h
// Purpose:   Include file for wex::listview_dir and wex::tool_dir classes
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/dir.h>
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
    auto & get_statistics() {return m_Statistics;};
  protected:  
    virtual bool on_file(const path& file) override;
  private:    
    stream_statistics m_Statistics;
    const tool m_Tool;
  };

  class listview;

  /// Offers a dir with reporting to a listview.
  /// All matching files and folders are added as listitem to the listview.
  class listview_dir : public dir
  {
  public:
    /// Constructor, provide your listview and a path.
    listview_dir(listview* listview,
      const path& fullpath,
      const std::string& filespec = std::string(),
      dir::type_t flags = dir::type_t().set());
  protected:
    virtual bool on_dir(const path& dir) override;
    virtual bool on_file(const path& file) override;
  private:
    listview* m_ListView;
  };
};
