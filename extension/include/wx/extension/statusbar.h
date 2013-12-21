////////////////////////////////////////////////////////////////////////////////
// Name:      statusbar.h
// Purpose:   Declaration of wxExStatusBar class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2013 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXSTATUSBAR_H
#define _EXSTATUSBAR_H

#include <vector>
#include <wx/statusbr.h> 

// Only if we have a gui.
#if wxUSE_GUI
#if wxUSE_STATUSBAR

/// This class defines our statusbar panes, used by wxExFrame::SetupStatusBar.
/// It just adds some members to the base class
/// (that offers GetText(), GetStyle() and GetWidth()).
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
    , m_IsShown(true)
    , m_Name()
    {};
  
  /// Constructor.
  wxExStatusBarPane(
    /// The name of the pane.
    /// The wxExtension lib uses PaneFileType, PaneInfo, PaneLexer, PaneMacro
    /// by setting up one of these panes,
    /// your panes will get controlled by the lib.
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
    , m_IsShown(true)
    , m_Name(name)
    {};
    
  /// Returns statusbar pane help text.
  const wxString& GetHelpText() const {return m_HelpText;};
  
  /// Returns statusbar pane name.
  const wxString& GetName() const {return m_Name;};
  
  /// Returns whether this pane is shown.
  bool IsShown() const {return m_IsShown;};
  
  /// Sets whether this pane is shown.
  void Show(bool show) {m_IsShown = show;};
private:
  wxString m_HelpText;
  wxString m_Name;
  bool m_IsShown;
};

class wxExFrame;

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
 
  /// Returns the status text on specified field.
  /// Returns empty string if field does not exist
  /// or is not shown.
  const wxString GetStatusText(const wxString& field) const;

  /// Sets the fields.
  void SetFields(const std::vector<wxExStatusBarPane>& fields);

  /// Sets text on specified field.
  /// Returns false if field does not exist or is not shown.
  bool SetStatusText(
    /// text
    const wxString& text, 
    /// field, default field pane 0
    const wxString& field = wxEmptyString);
  
  /// Shows or hides the field.
  /// Returns true if field visibility actually changed.
  bool ShowField(const wxString& field, bool show);
protected:
  /// React on some mouse events line button down, double click and
  /// moving over.
  void OnMouse(wxMouseEvent& event);
private:
  /// Returns the field no, or -1 if field does not exist
  /// or is not shown.
  int GetFieldNo(const wxString& field) const;
  void Handle(wxMouseEvent& event, const wxExStatusBarPane& wxExStatusBarPane);
  
  wxExFrame* m_Frame;
  std::vector<wxExStatusBarPane> m_Panes;
};
#endif // wxUSE_STATUSBAR
#endif // wxUSE_GUI
#endif
