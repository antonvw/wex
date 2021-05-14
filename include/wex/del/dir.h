////////////////////////////////////////////////////////////////////////////////
// Name:      dir.h
// Purpose:   Include file for wex::del::dir class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/dir.h>
#include <wex/listview.h>

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
  bool on_dir(const path& dir) const final;
  bool on_file(const path& file) const final;

  wex::listview* m_listview;
};
}; // namespace wex::del
