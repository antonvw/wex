////////////////////////////////////////////////////////////////////////////////
// Name:      lexers.h
// Purpose:   Declaration of wex::lexers class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2008-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <pugixml.hpp>

#include <array>
#include <map>
#include <set>
#include <unordered_map>
#include <vector>

#include <wex/core/path.h>
#include <wex/core/reflection.h>
#include <wex/syntax/indicator.h>
#include <wex/syntax/lexer.h>
#include <wex/syntax/marker.h>

class wxWindow;

namespace wex
{
class blame;

namespace factory
{
class stc;
};

/// Collection of all lexers.
/// The lexers are loaded from wex-lexers.xml, this is done
/// automatically during the first get call.
class lexers
{
public:
  /// Name values type for macros.
  typedef std::unordered_map<std::string, std::string> name_values_t;

  // Static interface

  /// Returns the lexers object.
  /// If this is the first invocation, and createOnDemand is true,
  /// it also invokes load_document (unless is_initial_load(false) was invoked).
  static lexers* get(bool create_on_demand = true);

  /// Sets the initial loading of document.
  static void is_initial_load(bool load) { m_is_initial_load = load; };

  /// Sets the object as the current one, returns the pointer
  /// to the previous current object
  /// (both the parameter and returned value may be nullptr).
  static lexers* set(lexers* lexers);

  // Other methods

  /// Applies containers (except global styles) to specified component.
  void apply(factory::stc* stc) const;

  /// Applies default style to functions for back and foreground colours.
  void apply_default_style(
    const std::function<void(const std::string&)>& back,
    const std::function<void(const std::string&)>& fore = nullptr) const;

  /// Sets global styles (and colours and indicators)
  /// for current theme for specified component.
  void apply_global_styles(factory::stc* stc);

  /// Applies macro to text:
  /// if text is referring to a macro, text is replaced by the macro value.
  // Otherwise the same text is returned.
  const std::string apply_macro(
    const std::string& text,
    const std::string& lexer = "global") const;

  /// Applies margin text style to stc line.
  /// If text is supplied also sets margin text.
  void apply_margin_text_style(
    /// stc component
    factory::stc* stc,
    /// blame info to use
    const blame* info) const;

  /// Clears the theme.
  void clear_theme();

  /// Finds a lexer specified by the (display scintilla) name.
  const lexer& find(const std::string& name) const;

  /// Finds a lexer specified by a filename (fullname).
  const lexer& find_by_filename(const std::string& fullname) const;

  /// Finds a lexer if text starts with some special tokens.
  const lexer& find_by_text(const std::string& text) const;

  /// Returns the default style.
  const style& get_default_style() const { return m_default_style; }

  /// Returns indicator from loaded indicators,
  /// based on the no of specified indicator.
  const indicator& get_indicator(const indicator& indicator) const;

  /// Returns the lexers.
  const auto& get_lexers() const { return m_lexers; }

  /// Returns the macros for specified lexer.
  const name_values_t& get_macros(const std::string& lexer) const;

  /// Returns marker from loaded markers,
  /// based on the no of specified marker.
  const marker& get_marker(const marker& marker) const;

  /// Returns number of themes (should at least contain empty theme).
  auto get_themes_size() const { return m_theme_macros.size(); }

  /// Returns true if specified indicator is available.
  bool indicator_is_loaded(const indicator& indic) const
  {
    return m_indicators.find(indic) != m_indicators.end();
  };

  /// Returns the keywords for the specified named set of keywords.
  /// Returns empty string if set does not exist.
  const std::string& keywords(const std::string& set) const;

  /// Loads all lexers from document.
  /// Returns true if the document is loaded.
  bool load_document();

  /// Returns true if specified marker is available.
  bool marker_is_loaded(const marker& marker) const
  {
    return m_markers.find(marker) != m_markers.end();
  };

  /// Returns max number of marker read from the lexers file.
  /// The markers used by folding are not included.
  int marker_max_no_used() const { return m_max_no_marker; }

  /// Returns the path.
  const auto& path() const { return m_path; }

  /// Returns global properties.
  const auto& properties() const { return m_global_properties; }

  /// Restores the theme from previous theme.
  void restore_theme() { m_theme = m_theme_previous; }

  /// Shows a dialog with all themes, allowing you to choose one.
  /// Returns true and sets current theme if you select one.
  bool show_theme_dialog(wxWindow* parent);

  /// Returns the current theme.
  const auto& theme() const { return m_theme; }

  /// Returns the theme macros for the current theme.
  const name_values_t& theme_macros() const;

private:
  explicit lexers();

  void load_document(pugi::xml_document& doc, const wex::path& p);
  void load_document_check();
  bool load_document_init();

  void parse_node_folding(const pugi::xml_node& node);
  void parse_node_global(const pugi::xml_node& node);
  void parse_node_keyword(const pugi::xml_node& node);
  void parse_node_macro(const pugi::xml_node& node);
  void
  parse_node_macro_def(const pugi::xml_node& node, const std::string& name);
  void parse_node_theme(const pugi::xml_node& node);
  void parse_node_themes(const pugi::xml_node& node);

  name_values_t m_default_colours, m_keywords;

  // Buffer used to keep textual representation of
  // styles, with max wxSTC_STYLE_MAX.
  std::array<char, 4> m_buffer;

  // This a map, so the theme dialog is ordered.
  std::map<std::string, name_values_t> m_theme_macros;

  std::set<indicator> m_indicators;
  std::set<marker>    m_markers;

  std::unordered_map<std::string, name_values_t> m_macros, m_theme_colours;

  std::vector<property> m_global_properties;
  std::vector<lexer>    m_lexers{lexer()}; // ensure we have a lexer
  std::vector<style>    m_styles, m_styles_hex;

  std::vector<std::pair<std::string, std::string>> m_texts;

  style m_default_style;

  const wex::path m_path, m_path_macro;

  std::string m_folding_background_colour, m_folding_foreground_colour;

  int m_style_no_text_margin{-1}, m_style_no_text_margin_day{-1},
    m_style_no_text_margin_week{-1}, m_style_no_text_margin_month{-1},
    m_style_no_text_margin_year{-1}, m_max_no_marker{-1};

  bool m_is_loaded{false};

  reflection m_reflect;

  static inline std::string m_theme, m_theme_previous;

  static inline lexers* m_self            = nullptr;
  static inline bool    m_is_initial_load = true;
};
}; // namespace wex
