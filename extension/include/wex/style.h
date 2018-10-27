////////////////////////////////////////////////////////////////////////////////
// Name:      style.h
// Purpose:   Declaration of wex::style class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <pugixml.hpp>
#include <set>

class wxStyledTextCtrl;

namespace wex
{
  /// This class defines our scintilla styles. The no as in xml or in the string
  /// can be a single style, or several styles separated by a comma.
  /// E.g.
  /// 1,2,3=fore:light steel blue,italic,size:8
  /// 1,2,3 are the scintilla styles numbers, and the rest is spec
  class style
  {
  public:
    /// Default constructor.
    style() {;};
    
    /// Constructor using xml node (sets no from the no attribute).
    style(const pugi::xml_node& node, const std::string& macro) {
      Set(node, macro);};

    /// Constructor using no and value.
    style(
      const std::string& no, 
      const std::string& value,
      const std::string& macro = "global")
      : m_Value(value) {
      SetNo(no, macro);};

    /// Applies this style to stc component.
    /// If no style is present, STC StyleResetDefault is invoked.
    void Apply(wxStyledTextCtrl* stc) const;

    /// Is the default style part of these styles.
    bool ContainsDefaultStyle() const;
    
    /// Returns the original define.
    const auto & GetDefine() const {return m_Define;};

    /// Returns the no ('s).
    const std::string GetNo() const;
    
    /// Returns the value.
    const auto & GetValue() const {return m_Value;};

    /// Returns true if this style is valid.
    bool is_ok() const {
      return !m_No.empty() && !m_Value.empty();};
  private:
    void Set(const pugi::xml_node& node, const std::string& macro);
    void SetNo(const std::string& no, const std::string& macro, 
      const pugi::xml_node& node = pugi::xml_node());

    std::set <int> m_No;
    std::string m_Define, m_Value; 
  };
};
