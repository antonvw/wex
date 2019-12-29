////////////////////////////////////////////////////////////////////////////////
// Name:      config.h
// Purpose:   Declaration of class wex::config
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <list>
#include <string>
#include <tuple>
#include <vector>

class wxColour;
class wxFont;

namespace wex
{
  class config_imp;

  /// Offers a configuration using key value pairs with defaults.
  class config
  {
  public:
    /// Type to hold statusbar panes setup
    /// as a vector of tuples:
    typedef std::vector < 
      std::tuple < 
        /// pane name
        std::string, 
        /// styles, first being actual used type
        std::list < std::string >, 
        /// width
        int > > 
      statusbar_t;

    /// static interface
    
    /// Returns the config dir for user data files.
    static const std::string dir();
    
    /// Stores config, and frees objects.
    static void on_exit();
    
    /// Initializes global class.
    /// This should be done before first use of config.
    static void on_init();
    
    /// Sets the config file to use.
    /// If you do no use this, the default config file is used.
    static void set_file(const std::string& file);
    
    /// Returns number of top level entries.
    static size_t size();

    /// other methods

    /// Default constructor.
    /// Optionally provide the item (key).
    /// You can also create a hierarchy using the parent dot child expression:
    /// @code
    /// wex::config("x.y.z").set(8);
    /// const auto j(config("x.y.z").get(9));
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
    config(const std::string& item = std::string());

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
    
    /// Saves changes to current config file, and sets
    /// and uses new file as config file.
    bool change_file(const std::string& file);
    
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
    
    /// Returns a list with strings for item.
    const std::list < std::string > get(
      const std::list < std::string> & def) const;

    /// Returns a vector with ints for item.
    const std::vector < int > get(const std::vector < int > & def) const;

    /// Returns a statusbar_t for item.
    const statusbar_t get(const statusbar_t & def) const;

    /// Returns first of a list of strings from item.
    const std::string get_firstof() const;

    /// Returns true if this item is a child.
    bool is_child() const;

    /// Item access / nested values.

    /// Returns the item.
    auto & item() const {return m_item;};
    
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
    
    /// Sets value from a list with strings.
    void set(const std::list < std::string > & v);

    /// Sets value from a vector with int.
    void set(const std::vector < int > & v);

    /// Sets value from a statusbar_t.
    void set(const statusbar_t & r);

    /// Sets first of a list of strings in config key,
    /// deletes it if present at other places.
    /// If the list size would be greater than max, the last element is deleted.
    /// And returns the value.
    const std::string set_firstof(const std::string& v, size_t max = 75);
    
    /// If this item is a bool, toggles value and returns new value.
    bool toggle(bool def = false);
  private:
    config_imp* get_store() const;

    std::string m_item;
    config_imp* m_local {nullptr};
    inline static config_imp* m_store = nullptr;
  };
};
