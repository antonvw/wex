////////////////////////////////////////////////////////////////////////////////
// Name:      dirctrl.h
// Purpose:   Declaration of class wxExGenericDirCtrl
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _DIRCTRL_H
#define _DIRCTRL_H

#include <wx/generic/dirctrlg.h>

#if wxUSE_DIRDLG

class wxExFrameWithHistory;

/// Offers our generic dir control.
/// It adds a popup menu and handling of the commands.
class WXDLLIMPEXP_BASE wxExGenericDirCtrl : public wxGenericDirCtrl
{
public:
  /// Constructor.
  wxExGenericDirCtrl(
    wxWindow* parent, 
    wxExFrameWithHistory* frame,
    const wxWindowID id = wxID_ANY, 
    const wxPoint& pos = wxDefaultPosition, 
    const wxSize& size = wxDefaultSize, 
    long style = wxDIRCTRL_3D_INTERNAL | wxDIRCTRL_MULTIPLE,
    const wxString& filter = wxEmptyString, 
    int defaultFilter = 0, 
    const wxString& name = wxTreeCtrlNameStr);
  /// Expands path and selects it.
  void ExpandAndSelectPath(const wxString& path);
protected:
  void OnCommand(wxCommandEvent& event);
  void OnTree(wxTreeEvent& event);
private:
  wxExFrameWithHistory* m_Frame;
  
  DECLARE_EVENT_TABLE()
};
#endif // wxUSE_DIRDLG
#endif
