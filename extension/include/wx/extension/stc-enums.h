////////////////////////////////////////////////////////////////////////////////
// Name:      stc-enums.h
// Purpose:   Declaration of enums for wex::stc
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

namespace wex
{
  /// Margin flags.
  enum stc_margin_flags
  {
    STC_MARGIN_NONE       = 0,      ///< no margins
    STC_MARGIN_DIVIDER    = 1 << 1, ///< divider margin
    STC_MARGIN_FOLDING    = 1 << 2, ///< folding margin
    STC_MARGIN_LINENUMBER = 1 << 3, ///< line number margin
    STC_MARGIN_TEXT       = 1 << 4, ///< text margin
    STC_MARGIN_ALL        = 0xFFFF, ///< all margins
  };

  /// Menu and tooltip flags.
  enum stc_menu_flags
  {
    STC_MENU_NONE      = 0,      ///< no context menu
    STC_MENU_CONTEXT   = 1 << 1, ///< context menu
    STC_MENU_OPEN_LINK = 1 << 2, ///< for adding link open menu
    STC_MENU_OPEN_WWW  = 1 << 3, ///< for adding search on www open menu
    STC_MENU_VCS       = 1 << 4, ///< for adding vcs menu
    STC_MENU_DEBUG     = 1 << 5, ///< for adding debug menu
  };

  /// Window flags.
  enum stc_window_flags
  {
    STC_WIN_DEFAULT      = 0,      ///< default, not readonly, not hex mode
    STC_WIN_READ_ONLY    = 1 << 1, ///< window is readonly, 
                                   ///<   overrides real mode from disk
    STC_WIN_HEX          = 1 << 2, ///< window in hex mode
    STC_WIN_NO_INDICATOR = 1 << 3, ///< a change indicator is not used
    STC_WIN_IS_PROJECT   = 1 << 4  ///< open as project
  };
};
