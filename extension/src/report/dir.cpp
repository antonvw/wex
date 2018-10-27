////////////////////////////////////////////////////////////////////////////////
// Name:      dir.cpp
// Purpose:   Implementation of wex::listview_dir and wex::tool_dir classes
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/listitem.h>
#include <wex/report/dir.h>
#include <wex/report/stream.h>

wex::tool_dir::tool_dir(const tool& tool,
  const path& fullpath, const std::string& filespec, int flags)
  : dir(fullpath, filespec, flags)
  , m_Statistics()
  , m_Tool(tool)
{
}

bool wex::tool_dir::OnFile(const path& file)
{
  listview_stream report(file, m_Tool);

  bool ret = report.RunTool();
  m_Statistics += report.GetStatistics();

  if (!ret)
  {
    Cancel();
    return false;
  }

  return true;
}

wex::listview_dir::listview_dir(listview* listview,
  const path& fullpath, const std::string& filespec, int flags)
  : dir(fullpath, filespec, flags)
  , m_ListView(listview)
{
}

bool wex::listview_dir::OnDir(const path& dir)
{
  listitem(m_ListView, dir, GetFileSpec()).Insert();
  return true;
}

bool wex::listview_dir::OnFile(const path& file)
{
  listitem(m_ListView, file, GetFileSpec()).Insert();
  return true;
}
