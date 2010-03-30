////////////////////////////////////////////////////////////////////////////////
// Name:      statusbar.h
// Purpose:   Declaration of wxExStatusBar class
// Author:    Anton van Wezenbeek
// RCS-ID:    $Id$
// Created:   2010-03-26
// Copyright: (c) 2010 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXSTATUSBAR_H
#define _EXSTATUSBAR_H

#include <map>
#include <vector>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/statusbr.h> 

// Only if we have a gui.
#if wxUSE_GUI
#if wxUSE_STATUSBAR

class wxExFrame;
class wxExStatusBar;

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
#endif // wxUSE_GUI
#endif
