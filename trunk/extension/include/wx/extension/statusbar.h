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
#include <wx/statusbr.h> 

// Only if we have a gui.
#if wxUSE_GUI
#if wxUSE_STATUSBAR

class wxExFrame;
class wxExStatusBar;

/// This class defines our statusbar panes, used by wxExFrame::SetupStatusBar.
/// It just adds some members to the base class, and keeps a static total.
class WXDLLIMPEXP_BASE wxExPane : public wxStatusBarPane
{
public:
  /// Default constructor.
  wxExPane() : m_No(-1){;};

  /// Constructor.
  wxExPane(
    /// If you do no provide helptext, it is derived from the name, by using
    /// text after the first 'e' character (so after 'Pane') if name is
    /// not 'PaneText'.
    const wxString& name,
    /// Width of the pane.
    int width = 50,
    /// The helptext shown as a tooltip.
    const wxString& helptext = wxEmptyString,
    /// The style.
    int style = wxSB_NORMAL)
    : wxStatusBarPane(style, width)
    , m_HelpText(
        helptext.empty() && name != "PaneText" ? 
          name.AfterFirst('e'): helptext)
    , m_Name(name)
    , m_No(m_Total)
    {m_Total++;};
  const wxString& GetHelpText() const {return m_HelpText;};
  const wxString& GetName() const {return m_Name;};
  int GetNo() const {return m_No;};
private:
  wxString m_HelpText;
  wxString m_Name;
  int m_No;
  static int m_Total;
};

/// Offers a status bar that calls virtual methods from wxExFrame.
class wxExStatusBar : public wxStatusBar
{
public:
  /// Constructor.
  wxExStatusBar(wxExFrame* parent,
    wxWindowID id = wxID_ANY,
    long style = wxST_SIZEGRIP,
    const wxString& name = wxStatusBarNameStr);
    
  /// Destructor.
 ~wxExStatusBar();  

  /// Sets the panes.
  /// Returns the number of panes added.
  int SetPanes(const std::vector<wxExPane>& panes);

  /// Sets text on specified pane.
  /// Returns true if the specified pane exists.
  bool SetStatusText(
    const wxString& text, 
    const wxString& pane = "PaneText");
protected:
  /// React on some mouse events line button down, double click and
  /// moving over.
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
