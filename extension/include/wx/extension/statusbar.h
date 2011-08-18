////////////////////////////////////////////////////////////////////////////////
// Name:      statusbar.h
// Purpose:   Declaration of wxExStatusBar class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011 Anton van Wezenbeek
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

/// This class defines our statusbar panes, used by wxExFrame::SetupStatusBar.
/// It just adds some members to the base class, and keeps a static total.
class WXDLLIMPEXP_BASE wxExStatusBarPane : public wxStatusBarPane
{
public:
  /// Default constructor.
  /// This constructs the pane text field.
  wxExStatusBarPane(
    int width = -3,
    int style = wxSB_NORMAL)
    : wxStatusBarPane(style, width)
    , m_HelpText()
    , m_Name()
    , m_No(m_Total)
    {m_Total++;};
  
  /// Constructor.
  wxExStatusBarPane(
    /// The name of the pane.
    /// The wxextension lib uses PaneLexer, PaneInfo, PaneFileType, 
    /// by setting up one of these panes,
    /// youre panes will get controlled by the lib.
    const wxString& name,
    /// Width of the field
    int width = 50,
    /// The helptext shown as a tooltip.
    /// If you do no provide helptext, it is derived from the name, by using
    /// text after the first 'e' character (so after 'Pane').
    const wxString& helptext = wxEmptyString,
    /// The style.
    int style = wxSB_NORMAL)
    : wxStatusBarPane(style, width)
    , m_HelpText(helptext.empty() ? name.AfterFirst('e'): helptext)
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
class WXDLLIMPEXP_BASE wxExStatusBar : public wxStatusBar
{
public:
  /// Constructor.
  wxExStatusBar(wxExFrame* parent,
    wxWindowID id = wxID_ANY,
    long style = wxST_SIZEGRIP,
    const wxString& name = wxStatusBarNameStr);
    
  /// Destructor.
 ~wxExStatusBar();  

  /// Sets the fields.
  void SetFields(const std::vector<wxExStatusBarPane>& fields);

  /// Sets text on specified field.
  void SetStatusText(const wxString& text, const wxString& field);
protected:
  /// React on some mouse events line button down, double click and
  /// moving over.
  void OnMouse(wxMouseEvent& event);
private:
  wxExFrame* m_Frame;
  std::map<wxString, wxExStatusBarPane> m_Panes;

  DECLARE_EVENT_TABLE()
};
#endif // wxUSE_STATUSBAR
#endif // wxUSE_GUI
#endif
