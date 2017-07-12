////////////////////////////////////////////////////////////////////////////////
// Name:      dirctrl.h
// Purpose:   Declaration of class wxExGenericDirCtrl
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/generic/dirctrlg.h>
#include <wx/extension/window-data.h>

#if wxUSE_DIRDLG

class wxExFrameWithHistory;

/// Offers our generic dir control.
/// It adds a popup menu and handling of the commands.
class WXDLLIMPEXP_BASE wxExGenericDirCtrl : public wxGenericDirCtrl
{
public:
  /// Default constructor.
  wxExGenericDirCtrl(
    wxExFrameWithHistory* frame,
    const wxString& filter = wxEmptyString, 
    int defaultFilter = 0,
    const wxExWindowData& data = wxExWindowData().
      Style(wxDIRCTRL_3D_INTERNAL | wxDIRCTRL_MULTIPLE));

  /// Expands path and selects it.
  void ExpandAndSelectPath(const wxExPath& path);
};
#endif // wxUSE_DIRDLG
