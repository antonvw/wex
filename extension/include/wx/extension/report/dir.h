////////////////////////////////////////////////////////////////////////////////
// Name:      dir.h
// Purpose:   Include file for wex::listview_dir and wex::tool_dir classes
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/extension/dir.h>
#include <wx/extension/stream-statistics.h>
#include <wx/extension/tool.h>

namespace wex
{
  /// Offers a dir with tool support.
  /// RunTool is FindFiles invoked on all matching files.
  class tool_dir : public dir
  {
  public:
    /// Constructor, provide your tool and a path.
    /// SetupTool should already be called.
    tool_dir(const tool& tool,
      const path& fullpath,
      const std::string& filespec = std::string(),
      int flags = DIR_DEFAULT);
      
    /// Returns the statistics.
    auto & GetStatistics() {return m_Statistics;};
  protected:  
    virtual bool OnFile(const path& file) override;
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
      int flags = DIR_DEFAULT);
  protected:
    virtual bool OnDir(const path& dir) override;
    virtual bool OnFile(const path& file) override;
  private:
    listview* m_ListView;
  };
};
