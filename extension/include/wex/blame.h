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
    bool use() const {return !m_blame_format.empty();};
  private:
    lexers::margin_style_t get_style(const std::string& text) const;
    
    std::string m_date_format, m_blame_format;
    size_t m_date_print {10};
  };
};
