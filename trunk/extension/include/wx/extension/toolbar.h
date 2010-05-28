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
#if wxUSE_AUI

class wxExFrame;

/// Offers a toolbar together with stock art.
class wxExToolBar : public wxAuiToolBar
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

  /// Adds standard controls.
  void AddControls();
protected:
  void OnCommand(wxCommandEvent& event);
  wxExFrame* m_Frame;
private:
  wxCheckBox* m_HexMode;
  wxCheckBox* m_SyncMode;

  DECLARE_EVENT_TABLE()
};

/// Offers a find toolbar, containing a find combobox, up and down arrows
/// and checkboxes.
/// The find combobox allows you to find in an wxExSTCFile
/// component on the specified wxExFrame.
class wxExFindToolBar : public wxExToolBar
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

  wxCheckBox* m_IsRegularExpression;
  wxCheckBox* m_MatchCase;
  wxCheckBox* m_MatchWholeWord;
  wxComboBox* m_FindStrings;

  DECLARE_EVENT_TABLE()
};
#endif // wxUSE_AUI
#endif // wxUSE_GUI
#endif
