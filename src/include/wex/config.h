////////////////////////////////////////////////////////////////////////////////
// Name:      config.h
// Purpose:   Declaration of class wex::config
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <list>
#include <string>

class wxColour;
class wxConfigBase;
class wxFont;

namespace wex
{
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
    
    /// Default constructor.
    config(type_t type = DATA_STORE_WRITE);
    
    /// Constructor for item (key).
    config(const std::string& item);

    /// Returns the config dir for user data files.
    static const std::string dir();
    
    /// Returns true if the (string) item is empty.
    bool empty() const;

    /// Deletes the item.
    void erase();

    /// Returns true if the item exists.
    bool exists() const;

    /// Returns first of a list of values from item.
    const std::string firstof() const;

    /// Sets first of a list of values in config key.
    /// And returns the value.
    const std::string firstof_write(const std::string& value) const;
    
    /// Initializes global class.
    /// This should be done before first use of config.
    static void init();

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
    
    /// Loads entries into a list with strings.
    const std::list < std::string > get_list() const;
    
    /// Item access.

    /// Returns the item.
    auto & item() const {return m_item;};
    
    /// Sets the item, and returns config.
    config& item(const std::string& item) {m_item = item; return *this;};
    
    // Setter methods.

    /// Sets config item from a string.
    void set(const std::string& def = std::string());

    /// Sets config item from a char array.
    void set(const char* def);

    /// Sets config item from a bool.
    void set(bool def);

    /// Sets config item from a long.
    void set(long def);
    
    /// Sets config item from a int.
    void set(int def);
    
    /// Sets config item from a float.
    void set(float def);
    
    /// Sets config item from a double.
    void set(double def);
    
    /// Sets config item from a colour.
    void set(const wxColour& def);
    
    /// Sets config item from a font.
    void set(const wxFont& def);
    
    /// Sets config item from a list with strings.
    void set(const std::list < std::string > & l);
    
    /// Records defaults.
    static void set_record_defaults(bool val);
    
    /// Returns wx config.
    static wxConfigBase* wx_config();
  private:
    std::string m_item;
    type_t m_type {DATA_STORE_WRITE};
  };
};
