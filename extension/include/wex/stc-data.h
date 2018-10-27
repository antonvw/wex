////////////////////////////////////////////////////////////////////////////////
// Name:      stc-data.h
// Purpose:   Declaration of wex::stc_data
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/control-data.h>

#define DEFAULT_TAGFILE "tags"

namespace wex
{
  class stc;

  /// Offers user data to be used by stc. 
  class stc_data
  {
  public:
    /// Menu and tooltip flags.
    enum menu_flags
    {
      MENU_NONE      = 0,      ///< no context menu
      MENU_CONTEXT   = 1 << 1, ///< context menu
      MENU_OPEN_LINK = 1 << 2, ///< for adding link open menu
      MENU_OPEN_WWW  = 1 << 3, ///< for adding search on www open menu
      MENU_VCS       = 1 << 4, ///< for adding vcs menu
      MENU_DEBUG     = 1 << 5, ///< for adding debug menu
    };

    /// Window flags.
    enum window_flags
    {
      WIN_DEFAULT      = 0,      ///< default, not readonly, not hex mode
      WIN_READ_ONLY    = 1 << 1, ///< window is readonly, 
                                 ///<   overrides real mode from disk
      WIN_HEX          = 1 << 2, ///< window in hex mode
      WIN_NO_INDICATOR = 1 << 3, ///< a change indicator is not used
      WIN_IS_PROJECT   = 1 << 4  ///< open as project
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
    auto& Control() {return m_Data;};

    /// Sets control data.
    stc_data& Control(control_data& data) {m_Data = data; return *this;};

    /// Returns ctags filename.
    const auto& CTagsFileName() const {return m_CTagsFileName;};

    /// Sets ctags filename.
    stc_data& CTagsFileName(const std::string& text);

    /// Returns window flags.
    const auto& Flags() const {return m_WinFlags;};
    
    /// Set window flags.
    stc_data& Flags(
      window_flags flags, 
      control_data::action action = control_data::SET);

    /// Injects data.  
    bool Inject() const;

    /// Returns menu flags.
    const auto& Menu() const {return m_MenuFlags;};

    /// Sets menu flags.
    stc_data& Menu(
      menu_flags flags, 
      control_data::action action = control_data::SET);

    /// Returns window data.
    const auto& Window() const {return m_Data.Window();};

    /// Sets window data.
    stc_data& Window(window_data& data) {m_Data.Window(data); return *this;};
  private:  
    stc* m_STC {nullptr};

    control_data m_Data;

    std::string m_CTagsFileName {DEFAULT_TAGFILE};

    menu_flags m_MenuFlags {static_cast<menu_flags>(
      MENU_CONTEXT | MENU_OPEN_LINK | MENU_OPEN_WWW | MENU_VCS)};
    window_flags m_WinFlags {WIN_DEFAULT};
  };
};
