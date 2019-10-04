////////////////////////////////////////////////////////////////////////////////
// Name:      stc-data.h
// Purpose:   Declaration of wex::stc_data
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/control-data.h>

namespace wex
{
  class stc;
  
  /// Offers user data to be used by stc. 
  class stc_data
  {
  public:
    /// Menu and tooltip flags.
    enum
    {
      MENU_CONTEXT   = 0, ///< context menu
      MENU_OPEN_LINK = 1, ///< for adding link open menu
      MENU_OPEN_WWW  = 2, ///< for adding search on www open menu
      MENU_VCS       = 3, ///< for adding vcs menu
      MENU_DEBUG     = 4, ///< for adding debug menu
    };

    /// Window flags.
    enum
    {
      WIN_READ_ONLY    = 0, ///< window is readonly, 
                            ///<   overrides real mode from disk
      WIN_HEX          = 1, ///< window in hex mode
      WIN_NO_INDICATOR = 2, ///< a change indicator is not used
      WIN_IS_PROJECT   = 3  ///< open as project
    };
    
    enum indicator_t
    {
      IND_LINE   = 0,
      IND_ERR    = 1,
      IND_DEBUG  = 2,
    };
    
    typedef std::bitset<5> menu_t;
    typedef std::bitset<4> window_t;
    
    /// Support class for client data stored at the event.
    class event_data
    {
    public:
      /// Default constructor.
      event_data() {;};

      /// Access.
      bool pos_at_end() const {return m_pos_at_end;};
      auto pos_end() const {return m_pos_end;};
      auto pos_start() const {return m_pos_start;};
      bool synced() const {return m_synced;};
      bool synced_log() const {return m_synced_log;};

      /// Fill the members.
      void set(stc* s, bool synced);
    private:
      bool 
        m_pos_at_end {false}, 
        m_synced {false},
        m_synced_log {false};

      int m_pos_start {-1}, m_pos_end {-1};
    };
    
    /// Default constructor.
    stc_data(stc* stc = nullptr);

    /// Constructor from control data.
    stc_data(control_data& data, stc* stc = nullptr);

    /// Constructor from window data.
    stc_data(window_data& data, stc* stc = nullptr);

    /// Copy constructor.
    stc_data(stc* stc, const stc_data& r);

    /// Assignment operator.
    stc_data& operator=(const stc_data& r);
    
    /// Returns control data.
    auto& control() {return m_data;};

    /// Sets control data.
    stc_data& control(control_data& data) {m_data = data; return *this;};
    
    /// Returns event data.
    const auto& event() const {return m_event_data;};
    
    /// Sets event data.
    stc_data& event(bool synced) {m_event_data.set(m_stc, synced); return *this;};

    /// Returns window flags.
    const auto& flags() const {return m_win_flags;};
    
    /// Set window flags.
    stc_data& flags(
      window_t flags, 
      control_data::action_t action = control_data::SET);

    /// Returns indicator type.
    const auto indicator_no() const {return m_indicator_no;};

    /// Sets indicator type.
    stc_data& indicator_no(indicator_t t);

    /// injects data.  
    bool inject() const;

    /// Returns menu flags.
    const auto& menu() const {return m_menu_flags;};

    /// Sets menu flags.
    stc_data& menu(
      menu_t flags, 
      control_data::action_t action = control_data::SET);

    /// Returns window data.
    const auto& window() const {return m_data.window();};

    /// Sets window data.
    stc_data& window(window_data& data) {m_data.window(data); return *this;};
  private:  
    stc* m_stc {nullptr};

    control_data m_data;
    indicator_t m_indicator_no {IND_LINE};
    menu_t m_menu_flags {menu_t().set(
      MENU_CONTEXT).set(MENU_OPEN_LINK).set(MENU_OPEN_WWW).set(MENU_VCS)};
    window_t m_win_flags {0};
    event_data m_event_data;
  };
};
