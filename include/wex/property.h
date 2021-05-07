////////////////////////////////////////////////////////////////////////////////
// Name:      property.h
// Purpose:   Declaration of wex::property class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <pugixml.hpp>
#include <wx/stc/stc.h>

namespace wex
{
/// This class defines our scintilla properties.
class property
{
public:
  /// Default constructor.
  explicit property(const pugi::xml_node& node = pugi::xml_node());

  /// Constructor using name, value pair.
  property(const std::string& name, const std::string& value);

  /// Applies this property to stc component.
  void apply(wxStyledTextCtrl* stc) const;

  /// Resets this property (resets the value of this property
  /// on the stc component, but does not change the value).
  void apply_reset(wxStyledTextCtrl* stc) const;

  /// Returns true if property is valid.
  bool is_ok() const;

  /// Returns the name of this property.
  const auto& name() const { return m_name; }

  /// Override this property (so does not apply this property).
  void set(const std::string& value) { m_value = value; }

  /// Returns the value of this property.
  const auto& value() const { return m_value; }

private:
  std::string m_name, m_value;
};
}; // namespace wex
