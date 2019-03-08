////////////////////////////////////////////////////////////////////////////////
// Name:      blame.h
// Purpose:   Declaration of class wex::blame
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <pugixml.hpp>
#include <wex/lexers.h>

namespace wex
{
  /// Offers a blame class for some vcs.
  class blame 
  {
  public:
    /// Default constructor.
    blame() {;};
      
    /// Constructor using xml node.
    blame(const pugi::xml_node& node);

    /// Returns a tuple with result, blame info and style type for blame text.
    std::tuple <
      /// whether building the info passed
      bool, 
      /// blame info will contain id, author, date depending on 
      /// settings in the config.
      const std::string,
      /// style for blame margin based on commit date
      lexers::margin_style_t>                
    get(const std::string& text) const;
    
    /// Returns true if blame is on.
    bool use() const;

    /// Offers a blame field.
    class field
    {
    public:
      /// Default constructor.
      field() {;};
        
      /// Constructor using field value.
      field(const std::string& value);

      /// Returns true if field value is a number.
      bool is_number() const {return m_number != std::string::npos;};
      
      /// Returns position inside text, or std::string::npos if not found.
      size_t pos(const std::string text) const;
      
      /// Returns original value.
      const auto& value() const {return m_value;};
    private:
      size_t m_number {std::string::npos};
      std::string m_value;
    };
  private:
    const std::string get_author(const std::string& text) const;
    const std::string get_date(const std::string& text) const;
    const std::string get_id(const std::string& text) const;
    lexers::margin_style_t get_style(const std::string& text) const;
    
    std::string m_date_format;
    size_t m_date_print {10};

    field
      m_pos_author_begin,
      m_pos_begin, 
      m_pos_end,
      m_pos_id_begin,
      m_pos_id_end;
  };
};
