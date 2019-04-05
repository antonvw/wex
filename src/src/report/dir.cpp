////////////////////////////////////////////////////////////////////////////////
// Name:      dir.cpp
// Purpose:   Implementation of wex::report::dir and wex::tool_dir classes
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/listitem.h>
#include <wex/report/dir.h>
#include <wex/report/stream.h>

wex::tool_dir::tool_dir(
  const tool& tool,
  const path& fullpath, 
  const std::string& filespec, 
  dir::type_t flags)
  : dir(fullpath, filespec, std::string(), flags)
  , m_Statistics()
  , m_Tool(tool)
{
}

bool wex::tool_dir::on_file(const path& file)
{
  report::stream report(file, m_Tool);

  bool ret = report.run_tool();
  m_Statistics += report.get_statistics();

  if (!ret)
  {
    cancel();
    return false;
  }

  return true;
}

wex::report::dir::dir(
  wex::listview* listview,
  const path& fullpath, 
  const std::string& filespec, 
  dir::type_t flags)
  : wex::dir(fullpath, filespec, std::string(), flags)
  , m_ListView(listview)
{
}

bool wex::report::dir::on_dir(const path& dir)
{
  listitem(m_ListView, dir, file_spec()).insert();
  return true;
}

bool wex::report::dir::on_file(const path& file)
{
  listitem(m_ListView, file, file_spec()).insert();
  return true;
}
