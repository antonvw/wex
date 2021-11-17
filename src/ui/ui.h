////////////////////////////////////////////////////////////////////////////////
// Name:      ui.h
// Purpose:   Declaration of several ui lib methods
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/aui/auibook.h>
#include <wx/bookctrl.h>
#include <wx/listbook.h>
#include <wx/notebook.h>
#include <wx/persist/treebook.h>
#include <wx/simplebook.h>
#include <wx/toolbook.h>

#include <wex/data/item.h>
#include <wex/ui/notebook.h>

namespace wex
{
template <typename T>
void create_book_control(
  wxWindow*              parent,
  wxWindow*&             win,
  const wex::data::item& data)
{
  auto* bookctrl = new T(
    parent,
    data.window().id(),
    data.window().pos(),
    data.window().size(),
    data.window().style());

  win = bookctrl;

  if (data.image_list() != nullptr)
  {
    bookctrl->SetImageList(data.image_list());
  }
}
} // namespace wex
