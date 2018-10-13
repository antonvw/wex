////////////////////////////////////////////////////////////////////////////////
// Name:      stc-data.h
// Purpose:   Declaration of wex::stc_data
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/extension/control-data.h>
#include <wx/extension/stc-enums.h>

#define DEFAULT_TAGFILE "tags"

namespace wex
{
  class stc;

  /// Offers user data to be used by stc. 
  class stc_data
  {
  public:
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
    stc_data& Flags(stc_window_flags flags, data_action action = DATA_SET);

    /// Injects data.  
    bool Inject() const;

    /// Returns menu flags.
    const auto& Menu() const {return m_MenuFlags;};

    /// Sets menu flags.
    stc_data& Menu(stc_menu_flags flags, data_action action = DATA_SET);

    /// Returns window data.
    const auto& Window() const {return m_Data.Window();};

    /// Sets window data.
    stc_data& Window(window_data& data) {m_Data.Window(data); return *this;};
  private:  
    stc* m_STC {nullptr};

    control_data m_Data;

    std::string m_CTagsFileName {DEFAULT_TAGFILE};

    stc_menu_flags m_MenuFlags {static_cast<stc_menu_flags>(
      STC_MENU_CONTEXT | STC_MENU_OPEN_LINK | STC_MENU_OPEN_WWW | STC_MENU_VCS)};
    stc_window_flags m_WinFlags {STC_WIN_DEFAULT};
  };
};
