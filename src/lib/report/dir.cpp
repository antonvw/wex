////////////////////////////////////////////////////////////////////////////////
// Name:      dir.cpp
// Purpose:   Implementation of wex::report::dir
//            and wex::report::tool_dir classes
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/listitem.h>
#include <wex/report/dir.h>
#include <wex/report/stream.h>

wex::report::dir::dir(
  wex::listview*   listview,
  const path&      fullpath,
  const data::dir& data)
  : wex::dir(fullpath, data)
  , m_listview(listview)
{
}

bool wex::report::dir::on_dir(const path& dir)
{
  listitem(m_listview, dir, data().file_spec()).insert();
  return true;
}

bool wex::report::dir::on_file(const path& file)
{
  listitem(m_listview, file, data().file_spec()).insert();
  return true;
}

wex::report::tool_dir::tool_dir(
  const tool&      tool,
  const path&      fullpath,
  const data::dir& data)
  : wex::dir(fullpath, data)
  , m_statistics()
  , m_tool(tool)
{
}

bool wex::report::tool_dir::on_file(const path& p)
{
  report::stream s(p, m_tool);

  const bool ret = s.run_tool();
  m_statistics += s.get_statistics();

  if (!ret)
  {
    cancel();
    return false;
  }

  return true;
}
