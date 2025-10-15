////////////////////////////////////////////////////////////////////////////////
// Name:      data/listview.h
// Purpose:   Declaration of wex::data::listview
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/factory/control.h>

#include <bitset>

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

  /// A typedef containing menu flags.
  typedef std::bitset<2> menu_t;

  /// Default constructor.
  listview();

  /// Constructor from control data.
  listview(data::control& data);

  /// Constructor from window data.
  listview(data::window& data);

  /// Returns control data.
  const data::control& control() const { return m_data; }

  /// Sets control data.
  listview& control(const data::control& data)
  {
    m_data = data;
    return *this;
  };

  /// Returns listview.
  factory::listview* get_listview() { return m_listview; };

  /// Returns image type.
  image_t image() const { return m_image_type; }

  /// Sets image type.
  listview& image(image_t type);

  /// injects data.
  bool inject();

  /// Returns lexer.
  const wex::lexer* lexer() const { return m_lexer; }

  /// Sets lexer.
  listview& lexer(const wex::lexer* lexer);

  /// Returns menu flags.
  const menu_t& menu() const { return m_menu_flags; }

  /// Sets menu flags.
  listview&
  menu(menu_t flags, data::control::action_t action = data::control::SET);

  /// Returns is revision member.
  bool revision() const { return m_is_revision; };

  /// Sets is revision member, to enable revision like menus.
  listview& revision(bool rhs);

  /// Sets listview.
  listview& set_listview(factory::listview* rhs);

  /// Returns type.
  const type_t& type() const { return m_type; }

  /// Sets type.
  listview& type(type_t type);

  /// Returns the list type as a string.
  const std::string type_description() const;

  /// Returns window data.
  const data::window& window() const { return m_data.window(); }

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

  const wex::lexer*  m_lexer{nullptr};
  factory::listview* m_listview{nullptr};

  image_t m_image_type{IMAGE_ART};
  type_t  m_type{NONE};

  bool m_initialized{false}, m_is_revision{false};
};
}; // namespace data
}; // namespace wex
