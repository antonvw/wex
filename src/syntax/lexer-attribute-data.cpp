////////////////////////////////////////////////////////////////////////////////
// Name:      lexer-attribute-data.cpp
// Purpose:   Implementation of wex::lexer_attribute_data class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/tokenizer.hpp>
#include <wex/core/log.h>
#include <wex/syntax/lexers.h>

#include <charconv>

#include "lexer-attribute-data.h"

wex::lexer_attribute_data::lexer_attribute_data(
  const pugi::xml_node*      n,
  const pugi::xml_attribute& a)
  : m_att(a)
  , m_node(n)
{
  const std::string nm(m_att.name());
  const auto        pos = nm.find('-');

  if (pos != std::string::npos)
  {
    const auto subs(nm.substr(pos + 1));
    std::from_chars(subs.data(), subs.data() + subs.size(), m_setno);
  }
}

void wex::lexer_attribute_data::add_keywords(lexer& l) const
{
  const std::string value(m_att.value());

  for (const auto& it : boost::tokenizer<boost::char_separator<char>>(
         value,
         boost::char_separator<char>(",")))
  {
    const auto& single = it;

    if (const auto& keywords = lexers::get()->keywords(single);
        keywords.empty())
    {
      wex::log("empty keywords for") << single << *m_node;
    }
    else if (!l.add_keywords(keywords, m_setno))
    {
      wex::log("keywords for") << single << "could not be set" << *m_node;
    }
  }
}
