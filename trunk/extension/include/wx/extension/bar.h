////////////////////////////////////////////////////////////////////////////////
// Name:      bar.h
// Purpose:   Declaration of bar classes
// Author:    Anton van Wezenbeek
// RCS-ID:    $Id$
// Created:   2010-03-26
// Copyright: (c) 2010 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXBAR_H
#define _EXBAR_H

#include <map>
#include <vector>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/statusbr.h> 

// Only if we have a gui.
#if wxUSE_GUI

class wxExFrame;
class wxExStatusBar;

#if wxUSE_STATUSBAR
/// This class defines our statusbar panes, to be used by wxExFrame::SetupStatusBar.
/// It just adds some members to the base class, and keeps a static total.
class wxExPane : public wxStatusBarPane
{
  friend class wxExStatusBar;
public:
  /// Default constructor.
  wxExPane(
    /// If you do no provide helptext, it is derived from the name, by using
    /// text after the first 'e' character (so after 'Pane') if name is
    /// not 'PaneText'.
    const wxString& name = wxEmptyString,
    /// Width of the pane.
    int width = 50,
    /// The helptext shown as a tooltip.
    const wxString& helptext = wxEmptyString,
    /// The style.
    int style = wxSB_NORMAL)
    : wxStatusBarPane(style, width)
    , m_Helptext(
        helptext.empty() && name != "PaneText" ? 
          name.AfterFirst('e'): helptext)
    , m_Name(name)
    , m_No(m_Total)
    {m_Total++;};
private:
  wxString m_Helptext;
  wxString m_Name;
  int m_No;
  static int m_Total;
};
#endif // wxUSE_STATUSBAR

#if wxUSE_STATUSBAR
/// Offers a status bar with popup menu in relation to wxExFrame.
class wxExStatusBar : public wxStatusBar
{
public:
  /// Constructor.
  wxExStatusBar(wxExFrame* parent,
    wxWindowID id = wxID_ANY,
    long style = wxST_SIZEGRIP,
    const wxString& name = wxStatusBarNameStr);

  /// Sets the panes.
  void SetPanes(const std::vector<wxExPane>& panes);

  /// Sets text on specified pane.
  void SetStatusText(
    const wxString& text, 
    const wxString& pane = "PaneText");
protected:
  void OnMouse(wxMouseEvent& event);
private:
  /// Returns the status bar pane.
  /// If pane could not be found, returns empty pane.
  const wxExPane GetPane(int pane) const;

  wxExFrame* m_Frame;
  std::map<wxString, wxExPane> m_Panes;

  DECLARE_EVENT_TABLE()
};
#endif // wxUSE_STATUSBAR

#if wxUSE_TOOLBAR
/// Offers a toolbar together with stock art.
class wxExToolBar : public wxToolBar
{
public:
  /// Constructor.
  wxExToolBar(wxWindow* parent,
    wxWindowID id = wxID_ANY,
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    long style = wxTB_HORIZONTAL,
    const wxString& name = wxToolBarNameStr);

  /// Adds automatic naming (for stock menu id's) and 
  /// art id for toolbar normal items.
  wxToolBarToolBase* AddTool(int toolId,
    const wxString& label = wxEmptyString,
    const wxBitmap& bitmap = wxNullBitmap,
    const wxString& shortHelp = wxEmptyString,
    wxItemKind kind = wxITEM_NORMAL);
};
#endif // wxUSE_TOOLBAR

#if wxUSE_TOOLBAR
/// Offers a find toolbar, containing a find combobox, up and down arrows
/// and checkboxes.
/// The find combobox allows you to find in an wxExSTCFile
/// component on the specified wxExFrame.
class wxExFindToolBar : public wxExToolBar
{
public:
  /// Constructor.
  wxExFindToolBar(
    wxWindow* parent, 
    wxExFrame* frame, 
    wxWindowID id = wxID_ANY);
protected:
  void OnCommand(wxCommandEvent& event);
  void OnUpdateUI(wxUpdateUIEvent& event);
private:
  void Initialize();

  wxCheckBox* m_RegularExpression;
  wxCheckBox* m_MatchCase;
  wxCheckBox* m_MatchWholeWord;
  wxComboBox* m_ComboBox;
  wxExFrame* m_Frame;

  DECLARE_EVENT_TABLE()
};
#endif // wxUSE_TOOLBAR
#endif // wxUSE_GUI
#endif
