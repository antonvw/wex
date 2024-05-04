////////////////////////////////////////////////////////////////////////////////
// Name:      stc.h
// Purpose:   Declaration of wex::data::stc
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/core/path.h>
#include <wex/factory/control.h>

#include <bitset>

namespace wex
{
namespace syntax
{
class stc;
};

namespace data
{
/// Offers user data to be used by stc.
class stc
{
public:
  /// Menu and tooltip flags.
  enum
  {
    MENU_CONTEXT = 0, ///< context menu
    MENU_OPEN_LINK,   ///< for adding link open menu
    MENU_OPEN_WWW,    ///< for adding search on www open menu
    MENU_VCS,         ///< for adding vcs menu
    MENU_DEBUG,       ///< for adding debug menu
  };

  /// A typedef containing menu flags.
  typedef std::bitset<5> menu_t;

  /// Window flags.
  enum
  {
    WIN_READ_ONLY = 0, ///< window is readonly,
                       ///<   overrides real mode from disk
    WIN_HEX,           ///< window in hex mode
    WIN_NO_INDICATOR,  ///< a change indicator is not used
    WIN_IS_PROJECT,    ///< open as project
    WIN_EX,            ///< window in ex mode, instead of vi
    WIN_SINGLE_LINE    ///< window has only a single line
  };

  /// A typedef containing window flags.
  typedef std::bitset<6> window_t;

  /// Indicator type.
  enum indicator_t
  {
    IND_LINE = 0, ///< line indicator
    IND_ERR,      ///< error indicator
    IND_DEBUG     ///< debug indicator
  };

  /// Support class for client data stored at the event.
  class event_data
  {
  public:
    /// Access.
    auto pos_end() const { return m_pos_end; }
    auto pos_start() const { return m_pos_start; }
    bool is_pos_at_end() const { return m_pos_at_end; }
    bool is_synced() const { return m_synced; }
    bool is_synced_log() const { return m_synced_log; }

    /// Fill the members.
    void set(syntax::stc* s, bool synced);

  private:
    bool m_pos_at_end{false}, m_synced{false}, m_synced_log{false};

    int m_pos_start{-1}, m_pos_end{-1};
  };

  /// Default constructor.
  stc();

  /// Constructor from control data.
  stc(const data::control& data);

  /// Constructor from window data.
  stc(const data::window& data);

  /// Returns control data.
  auto& control() const { return m_data; }

  /// Returns (or sets) control data.
  auto& control() { return m_data; }

  /// Sets control data.
  stc& control(const data::control& data)
  {
    m_data = data;
    return *this;
  };

  /// Returns event data.
  const auto& event() const { return m_event_data; }

  /// Sets event data.
  stc& event(bool synced)
  {
    m_event_data.set(m_stc, synced);
    return *this;
  };

  /// Returns window flags.
  const auto& flags() const { return m_win_flags; }

  /// Set window flags.
  stc&
  flags(window_t flags, data::control::action_t action = data::control::SET);

  /// Returns stc.
  auto* get_stc() { return m_stc; };

  /// Returns head path.
  const auto& head_path() const { return m_head_path; }

  /// Sets head path.
  stc& head_path(const path& r)
  {
    m_head_path = r;
    return *this;
  }

  /// Returns indicator type.
  const auto indicator_no() const { return m_indicator_no; }

  /// Sets indicator type.
  stc& indicator_no(indicator_t t);

  /// injects data.
  bool inject() const;

  /// Returns menu flags.
  const auto& menu() const { return m_menu_flags; }

  /// Sets menu flags.
  stc& menu(menu_t flags, data::control::action_t action = data::control::SET);

  /// Returns whether recent is on, for allowing set_recent_file.
  bool recent() const { return m_recent; }

  /// Sets recent.
  stc& recent(bool recent)
  {
    m_recent = recent;
    return *this;
  };

  /// Sets stc.
  stc& set_stc(syntax::stc* rhs)
  {
    m_stc = rhs;
    return *this;
  };

  /// Returns window data.
  const auto& window() const { return m_data.window(); }

  /// Sets window data.
  stc& window(const data::window& data)
  {
    m_data.window(data);
    return *this;
  };

private:
  bool inject_col() const;
  bool inject_command() const;
  bool inject_find() const;
  bool inject_line() const;

  syntax::stc* m_stc{nullptr};

  path m_head_path;

  data::control m_data;
  indicator_t   m_indicator_no{IND_LINE};
  menu_t        m_menu_flags{menu_t()
                        .set(MENU_CONTEXT)
                        .set(MENU_OPEN_LINK)
                        .set(MENU_OPEN_WWW)
                        .set(MENU_VCS)};
  window_t      m_win_flags{0};
  event_data    m_event_data;

  bool m_recent{true};
};
}; // namespace data
}; // namespace wex
