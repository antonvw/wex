////////////////////////////////////////////////////////////////////////////////
// Name:      blame.h
// Purpose:   Declaration of class wex::blame
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <pugixml.hpp>
#include <wex/factory/lexers.h>

namespace wex
{
class regex;

/// Offers a blame class for some vcs. The vcs used is configured
/// using the xml_node constructor (see wex-menus.xml),
/// the kind of info returned after parse is configurable using
/// blame_get_author, blame_get_id, blame_get_date.
class blame
{
public:
  /// Default constructor using xml node.
  explicit blame(const pugi::xml_node& node = pugi::xml_node());

  /// Returns the suitable blame caption.
  const auto& caption() const { return m_caption; };

  /// Sets a suitable blame caption.
  void caption(const std::string& text) { m_caption = text; };

  /// Returns blame info will contain id, author, date depending on
  /// settings in the config.
  const auto& info() const { return m_info; };

  /// Returns true if path is present.
  bool is_path() const { return !m_path.empty(); };

  /// Returns line number (starting with line 0).
  const auto line_no() const { return m_line_no; };

  /// Sets line number, to override the one from parsed text.
  void line_no(int no) { m_line_no = no; };

  /// Returns rest of line text (without blame).
  const auto& line_text() const { return m_line_text; };

  /// Parses blame text and returns false if there was an error
  bool parse(const std::string& line);

  /// If blame file was changed, then the old name
  /// is present on the git blame line, and it will
  /// be stored in the path.
  const auto& path() const { return m_path; };

  /// Sets the skip info member, until next parse.
  void skip_info(bool rhs) { m_skip_info = rhs; };

  /// Returns current skip info value.
  bool skip_info() const { return m_skip_info; };

  /// Returns true if blame is on.
  bool use() const { return !m_blame_format.empty(); }

  /// Style for blame margin based on commit date.
  lexers::margin_style_t style() const { return m_style; };

  /// Returns vcs name for which this blaming is done.
  const auto& vcs_name() const { return m_name; };

private:
  bool parse_compact(const std::string& line, const regex& r);
  bool parse_full(const std::string& line, const regex& r);

  lexers::margin_style_t get_style(const std::string& text) const;

  std::string m_blame_format, m_caption, m_date_format, m_info, m_line_text,
    m_name, m_path;

  lexers::margin_style_t m_style{lexers::margin_style_t::UNKNOWN};

  int m_line_no{0};

  bool m_skip_info{false};

  size_t m_date_print{10};
};
}; // namespace wex
