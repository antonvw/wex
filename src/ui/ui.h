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

#include <wex/ui/item.h>
#include <wex/ui/notebook.h>

namespace wex
{
template <typename T>
void create_book_control(
  wxWindow*        parent,
  wxWindow*&       win,
  const wex::item& item)
{
  auto* bookctrl = new T(
    parent,
    item.data().window().id(),
    item.data().window().pos(),
    item.data().window().size(),
    item.data().window().style());

  win = bookctrl;

  if (item.data().image_list() != nullptr)
  {
    bookctrl->SetImageList(item.data().image_list());
  }
}
} // namespace wex
