////////////////////////////////////////////////////////////////////////////////
// Name:      frd.h
// Purpose:   Declaration of wex::find_replace_data class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/factory/frd.h>
#include <wex/ui/textctrl-input.h>

namespace wex
{
/// Offers a class to hold data for find replace functionality.
class find_replace_data : public factory::find_replace_data
{
  friend class find_textctrl;
  friend class textctrl_imp;

public:
  /// Static interface.

  /// Returns the find replace data.
  static find_replace_data* get(bool createOnDemand = true);

  /// Sets the object as the current one, returns the pointer
  /// to the previous current object
  /// (both the parameter and returned value may be nullptr).
  static find_replace_data* set(find_replace_data* frd);

  /// Other methods.

  /// Returns the find strings.
  const auto& get_find_strings() const { return m_find_strings.values(); }

  /// Returns the replace strings.
  const auto& get_replace_strings() const
  {
    return m_replace_strings.values();
  };

  /// Sets the find strings.
  /// Also moves the find string to the beginning of the find
  /// strings list.
  void set_find_strings(const textctrl_input::values_t& v);

  /// Sets the replace strings.
  /// Also moves the replace string to the beginning of the replace
  /// strings list.
  void set_replace_strings(const textctrl_input::values_t& v);

  /// Virtual overrides.

  void set_find_string(const std::string& value) override;
  void set_replace_string(const std::string& value) override;

private:
  find_replace_data();

  static inline find_replace_data* m_self = nullptr;

  textctrl_input m_find_strings, m_replace_strings;
};
}; // namespace wex
