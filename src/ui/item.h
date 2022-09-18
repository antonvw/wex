////////////////////////////////////////////////////////////////////////////////
// Name:      item.h
// Purpose:   Several item include files
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/checklst.h>
#include <wx/choicebk.h>
#include <wx/clrpicker.h> // for wxColourPickerWidget
#include <wx/combobox.h>
#include <wx/commandlinkbutton.h>
#include <wx/filepicker.h>
#include <wx/fontpicker.h>
#include <wx/hyperlink.h>
#include <wx/imaglist.h>
#include <wx/radiobox.h>
#include <wx/spinctrl.h>
#include <wx/statbox.h>
#include <wx/statline.h>
#include <wx/stattext.h>
#include <wx/tglbtn.h>
#include <wx/valnum.h>
#include <wx/valtext.h>
#include <wx/window.h>

#include <wex/core/core.h>
#include <wex/ui/grid.h>
#include <wex/ui/item.h>
#include <wex/ui/listview.h>

namespace wex
{
const std::any get_value_prim(const wex::item* item);

template <typename T> std::any get_value_simple(wxWindow* window)
{
  return (reinterpret_cast<T>(window))->GetValue();
}

bool get_value_simple(wex::item::type_t t, wxWindow* window, std::any& any);

bool no_value(wex::item::type_t t);

const std::string str(const std::string& name, const std::any& any);

const item::type_t use_type(const std::string& label, item::type_t t);
} // namespace wex
