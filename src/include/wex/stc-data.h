////////////////////////////////////////////////////////////////////////////////
// Name:      stc-data.h
// Purpose:   Declaration of wex::data::stc
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <bitset>
#include <wex/control-data.h>

namespace wex
{
  namespace core
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
        MENU_CONTEXT   = 0, ///< context menu
        MENU_OPEN_LINK = 1, ///< for adding link open menu
        MENU_OPEN_WWW  = 2, ///< for adding search on www open menu
        MENU_VCS       = 3, ///< for adding vcs menu
        MENU_DEBUG     = 4, ///< for adding debug menu
      };

      /// Window flags.
      enum
      {
        WIN_READ_ONLY = 0,    ///< window is readonly,
                              ///<   overrides real mode from disk
        WIN_HEX          = 1, ///< window in hex mode
        WIN_NO_INDICATOR = 2, ///< a change indicator is not used
        WIN_IS_PROJECT   = 3  ///< open as project
      };

      enum indicator_t
      {
        IND_LINE  = 0,
        IND_ERR   = 1,
        IND_DEBUG = 2,
      };

      typedef std::bitset<5> menu_t;
      typedef std::bitset<4> window_t;

      /// Support class for client data stored at the event.
      class event_data
      {
      public:
        /// Access.
        bool pos_at_end() const { return m_pos_at_end; };
        auto pos_end() const { return m_pos_end; };
        auto pos_start() const { return m_pos_start; };
        bool synced() const { return m_synced; };
        bool synced_log() const { return m_synced_log; };

        /// Fill the members.
        void set(core::stc* s, bool synced);

      private:
        bool m_pos_at_end{false}, m_synced{false}, m_synced_log{false};

        int m_pos_start{-1}, m_pos_end{-1};
      };

      /// Default constructor.
      stc(core::stc* stc = nullptr);

      /// Constructor from control data.
      stc(data::control& data, core::stc* stc = nullptr);

      /// Constructor from window data.
      stc(data::window& data, core::stc* stc = nullptr);

      /// Copy constructor.
      stc(core::stc* stc, const data::stc& r);

      /// Assignment operator.
      stc& operator=(const data::stc& r);

      /// Returns control data.
      auto& control() { return m_data; };

      /// Sets control data.
      stc& control(data::control& data)
      {
        m_data = data;
        return *this;
      };

      /// Returns event data.
      const auto& event() const { return m_event_data; };

      /// Sets event data.
      stc& event(bool synced)
      {
        m_event_data.set(m_stc, synced);
        return *this;
      };

      /// Returns window flags.
      const auto& flags() const { return m_win_flags; };

      /// Set window flags.
      stc& flags(
        window_t                flags,
        data::control::action_t action = data::control::SET);

      /// Returns indicator type.
      const auto indicator_no() const { return m_indicator_no; };

      /// Sets indicator type.
      stc& indicator_no(indicator_t t);

      /// injects data.
      bool inject() const;

      /// Returns menu flags.
      const auto& menu() const { return m_menu_flags; };

      /// Sets menu flags.
      stc&
      menu(menu_t flags, data::control::action_t action = data::control::SET);

      /// Returns window data.
      const auto& window() const { return m_data.window(); };

      /// Sets window data.
      stc& window(data::window& data)
      {
        m_data.window(data);
        return *this;
      };

    private:
      core::stc* m_stc{nullptr};

      data::control m_data;
      indicator_t   m_indicator_no{IND_LINE};
      menu_t        m_menu_flags{menu_t()
                            .set(MENU_CONTEXT)
                            .set(MENU_OPEN_LINK)
                            .set(MENU_OPEN_WWW)
                            .set(MENU_VCS)};
      window_t      m_win_flags{0};
      event_data    m_event_data;
    };
  }; // namespace data
};   // namespace wex
