////////////////////////////////////////////////////////////////////////////////
// Name:      dir.cpp
// Purpose:   Implementation of wex::del::dir
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/del/dir.h>
#include <wex/listitem.h>
#include <wex/log.h>

wex::del::dir::dir(
  wex::listview*   listview,
  const path&      fullpath,
  const data::dir& data)
  : wex::dir(fullpath, data)
  , m_listview(listview)
{
}

bool wex::del::dir::on_dir(const path& dir) const
{
  listitem(m_listview, dir, data().file_spec()).insert();
  return true;
}

bool wex::del::dir::on_file(const path& file) const
{
  listitem(m_listview, file, data().file_spec()).insert();
  return true;
}
