////////////////////////////////////////////////////////////////////////////////
// Name:      blame.h
// Purpose:   Declaration of class wex::blame
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <pugixml.hpp>
#include <wex/factory/lexers.h>

namespace wex
{
/// Offers a blame class for some vcs. The vcs used is configured
/// using the xml_node constructor (see wex-menus.xml),
/// the kind of info returned by get is configurable using
/// blame_get_author, blame_get_id, blame_get_date.
class blame
{
public:
  /// Default constructor using xml node.
  explicit blame(const pugi::xml_node& node = pugi::xml_node());

  /// Returns a tuple with result, blame info, style type
  /// and line number for blame text.
  std::tuple<
    /// whether building the info passed
    bool,
    /// blame info will contain id, author, date depending on
    /// settings in the config.
    const std::string,
    /// style for blame margin based on commit date
    lexers::margin_style_t,
    /// line number (starting with line 0)
    int>
  get(const std::string& text) const;

  /// Returns true if blame is on.
  bool use() const { return !m_blame_format.empty(); }

private:
  lexers::margin_style_t get_style(const std::string& text) const;

  std::string m_blame_format, m_date_format;

  size_t m_date_print{10};
};
}; // namespace wex
