////////////////////////////////////////////////////////////////////////////////
// Name:      data/listview.h
// Purpose:   Declaration of wex::data::listview
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017-2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <bitset>
#include <wex/data/control.h>

namespace wex
{
  namespace factory
  {
    class listview;
  };

  class lexer;

  namespace data
  {
    /// Offers user data to be used by listview.
    class listview
    {
    public:
      /// The supported lists.
      enum type_t
      {
        FOLDER,  ///< a list containing folders only
        FIND,    ///< a list to show find results
        HISTORY, ///< a list to show history items
        KEYWORD, ///< a list to show keywords
        FILE,    ///< a list associated with a file
        TSV,     ///< a list with tab separated values for columns
        NONE,    ///< a list without predefined columns
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
      listview(factory::listview* lv = nullptr);

      /// Copy constructor.
      listview(factory::listview* lv, const data::listview& r);

      /// Constructor from control data.
      listview(data::control& data, factory::listview* lv = nullptr);

      /// Constructor from window data.
      listview(data::window& data, factory::listview* lv = nullptr);

      /// Assignment operator.
      listview& operator=(const data::listview& r);

      /// Returns control data.
      const auto& control() const { return m_data; }

      /// Sets control data.
      listview& control(const data::control& data)
      {
        m_data = data;
        return *this;
      };

      /// Returns image type.
      const auto& image() const { return m_image_type; }

      /// Sets image type.
      listview& image(image_t type);

      /// injects data.
      bool inject();

      /// Returns lexer.
      const auto& lexer() const { return m_lexer; }

      /// Sets lexer.
      listview& lexer(const wex::lexer* lexer);

      /// Returns menu flags.
      const auto& menu() const { return m_menu_flags; }

      /// Sets menu flags.
      listview&
      menu(menu_t flags, data::control::action_t action = data::control::SET);

      /// Returns type.
      const auto& type() const { return m_type; }

      /// Sets type.
      listview& type(type_t type);

      /// Returns the list type as a string.
      const std::string type_description() const;

      /// Returns window data.
      const auto& window() const { return m_data.window(); }

      /// Sets window data.
      listview& window(const data::window& data)
      {
        m_data.window(data);
        return *this;
      };

    private:
      void add_columns();

      data::control m_data;

      menu_t m_menu_flags = menu_t().set();

      const wex::lexer*  m_lexer    = nullptr;
      factory::listview* m_listview = nullptr;

      image_t m_image_type = IMAGE_ART;
      type_t  m_type       = NONE;

      bool m_initialized = false;
    };
  }; // namespace data
};   // namespace wex
