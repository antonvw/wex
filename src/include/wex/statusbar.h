////////////////////////////////////////////////////////////////////////////////
// Name:      statusbar.h
// Purpose:   Declaration of wex::statusbar class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <tuple>
#include <vector>
#include <wex/window-data.h>
#include <wx/statusbr.h>

namespace wex
{
  /// This class defines our statusbar panes, used by
  /// wex::frame::setup_statusbar. It just adds some members to the base class
  /// (that offers GetText(), style() and GetWidth()).
  class statusbar_pane : public wxStatusBarPane
  {
  public:
    /// Default constructor.
    /// This constructs the PaneText pane.
    statusbar_pane(
      /// width of the pane (default, might be overridden in the config)
      int width = -5,
      /// style (default, might be overridden in the config)
      int style = wxSB_NORMAL)
      : wxStatusBarPane(style, width)
      , m_name("PaneText"){};

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
      int style = wxSB_NORMAL,
      /// initially show or hide the pane
      bool is_shown = true);

    /// Returns hidden text.
    const auto& get_hidden_text() const { return m_hidden; };

    /// Returns statusbar pane name.
    const auto& get_name() const { return m_name; };

    /// Returns statusbar pane help text.
    const auto& help_text() const { return m_help_text; };

    /// Returns whether this pane is shown.
    bool is_shown() const { return m_is_shown; };

    /// Sets hidden text.
    void set_hidden_text(const std::string& text) { m_hidden = text; };

    /// Sets whether this pane is shown.
    /// Resets the hidden text if show is true.
    void show(bool show);

  private:
    std::string m_help_text, m_hidden,
      m_name; // no const

    bool m_is_shown{true};
  };

  class frame;

  /// Offers a status bar that calls virtual methods from wex::frame,
  /// and allows you to address panes by name instead of number.
  class statusbar : public wxStatusBar
  {
  public:
    /// Sets up the statusbar.
    /// The status pane reserved for display status text messages is
    /// automatically added by the framework as the first pane.
    /// The next panes are used by the framework:
    /// - PaneFileType, shows file types
    /// - PaneInfo, shows info for control, e.g. lines
    /// - PaneLexer, shows lexer
    /// Returns created statusbar.
    static statusbar* setup(
      frame*                             frame,
      const std::vector<statusbar_pane>& panes,
      long                               style = wxST_SIZEGRIP,
      const std::string&                 name  = "statusBar");

    /// Constructor.
    statusbar(
      /// parent
      frame* parent,
      /// style
      /// - wxSTB_DEFAULT_STYLE
      /// (wxSTB_SIZEGRIP|wxSTB_ELLIPSIZE_END|wxSTB_SHOW_TIPS|wxFULL_REPAINT_ON_RESIZE)
      /// - wxSTB_ELLIPSIZE_END
      /// - wxSTB_ELLIPSIZE_MIDDLE
      /// - wxSTB_ELLIPSIZE_START
      /// - wxSTB_SHOW_TIPS
      /// - wxSTB_SIZEGRIP
      const window_data& data = window_data().style(wxSTB_DEFAULT_STYLE));

    /// Destructor.
    ~statusbar();

    /// Returns the statusbar_pane representing the n-th pane.
    const statusbar_pane& get_pane(int n) const;

    /// Returns the status text on specified pane.
    /// Returns empty string if pane does not exist
    /// or is not shown.
    const std::string get_statustext(const std::string& pane) const;

    /// Shows or hides the pane.
    /// Returns true if pane visibility actually changed.
    bool pane_show(const std::string& pane, bool show);

    /// Sets text on specified pane.
    /// Returns false if pane does not exist or is not shown.
    bool set_statustext(
      /// text
      const std::string& text,
      /// pane, default pane text pane,
      const std::string& pane = std::string());

  private:
    std::tuple<bool, int, int> pane_info(const std::string& pane) const;
    void handle(wxMouseEvent& event, const statusbar_pane& statusbar_pane);
    void on_mouse(wxMouseEvent& event);

    frame*                             m_frame;
    static std::vector<statusbar_pane> m_panes;
  };
}; // namespace wex
