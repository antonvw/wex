////////////////////////////////////////////////////////////////////////////////
// Name:      test.cpp
// Purpose:   Implementation of general test functions.
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/ex.h>
#include <wex/macros.h>

#include "test.h"

const std::string add_pane(wex::frame* frame, wxWindow* pane)
{
  static int no = 0;

  const auto& info(
    frame->panes() == 5 ? wxAuiPaneInfo().Center() : wxAuiPaneInfo().Bottom());

  const std::string name("PANE " + std::to_string(no++));

  frame->pane_add(
    {{pane, wxAuiPaneInfo(info).Name(name).MinSize(250, 200).Caption(name)}});

  return name;
}

std::vector<std::string> get_builtin_variables()
{
  std::vector<std::string> v;

  for (const auto i : wex::ex::get_macros().get_variables())
  {
    if (i.second.is_builtin())
    {
      v.push_back(i.first);
    }
  }

  return v;
}
