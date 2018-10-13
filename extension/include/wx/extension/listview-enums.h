////////////////////////////////////////////////////////////////////////////////
// Name:      listview-enums.h
// Purpose:   Declaration of enums for wex::listview
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

namespace wex
{
  /// Which images to use.
  enum image_type
  {
    IMAGE_NONE,
    IMAGE_ART,       ///< using wxArtProvider
    IMAGE_FILE_ICON, ///< using the wxFileIconsTable
    IMAGE_OWN        ///< use your own images
  };

  /// The supported lists.
  enum listview_type
  {
    LISTVIEW_FOLDER,     ///< a list containing folders only
    LISTVIEW_FIND,       ///< a list to show find results
    LISTVIEW_HISTORY,    ///< a list to show history items
    LISTVIEW_KEYWORD,    ///< a list to show keywords
    LISTVIEW_FILE,       ///< a list associated with a file
    LISTVIEW_NONE,       ///< a list without predefined columns
  };

  /// Menu flags, they determine how the context menu will appear.
  enum
  {
    LISTVIEW_MENU_REPORT_FIND = 0x0001, ///< for adding find and replace in files
    LISTVIEW_MENU_TOOL        = 0x0002, ///< for adding tool menu

    LISTVIEW_MENU_DEFAULT = 
      LISTVIEW_MENU_REPORT_FIND | 
      LISTVIEW_MENU_TOOL
  };
};
