////////////////////////////////////////////////////////////////////////////////
// Name:      config.h
// Purpose:   Declaration of class wex::config
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/core/path.h>
#include <wex/core/types.h>

class wxColour;
class wxFont;

#include <string>
#include <tuple>
#include <vector>

namespace wex
{
class config_imp;

/// Offers a configuration using key value pairs with defaults.
class config
{
public:
  /// Type for keeping the string values. We use the same as core types.
  typedef wex::strings_t strings_t;

  /// Type for keeping the int values. We use the same as core types.
  typedef wex::ints_t ints_t;

  /// Type to hold statusbar panes setup
  /// as a vector of tuples:
  typedef std::vector<std::tuple<
    /// pane name
    std::string,
    /// styles, first being actual used type
    strings_t,
    /// width
    int>>
    statusbar_t;

  // Static interface

  /// Returns the config path for user data files.
  static const wex::path dir();

  /// Do not save current config file on exit.
  static void discard() { m_store_save = false; }

  /// Saves changes to store (unless discard was invoked), and frees objects.
  /// This is done in app::OnExit.
  static bool on_exit();

  /// Initializes the store, and reads previous file.
  /// This should be done before first use of config,
  /// and is done in app::OnInit.
  static bool on_init();

  /// Returns the current config path.
  static const wex::path path();

  /// Reads current config file.
  static void read();

  /// Saves current config file.
  static void save();

  /// Sets the config path to use.
  /// If you do no use this, the default config path is used.
  static void set_path(const wex::path& p);

  /// Returns number of top level entries.
  static size_t size();

  /// Returns whether store is available.
  static bool store_is_active() { return m_store != nullptr; };

  // Other methods

  /// Default constructor.
  /// Optionally provide the item (key).
  /// You can also create a hierarchy using the parent dot child expression:
  /// @code
  /// wex::config("x.y.z").set(8);
  /// const auto i(config("x.y.z").get(9));
  /// @endcode
  /// i will be 8.
  /// If you are using children, you can retrieve the value
  /// using parent dot child expression:
  /// @code
  /// wex::config c("x");
  /// c.child_start();
  /// c.item("u").set(1);
  /// c.item("v").set(2);
  /// c.item("w").set(3);
  /// c.child_end();
  /// const auto i(c.get("x.u", 9));
  /// @endcode
  /// i will be 1.
  explicit config(const std::string& item = std::string());

  /// Constructor for one child item (calling child_start not necessary).
  /// @code
  /// wex::config c("y", "u");
  /// c.item("u").set(1);
  /// c.item("v").set(2);
  /// c.item("w").set(3);
  /// const auto i(c.get("y.u", 9));
  /// @endcode
  config(const std::string& parent, const std::string& child);

  /// Destructor, calls child_end.
  ~config();

  /// Ends setting child values for this item,
  /// deletes a possible local store for a child item.
  /// Returns false if not yet started.
  bool child_end();

  /// Starts setting child values for this item.
  /// Returns false if already started.
  bool child_start();

  /// Returns number of children for this item.
  size_t children() const;

  /// Returns true if the (string) item is empty.
  bool empty() const;

  /// Deletes the item.
  void erase() const;

  /// Returns true if the item exists.
  bool exists() const;

  /// Getter methods.

  /// Returns text config value for item.
  const std::string get(const std::string& def = std::string()) const;

  /// Returns text config value for item.
  const std::string get(const char* def) const;

  /// Returns boolean config value for item.
  bool get(bool def) const;

  /// Returns long config value for item.
  long get(long def) const;

  /// Returns long config value for item.
  int get(int def) const;

  /// Returns float config value for item.
  float get(float def) const;

  /// Returns double config value for item.
  double get(double def) const;

  /// Returns colour config value for item.
  wxColour get(const wxColour& def) const;

  /// Returns font config value for item.
  wxFont get(const wxFont& def) const;

  /// Returns a strings type for item.
  const strings_t get(const strings_t& def) const;

  /// Returns a vector with ints for item.
  const ints_t get(const ints_t& def) const;

  /// Returns a statusbar_t for item.
  const statusbar_t get(const statusbar_t& def) const;

  /// Returns first of a list of strings from item.
  const std::string get_first_of(const std::string& def = std::string()) const;

  /// Returns true if this item is a child.
  bool is_child() const;

  /// Item access / nested values.

  /// Returns the item.
  auto& item() const { return m_item; }

  /// Sets the item, and returns config.
  config& item(const std::string& item);

  // Setter methods.

  /// Sets value from a string.
  void set(const std::string& v = std::string());

  /// Sets value from a char array.
  void set(const char* v);

  /// Sets value from a bool.
  void set(bool v);

  /// Sets value from a long.
  void set(long v);

  /// Sets value from a int.
  void set(int v);

  /// Sets value from a float.
  void set(float v);

  /// Sets value from a double.
  void set(double v);

  /// Sets value from a colour.
  void set(const wxColour& v);

  /// Sets value from a font.
  void set(const wxFont& v);

  /// Sets value from a strings type.
  void set(const strings_t& v);

  /// Sets value from a ints type.
  void set(const ints_t& v);

  /// Sets value from a statusbar_t.
  void set(const statusbar_t& r);

  /// Sets first of a list of strings in config key,
  /// deletes it if present at other places.
  /// If the list size would be greater than max, the last element is deleted.
  /// And returns the value.
  const std::string set_first_of(const std::string& v, size_t max = 75);

  /// If this item is a bool, toggles value and returns new value.
  bool toggle(bool def = false);

private:
  config_imp* get_store() const;

  std::string               m_item;
  config_imp*               m_local{nullptr};
  inline static config_imp* m_store = nullptr;
  inline static bool        m_store_save{true};
};
} // namespace wex
