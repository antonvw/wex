////////////////////////////////////////////////////////////////////////////////
// Name:      lexer.h
// Purpose:   Declaration of wex::lexer class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/factory/property.h>
#include <wex/factory/style.h>

import<map>;
import<set>;
import<string>;
import<vector>;

namespace pugi
{
class xml_node;
};

namespace wex
{
namespace factory
{
class stc;
};

/// Parses properties node.
void node_properties(
  const pugi::xml_node*  node,
  std::vector<property>& properties);

/// Parses style node.
void node_styles(
  const pugi::xml_node* node,
  const std::string&    lexer,
  std::vector<style>&   styles);

/// This class defines a lexer using file extensions,
/// syntax colouring and comment definitions.
/// This lexer is one of the Scintilla lexers.
/// The lexers are read by and kept in the wex::lexers class.
class lexer
{
public:
  /// Default constructor.
  explicit lexer(const std::string& lexer = std::string());

  /// Constructor using factory stc.
  explicit lexer(factory::stc* stc);

  /// Constructor using xml node.
  explicit lexer(const pugi::xml_node* node);

  /// Assignment operator.
  /// The stc component is assigned only if it is nullptr.
  lexer& operator=(const lexer& l);

  /// Adds keywords (public for testing only).
  bool add_keywords(const std::string& text, int setno = 0);

  /// Aligns text.
  /// Fills out over lexer comment lines
  /// If the lexer has no comment end character, fill out
  /// with spaces is not done.
  const std::string align_text(
    /// lines to align
    const std::string_view& lines,
    /// The header is used as a prefix for the line, directly
    /// followed by the lines, and if necessary on the next line
    /// the header is repeated as a string of spaces.
    const std::string_view& header = std::string(),
    /// if fill out, then use space
    bool fill_out_with_space = true,
    /// fill out
    bool fill_out = false) const;

  /// Applies this lexer to stc component (and colours the component).
  bool apply() const;

  /// Returns specified config attrib.
  /// The value depends on attrib, but is -1 if not present.
  int attrib(const std::string& name) const;

  /// Returns all the attribs.
  const auto& attribs() const { return m_attribs; }

  /// Clears lexer and applies it to stc if available.
  /// The is ok member is set to false.
  void clear();

  /// Returns a string that completes specified comment,
  /// by adding spaces and a comment end at the end.
  /// If the comment end string is empty, it returns empty string.
  const std::string comment_complete(const std::string& comment) const;

  /// Returns the comment begin.
  const auto& comment_begin() const { return m_comment_begin; }

  /// Returns the comment begin 2.
  const auto& comment_begin2() const { return m_comment_begin2; }

  /// Returns the comment end.
  const auto& comment_end() const { return m_command_end; }

  /// Returns the comment end 2.
  const auto& comment_end2() const { return m_command_end2; }

  /// Returns the display lexer (as shown in dialog).
  const auto& display_lexer() const { return m_display_lexer; }

  /// Returns the extensions.
  const auto& extensions() const { return m_extensions; }

  /// Is this word a keyword (always all keywords), case sensitive.
  bool is_keyword(const std::string& word) const;

  /// Is this lexer valid.
  bool is_ok() const { return m_is_ok; }

  /// Does any keyword (always all keywords) start with this word,
  /// case insensitive.
  bool keyword_starts_with(const std::string& word) const;

  /// Returns the keywords.
  const auto& keywords() const { return m_keywords; }

  /// Returns the keywords as one large string,
  const std::string keywords_string(
    /// if keyword_set -1 take all the sets,
    /// otherwise take the specified set.
    int keyword_set = -1,
    /// if min_size 0, use all keywords,
    /// otherwise use keywords with minimum size
    size_t min_size = 0,
    /// prefix keyword should start with
    const std::string& prefix = std::string()) const;

  /// Returns the language.
  const auto& language() const { return m_language; }

  /// Returns the line size.
  size_t line_size() const;

  /// Logs info about this class.
  std::stringstream log() const;

  /// Returns a lexer comment string with text formatted.
  const std::string make_comment(
    const std::string& text,
    bool               fill_out_with_space = true,
    bool               fill_out            = true) const;

  /// Returns a lexer comment string with prefix.
  const std::string
  make_comment(const std::string& prefix, const std::string& text) const;

  /// Returns a lexer comment string filled out over one line.
  const std::string make_single_line_comment(
    const std::string_view& text,
    bool                    fill_out_with_space = true,
    bool                    fill_out            = true) const;

  /// Returns true if the stc component
  /// associated with this lexer can be previewed.
  bool is_previewable() const { return m_previewable; }

  /// Returns the properties.
  const auto& properties() const { return m_properties; }

  /// Returns the scintilla lexer.
  const auto& scintilla_lexer() const { return m_scintilla_lexer; }

  /// Sets lexer to specified lexer (finds by name from lexers),
  /// Shows error message when lexer could not be set.
  bool set(const std::string& lexer, bool fold = false);

  /// Sets lexer to specified lexer, and applies it to stc if present.
  /// Returns true if a scintilla lexer has been set.
  bool set(const lexer& lexer, bool fold = false);

  /// Overrides a local property.
  void set_property(const std::string& name, const std::string& value);

  /// Returns the styles.
  const auto& styles() const { return m_styles; }

  /// Returns number of chars that fit on a line, skipping comment chars.
  size_t usable_chars_per_line() const;

private:
  void              auto_match(const std::string& lexer);
  const std::string formatted_text(
    const std::string& lines,
    const std::string& header,
    bool               fill_out_with_space,
    bool               fill_out) const;
  void parse_attrib(const pugi::xml_node* node);

  // The scintilla name for this lexer cannot be const,
  // as in path the operator= is used on a lexer.
  // Normally the lexer displayed is the scintilla lexer,
  // however this might be different, as with c#.
  // In that case the scintilla lexer is cpp, whereas the display lexer is c#.
  std::string m_comment_begin, m_comment_begin2, m_command_end, m_command_end2,
    m_display_lexer, m_extensions, m_language, m_scintilla_lexer;

  // each keyword set in a separate keyword set
  std::map<int, std::set<std::string>> m_keywords_set;
  std::set<std::string>                m_keywords;
  std::vector<size_t>   m_edge_columns; // last one is used for line size
  std::vector<property> m_properties;
  std::vector<style>    m_styles;
  std::vector<std::tuple<
    std::string,
    int,
    std::function<void(factory::stc* stc, int attrib)>>>
    m_attribs;

  bool m_is_ok{false}, m_previewable{false};

  factory::stc* m_stc{nullptr};
};
}; // namespace wex
