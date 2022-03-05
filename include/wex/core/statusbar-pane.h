////////////////////////////////////////////////////////////////////////////////
// Name:      statusbar-pane.h
// Purpose:   Declaration of wex::statusbar-pane class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/statusbr.h>

namespace wex
{
/// This class defines our statusbar panes, used by
/// wex::frame::setup_statusbar. It just adds some members to the base class
/// (that offers GetText(), GetStyle() and GetWidth()).
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
    , m_name("PaneText")
  {
  }

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
    /// show pane
    bool show = true);

  /// Sets helptext.
  /// helptext shown as a tooltip
  /// If you do no provide helptext, it is derived from the name, by using
  /// text after the first 'e' character (so after 'Pane').
  statusbar_pane& help(const std::string& rhs);

  /// Returns statusbar pane help text.
  const auto& help_text() const { return m_help_text; }

  /// Returns hidden text.
  const auto& hidden_text() const { return m_hidden; }

  /// Sets hidden text.
  statusbar_pane& hidden_text(const std::string& text);

  /// Returns whether this pane is shown.
  bool is_shown() const { return m_is_shown; }

  /// Returns statusbar pane name.
  const auto& name() const { return m_name; }

  /// Sets whether this pane is shown.
  /// Resets the hidden text if show is true.
  statusbar_pane& show(bool show);

  /// Sets style.
  /// style (default, might be overridden in the config)
  /// - wxSB_NORMAL (0)
  /// - wxSB_FLAT (1)
  /// - wxSB_RAISED (2)
  /// - wxSB_SUNKEN (3)
  statusbar_pane& style(int rhs);

private:
  std::string m_help_text, m_hidden, m_name;

  bool m_is_shown{true};
};
}; // namespace wex
