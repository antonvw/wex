////////////////////////////////////////////////////////////////////////////////
// Name:      statusbar.h
// Purpose:   Declaration of wex::statusbar class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <tuple>
#include <vector>
#include <wx/statusbr.h> 
#include <wex/window-data.h> 

namespace wex
{
  /// This class defines our statusbar panes, used by wex::frame::setup_statusbar.
  /// It just adds some members to the base class
  /// (that offers GetText(), style() and GetWidth()).
  class statusbar_pane : public wxStatusBarPane
  {
  public:
    /// Default constructor.
    /// This constructs the PaneText pane.
    statusbar_pane(
      /// width of the pane (default, might be overridden in the config)
      int width = -3,
      /// style (default, might be overridden in the config)
      int style = wxSB_NORMAL)
      : wxStatusBarPane(style, width) 
      , m_Name("PaneText") {};
    
    /// Constructor.
    statusbar_pane(
      /// name of the pane
      /// this lib uses:
      /// - PaneFileType
      /// - PaneInfo
      /// - PaneLexer
      /// - PaneTheme
      /// - PaneMacro
      /// - PaneMode
      /// by setting up one of these panes,
      /// your panes will get controlled by the lib.
      const std::string& name,
      /// width of the pane (default, might be overridden in the config)
      int width = 50,
      /// helptext shown as a tooltip
      /// If you do no provide helptext, it is derived from the name, by using
      /// text after the first 'e' character (so after 'Pane').
      const std::string& helptext = std::string(),
      /// style (default, might be overridden in the config)
      /// - wxSB_NORMAL (0)
      /// - wxSB_FLAT (1)
      /// - wxSB_RAISED (2)
      /// - wxSB_SUNKEN (3)
      int style = wxSB_NORMAL)
      : wxStatusBarPane(style, width)
      , m_HelpText(helptext.empty() ? name.substr(name.find('e') + 1): helptext)
      , m_Name(name) {};
      
    /// Returns hidden text.
    const auto& get_hidden_text() const {return m_HiddenText;};
    
    /// Returns statusbar pane name.
    const auto& get_name() const {return m_Name;};
    
    /// Returns statusbar pane help text.
    const auto& help_text() const {return m_HelpText;};
    
    /// Returns whether this pane is shown.
    bool is_shown() const {return m_IsShown;};
    
    /// Sets hidden text.
    void set_hidden_text(const std::string& text) {m_HiddenText = text;};
    
    /// Sets whether this pane is shown.
    /// Resets the hidden text if show is true.
    void show(bool show);
  private:
    std::string m_HelpText, m_HiddenText, m_Name; // no const
    bool m_IsShown {true};
  };

  class frame;

  /// Offers a status bar that calls virtual methods from wex::frame,
  /// and allows you to address panes by name instead of number.
  class statusbar : public wxStatusBar
  {
  public:
    /// Constructor.
    statusbar(
      /// parent
      frame* parent,
      /// style
      /// - wxSTB_DEFAULT_STYLE (wxSTB_SIZEGRIP|wxSTB_ELLIPSIZE_END|wxSTB_SHOW_TIPS|wxFULL_REPAINT_ON_RESIZE)
      /// - wxSTB_ELLIPSIZE_END
      /// - wxSTB_ELLIPSIZE_MIDDLE
      /// - wxSTB_ELLIPSIZE_START
      /// - wxSTB_SHOW_TIPS
      /// - wxSTB_SIZEGRIP
      const window_data& data = window_data().style(wxSTB_DEFAULT_STYLE));
      
    /// Destructor.
   ~statusbar();  

    /// Returns the statusbar_pane representing the n-th pane. 
    const statusbar_pane& get_field(int n) const;
   
    /// Returns the status text on specified field.
    /// Returns empty string if field does not exist
    /// or is not shown.
    const std::string get_statustext(const std::string& field) const;

    /// Sets text on specified field.
    /// Returns false if field does not exist or is not shown.
    bool set_statustext(
      /// text
      const std::string& text, 
      /// field, default field text pane,
      const std::string& field = std::string());
    
    /// Sets up the statusbar.
    /// The status pane reserved for display status text messages is
    /// automatically added by the framework as the first pane.
    /// The next panes are used by the framework:
    /// - PaneFileType, shows file types
    /// - PaneInfo, shows info for control, e.g. lines
    /// - PaneLexer, shows lexer
    /// Returns created statusbar.
    static statusbar* setup(
      frame* frame,
      const std::vector<statusbar_pane>& panes,
      long style = wxST_SIZEGRIP,
      const wxString& name = "statusBar");

    /// Shows or hides the field.
    /// Returns true if field visibility actually changed.
    bool show_field(const std::string& field, bool show);
  protected:
    /// React on some mouse events line button down, double click and
    /// moving over.
    void OnMouse(wxMouseEvent& event);
  private:
    std::tuple <bool, int, int> GetFieldNo(const std::string& field) const;
    void Handle(wxMouseEvent& event, const statusbar_pane& statusbar_pane);
    
    frame* m_Frame;
    static std::vector<statusbar_pane> m_Panes;
  };
};
