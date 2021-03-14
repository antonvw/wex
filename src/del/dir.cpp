////////////////////////////////////////////////////////////////////////////////
// Name:      dir.cpp
// Purpose:   Implementation of wex::del::dir
//            and wex::del::tool_dir classes
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/listitem.h>
#include <wex/del/dir.h>
#include <wex/del/stream.h>

wex::del::dir::dir(
  wex::listview*   listview,
  const path&      fullpath,
  const data::dir& data)
  : wex::dir(fullpath, data)
  , m_listview(listview)
{
}

bool wex::del::dir::on_dir(const path& dir)
{
  listitem(m_listview, dir, data().file_spec()).insert();
  return true;
}

bool wex::del::dir::on_file(const path& file)
{
  listitem(m_listview, file, data().file_spec()).insert();
  return true;
}

wex::del::tool_dir::tool_dir(
  const tool&      tool,
  const path&      fullpath,
  const data::dir& data)
  : wex::dir(fullpath, data)
  , m_statistics()
  , m_tool(tool)
{
}

bool wex::del::tool_dir::on_file(const path& p)
{
  del::stream s(p, m_tool);

  const bool ret = s.run_tool();
  m_statistics += s.get_statistics();

  if (!ret)
  {
    cancel();
    return false;
  }

  return true;
}
