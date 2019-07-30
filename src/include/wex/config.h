////////////////////////////////////////////////////////////////////////////////
// Name:      config.h
// Purpose:   Declaration of class wex::config
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <list>
#include <string>
#include <vector>

class wxColour;
class wxFont;

namespace wex
{
  class config_imp;

  /// Offers a configuration.
  class config
  {
  public:
    enum type_t
    {
      DATA_STORE_WRITE,     ///< create store if not present
      DATA_STORE_OVERWRITE, ///< always create new store
      DATA_NO_STORE,        ///< never store (so use defaults)
    };
    
    /// static interface
    
    /// Returns the config dir for user data files.
    static const std::string dir();
    
    /// Initializes global class.
    /// This should be done before first use of config.
    static void init();

    /// Stores config, and frees objects.
    static void on_exit();
    
    /// other methods

    /// Default constructor.
    config(type_t type = DATA_STORE_WRITE);
    
    /// Constructor for item (key).
    config(const std::string& item);

    /// Returns true if the (string) item is empty.
    bool empty() const;

    /// Deletes the item.
    void erase();

    /// Returns true if the item exists.
    bool exists() const;

    // Getter methods.

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

    /// Returns a vector with tuples for item.
    const std::vector < std::tuple < std::string, int, int > > get(
      const std::vector < std::tuple < std::string, int, int > > & def) const;

    /// Returns first of a list of strings from item.
    const std::string get_firstof() const;

    /// Item access.

    /// Returns the item.
    auto & item() const {return m_item;};
    
    /// Sets the item, and returns config.
    config& item(const std::string& item) {m_item = item; return *this;};
    
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

    /// Sets value from a vector with tuples.
    void set(const std::vector < std::tuple < std::string, int, int > > & v);

    /// Sets first of a list of strings in config key,
    /// deletes it if present at other places.
    /// If the list size would be greater than max, the last element is deleted.
    /// And returns the value.
    const std::string set_firstof(const std::string& v, size_t max = 75);
  private:
    std::string m_item;
    const type_t m_type {DATA_STORE_WRITE};
    inline static config_imp* m_store = nullptr;
  };
};
