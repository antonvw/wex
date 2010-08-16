////////////////////////////////////////////////////////////////////////////////
// Name:      dirctrl.h
// Purpose:   Declaration of class 'wxExGenericDirCtrl'
// Author:    Anton van Wezenbeek
// Created:   2010-08-16
// RCS-ID:    $Id$
// Copyright: (c) 2010 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _DIRCTRL_H
#define _DIRCTRL_H

#include <wx/generic/dirctrlg.h>

class wxExFrameWithHistory;

/// Offers our generic dir control.
class wxExGenericDirCtrl : public wxGenericDirCtrl
{
public:
  /// Constructor.
  wxExGenericDirCtrl(
    wxWindow* parent, 
    wxExFrameWithHistory* frame,
    const wxWindowID id = wxID_ANY, 
    const wxString& dir = wxDirDialogDefaultFolderStr, 
    const wxPoint& pos = wxDefaultPosition, 
    const wxSize& size = wxDefaultSize, 
    long style = wxDIRCTRL_3D_INTERNAL, 
    const wxString& filter = wxEmptyString, 
    int defaultFilter = 0, 
    const wxString& name = wxTreeCtrlNameStr);
protected:
  void OnCommand(wxCommandEvent& event);
  void OnTree(wxTreeEvent& event);
private:
  wxExFrameWithHistory* m_Frame;
  
  DECLARE_EVENT_TABLE()
};
#endif
