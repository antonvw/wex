////////////////////////////////////////////////////////////////////////////////
// Name:      blame.h
// Purpose:   Declaration of class wex::blame
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <pugixml.hpp>

namespace wex
{
class path;
class regex;

namespace factory
{
class stc;
};

/// Offers a blame class for some vcs. The vcs used is configured
/// using the xml_node constructor (see wex-menus.xml),
/// the kind of info returned after parse is configurable using
/// blame_get_author, blame_get_id, blame_get_date.
// clang-format off
/// \dot
/// digraph mode {
///   apply_margin    [label="lexers::apply_margin_text_style", fontsize=9, shape=diamond, color=grey]
///   blame           [label="wex::blame", fontsize=9, shape=component, color=grey]
///   blame_margin    [label="stc::blame_margin", fontsize=9, shape=diamond, color=grey]
///   blame_revision  [label="stc::blame_revision", fontsize=9, shape=diamond, color=grey]
///   boost           [label="boost::process", fontsize=9, shape=component, color=grey]
///   open_file       [label="frame::open_file", fontsize=9, shape=diamond, color=grey]
///   process         [label="wex::process", fontsize=9, shape=component, color=grey]
///   stc             [label="wex::stc", fontsize=9, shape=component, color=grey]
///   stc_no_margin   [label="stc no margin", fontname="times bold"]
///   stc_margin      [label="stc margin", fontname="times bold"]
///   vcs_blame_show  [label="frame::vcs_blame_show", fontsize=9, shape=diamond, color=grey]
///   vcs_entry       [label="wex::vcs_entry", fontsize=9, shape=component, color=grey]
///   vcs_execute     [label="frame::vcs_execute", fontsize=9, shape=diamond, color=grey]
///
///   {rank=same init stc_no_margin stc_margin}
///   {rank=same blame_margin blame_revision}
///   {rank=same blame stc}
///   {rank=same vcs_execute vcs_entry vcs_blame_show}
/// 
///   init            -> stc_no_margin [style=dotted,label="start"]
///   apply_margin    -> stc_margin [label="done"]
///   apply_margin    -> blame [label="info" dir="none"]
///   blame           -> vcs_blame_show [label="done"]
///   blame           -> blame_margin [label="info" dir="none"]
///   blame           -> stc [label="margin_renamed" dir="none"]
///   blame_margin    -> stc [label="SetMarginWidth"]
///   blame_revision  -> vcs_entry [label="system"]
///   open_file       -> vcs_blame_show [label="done"]
///   process         -> vcs_entry [label="done"]
///   process         -> boost [label="system" dir="none"]
///   stc             -> blame_revision [label="margin_get_revision_id\nmargin_get_revision_renamed" dir="none" ]
///   stc             -> stc_no_margin [label="reset_margins"]
///   stc             -> stc_margin [label="blame_margin"]
///   stc_no_margin   -> vcs_execute [label="git blame", fontsize=9, fontname="times italic"]
///   stc_margin      -> blame_revision [label="Blame Revision", fontsize=9, fontname="times italic"]
///   stc_margin      -> blame_revision [label="Blame Previous", fontsize=9, fontname="times italic"]
///   stc_margin      -> stc [label="Hide", fontsize=9, fontname="times italic"]
///   vcs_blame_show  -> apply_margin [label=""]
///   vcs_blame_show  -> blame [label="parse"]
///   vcs_blame_show  -> blame_margin [label=""]
///   vcs_blame_show  -> vcs_entry [label="std_out" dir="none"]
///   vcs_entry       -> open_file [label="done"]
///   vcs_entry       -> process [label="system"]
///   vcs_execute     -> vcs_entry [label="system"]
///  }
/// \enddot
// clang-format on
class blame
{
public:
  /// Margin text style type.
  enum class margin_style_t
  {
    UNKNOWN, ///< no style
    DAY,     ///< within a day
    WEEK,    ///< within a week
    MONTH,   ///< within a month
    YEAR,    ///< within a year
    OTHER    ///< more than a year
  };

  // Static interface.

  /// Returns a renamed path present in the stc margin,
  /// or empty string if no rename present.
  static std::string margin_renamed(const factory::stc* stc);

  // Other methods.

  /// Default constructor using xml node.
  explicit blame(const pugi::xml_node& node = pugi::xml_node());

  /// Returns the suitable blame caption.
  const auto& caption() const { return m_caption; };

  /// Sets a suitable blame caption.
  void caption(const std::string& text) { m_caption = text; };

  /// Returns blame info will contain id, author, date depending on
  /// settings in the config. Also adds a renamed path if a changed
  /// path is present.
  const std::string info() const;

  /// Returns line number (starting with line 0).
  const auto line_no() const { return m_line_no; };

  /// Sets line number, to override the one from parsed text.
  void line_no(int no) { m_line_no = no; };

  /// Returns rest of line text (without blame).
  const auto& line_text() const { return m_line_text; };

  /// Parses blame text and returns false if there was an error
  bool parse(
    /// original path
    const path& p,
    /// line to parse
    const std::string& line);

  /// Sets the skip info member, until next parse.
  void skip_info(bool rhs) { m_skip_info = rhs; };

  /// Returns current skip info value.
  bool skip_info() const { return m_skip_info; };

  /// Returns true if blame is on.
  bool use() const { return !m_blame_format.empty(); }

  /// Style for blame margin based on commit date.
  margin_style_t style() const { return m_style; };

  /// Returns vcs name for which this blaming is done.
  const auto& vcs_name() const { return m_name; };

private:
  bool is_renamed_path() const;
  bool parse_compact(const std::string& line, const regex& r);
  bool parse_full(const std::string& line, const regex& r);

  margin_style_t get_style(const std::string& text) const;

  std::string m_blame_format, m_caption, m_date_format, m_info, m_line_text,
    m_name, m_path;

  wex::path m_path_original;

  margin_style_t m_style{margin_style_t::UNKNOWN};

  int m_line_no{0};

  bool m_skip_info{false};

  size_t m_date_print{10}; // cannot be const because of used copy assignment
};
}; // namespace wex
