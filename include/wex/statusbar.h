////////////////////////////////////////////////////////////////////////////////
// Name:      statusbar.h
// Purpose:   Declaration of wex::statusbar class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/data/window.h>
#include <wex/statusbar-pane.h>
#include <wx/statusbr.h>

import<tuple>;
import<vector>;

namespace wex
{
namespace factory
{
class frame;
};

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
    factory::frame*                    frame,
    const std::vector<statusbar_pane>& panes,
    long                               style = wxST_SIZEGRIP,
    const std::string&                 name  = "statusBar");

  /// Constructor.
  statusbar(
    /// parent
    factory::frame* parent,
    /// style
    /// - wxSTB_DEFAULT_STYLE
    /// (wxSTB_SIZEGRIP|wxSTB_ELLIPSIZE_END|wxSTB_SHOW_TIPS|wxFULL_REPAINT_ON_RESIZE)
    /// - wxSTB_ELLIPSIZE_END
    /// - wxSTB_ELLIPSIZE_MIDDLE
    /// - wxSTB_ELLIPSIZE_START
    /// - wxSTB_SHOW_TIPS
    /// - wxSTB_SIZEGRIP
    const data::window& data = data::window().style(wxSTB_DEFAULT_STYLE));

  /// Returns the statusbar_pane representing the n-th pane.
  const statusbar_pane& get_pane(int n) const { return m_panes[n]; }

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
  void handle(wxMouseEvent& event, const statusbar_pane& statusbar_pane);
  void on_mouse(wxMouseEvent& event);

  std::tuple<bool, int, int> pane_info(const std::string& pane) const;

  factory::frame*                    m_frame;
  static std::vector<statusbar_pane> m_panes;
};
}; // namespace wex
