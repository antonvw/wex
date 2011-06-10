////////////////////////////////////////////////////////////////////////////////
// Name:      toolbar.h
// Purpose:   Declaration of wxExToolBar classes
// Author:    Anton van Wezenbeek
// RCS-ID:    $Id$
// Created:   2010-03-26
// Copyright: (c) 2010 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXTOOLBAR_H
#define _EXTOOLBAR_H

#include <wx/aui/auibar.h> 

// Only if we have a gui.
#if wxUSE_GUI

class wxExFrame;
class wxExTextCtrl;

/// Offers a toolbar together with stock art.
/// Default no controls are added, you have to call AddControls to do that.
class WXDLLIMPEXP_BASE wxExToolBar : public wxAuiToolBar
{
public:
  /// Constructor.
  wxExToolBar(wxExFrame* frame, 
    wxWindowID id = wxID_ANY,
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    long style = wxAUI_TB_DEFAULT_STYLE);

  /// Adds automatic naming (for stock menu id's) and 
  /// art id for toolbar normal items.
  wxAuiToolBarItem* AddTool(int toolId,
    const wxString& label = wxEmptyString,
    const wxBitmap& bitmap = wxNullBitmap,
    const wxString& shortHelp = wxEmptyString,
    wxItemKind kind = wxITEM_NORMAL);

  /// Adds the standard controls.
  /// This adds a file open, save and print control, and
  /// the hex mode and sync check boxes.
  void AddControls();
  
  /// Gets the frame.
  wxExFrame* GetFrame() {return m_Frame;};
protected:
  void OnCommand(wxCommandEvent& event);
private:
  wxCheckBox* m_HexMode;
  wxCheckBox* m_SyncMode;
  wxExFrame* m_Frame;

  DECLARE_EVENT_TABLE()
};

/// Offers a find toolbar, containing a find ctrl, up and down arrows
/// and checkboxes.
/// The find ctrl allows you to find in an wxExSTC
/// component on the specified wxExFrame.
class WXDLLIMPEXP_BASE wxExFindToolBar : public wxExToolBar
{
public:
  /// Constructor.
  wxExFindToolBar(wxExFrame* frame, 
    wxWindowID id = wxID_ANY,
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    long style = wxAUI_TB_DEFAULT_STYLE);
protected:
  void OnCommand(wxCommandEvent& event);
  void OnUpdateUI(wxUpdateUIEvent& event);
private:
  void Initialize();

  wxExTextCtrl* m_FindCtrl;
  wxCheckBox* m_IsRegularExpression;
  wxCheckBox* m_MatchCase;
  wxCheckBox* m_MatchWholeWord;

  DECLARE_EVENT_TABLE()
};
#endif // wxUSE_GUI
#endif
