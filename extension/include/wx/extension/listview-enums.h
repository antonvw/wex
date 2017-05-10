////////////////////////////////////////////////////////////////////////////////
// Name:      listview-enums.h
// Purpose:   Declaration of enums for wxExListView
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

/// Which images to use.
enum wxExImageType
{
  IMAGE_NONE,
  IMAGE_ART,       ///< using wxArtProvider
  IMAGE_FILE_ICON, ///< using the wxFileIconsTable
  IMAGE_OWN        ///< use your own images
};

/// The supported lists.
enum wxExListType
{
  LIST_FOLDER,     ///< a list containing folders only
  LIST_FIND,       ///< a list to show find results
  LIST_HISTORY,    ///< a list to show history items
  LIST_KEYWORD,    ///< a list to show keywords
  LIST_FILE,       ///< a list associated with a file
  LIST_NONE,       ///< a list without predefined columns
};

/// Menu flags, they determine how the context menu will appear.
enum
{
  LIST_MENU_REPORT_FIND = 0x0001, ///< for adding find and replace in files
  LIST_MENU_TOOL        = 0x0002, ///< for adding tool menu

  LIST_MENU_DEFAULT = 
    LIST_MENU_REPORT_FIND | 
    LIST_MENU_TOOL
};
