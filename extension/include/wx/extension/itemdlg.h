////////////////////////////////////////////////////////////////////////////////
// Name:      itemdlg.h
// Purpose:   Declaration of wxExItemDialog class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/extension/item.h>
#include <wx/extension/itemtpldlg.h>

#if wxUSE_GUI
/// Offers an item dialog.
class WXDLLIMPEXP_BASE wxExItemDialog: public wxExItemTemplateDialog <wxExItem>
{
public:
  /// Constructor.
  wxExItemDialog(
    wxWindow* parent,
    const std::vector< wxExItem >& v,
    const wxString& title = _("Options"),
    int rows = 0,
    int cols = -1,
    long flags = wxOK | wxCANCEL,
    wxWindowID id = wxID_ANY,
    int bookctrl_style = 0,
    wxImageList* imageList = nullptr,
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize, 
    long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER,
    const wxString& name = "wxExItemDialog")
  : wxExItemTemplateDialog(parent, v, title, rows, cols, flags, id, bookctrl_style,
      imageList, pos, size, style, name) {};
};
#endif // wxUSE_GUI
