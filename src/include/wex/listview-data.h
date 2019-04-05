////////////////////////////////////////////////////////////////////////////////
// Name:      listview-data.h
// Purpose:   Declaration of wex::listview_data
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/control-data.h>

namespace wex
{
  class lexer;
  class listview;

  /// Offers user data to be used by listview. 
  class listview_data
  {
  public:
    /// The supported lists.
    enum type_t
    {
      FOLDER,   ///< a list containing folders only
      FIND,     ///< a list to show find results
      HISTORY,  ///< a list to show history items
      KEYWORD,  ///< a list to show keywords
      FILE,     ///< a list associated with a file
      NONE,     ///< a list without predefined columns
    };

    /// Which images to use.
    enum image_t
    {
      IMAGE_NONE,
      IMAGE_ART,       ///< using wxArtProvider
      IMAGE_FILE_ICON, ///< using the wxFileIconsTable
      IMAGE_OWN        ///< use your own images
    };

    /// Menu flags, they determine how the context menu will appear.
    enum
    {
      MENU_REPORT_FIND = 0, ///< for adding find and replace in files
      MENU_TOOL        = 1, ///< for adding tool menu
    };
    
    typedef std::bitset<2> menu_t;
    
    /// Default constructor.
    listview_data(listview* lv = nullptr);

    /// Copy constructor.
    listview_data(listview* lv, const listview_data& r);

    /// Constructor from control data.
    listview_data(control_data& data, listview* lv = nullptr);

    /// Constructor from window data.
    listview_data(window_data& data, listview* lv = nullptr);

    /// Assignment operator.
    listview_data& operator=(const listview_data& r);

    /// Returns control data.
    const auto& control() const {return m_Data;};

    /// Sets control data.
    listview_data& control(control_data& data) {m_Data = data; return *this;};
    
    /// Returns image type.
    const auto& image() const {return m_ImageType;};

    /// Sets image type.
    listview_data& image(image_t type);

    /// injects data.  
    bool inject();

    /// Returns lexer.
    const auto& lexer() const {return m_Lexer;};

    /// Sets lexer.
    listview_data& lexer(const wex::lexer* lexer);

    /// Returns menu flags.
    const auto& menu() const {return m_MenuFlags;};

    /// Sets menu flags.
    listview_data& menu(
      menu_t flags, 
      control_data::action_t action = control_data::SET);

    /// Returns type.
    const auto& type() const {return m_Type;};
    
    /// Sets type.
    listview_data& type(type_t type);

    /// Returns the list type as a string.
    const std::string type_description() const;

    /// Returns window data.
    const auto& window() const {return m_Data.window();};

    /// Sets window data.
    listview_data& window(window_data& data) {
      m_Data.window(data); return *this;};
  private:  
    void add_columns();

    control_data m_Data;

    menu_t m_MenuFlags = menu_t().set();

    const wex::lexer* m_Lexer = nullptr;
    listview* m_ListView = nullptr;

    image_t m_ImageType = IMAGE_ART;
    type_t m_Type = NONE;

    bool m_Initialized = false;
  };
};
