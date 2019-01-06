////////////////////////////////////////////////////////////////////////////////
// Name:      lexers.h
// Purpose:   Declaration of wex::lexers class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <map>
#include <set>
#include <vector>
#include <pugixml.hpp>
#include <wex/indicator.h>
#include <wex/lexer.h>
#include <wex/marker.h>
#include <wex/path.h>
#include <wex/property.h>
#include <wex/style.h>

class wxWindow;

namespace wex
{
  class stc;
  
  /// Collection of all lexers.
  /// The lexers are loaded from wex-lexers.xml, this is done
  /// automatically during the first get call.
  class lexers
  {
  public:
    /// Margin text style type.
    enum margin_style_t
    {
      MARGIN_STYLE_DAY,
      MARGIN_STYLE_WEEK,
      MARGIN_STYLE_MONTH,
      MARGIN_STYLE_YEAR,
      MARGIN_STYLE_OTHER
    };
    
    /// Applies containers (except global styles) to specified component.
    void apply(stc* stc) const;

    /// Sets global styles (and colours and indicators) 
    /// for current theme for specified component.
    void apply_global_styles(stc* stc);

    /// Applies macro to text:
    /// if text is referring to a macro, text is replaced by the macro value.
    /// Otherwise the same text is returned.
    const std::string apply_macro(
      const std::string& text, 
      const std::string& lexer = "global");

    /// Applies margin text style to stc line.
    /// If text is supplied also sets margin text.
    void apply_margin_text_style(
      /// stc component
      stc* stc, 
      /// line no
      int line, 
      /// style type
      margin_style_t style, 
      /// text to be set in the margin
      const std::string& text = std::string()) const;

    /// Finds a lexer specified by a filename (fullname).
    const lexer find_by_filename(const std::string& fullname) const;

    /// Finds a lexer specified by the (display scintilla) name.
    const lexer find_by_name(const std::string& name) const;

    /// Finds a lexer if text starts with some special tokens.
    const lexer find_by_text(const std::string& text) const;

    /// Returns the lexers object.
    /// If this is the first invocation, and createOnDemand is true,
    /// it also invokes load_document.
    static lexers* get(bool createOnDemand = true);

    /// Returns the filename.
    const auto & get_filename() const {return m_Path;};
    
    /// Returns indicator from loaded indicators,
    /// based on the no of specified indicator.
    const indicator get_indicator(const indicator& indicator) const;

    /// Returns the lexers.
    const auto & get_lexers() const {return m_Lexers;};

    /// Returns the macros for specified lexer.
    const auto & get_macros(const std::string& lexer) {return m_Macros[lexer];};

    /// Returns marker from loaded markers,
    /// based on the no of specified marker.
    const marker get_marker(const marker& marker) const;
    
    /// Returns number of themes (should at least contain empty theme).
    auto get_themes_size() const {return m_ThemeMacros.size();};
    
    /// Returns true if specified indicator is available.
    bool indicator_is_loaded(const indicator& indic) const {
      return m_Indicators.find(indic) != m_Indicators.end();};

    /// Returns the keywords for the specified named set of keywords.
    /// Returns empty string if set does not exist.
    const std::string keywords(const std::string& set) const;

    /// Loads all lexers (first clears them) from document.
    /// Returns true if the document is loaded.
    bool load_document();

    /// Returns true if specified marker is available.
    bool marker_is_loaded(const marker& marker) const {
      return m_Markers.find(marker) != m_Markers.end();};

    /// Returns global properties.
    const auto & properties() const {return m_globalProperties;};

    /// Resets the theme.
    void reset_theme() {
      if (!m_Theme.empty()) 
      {
        m_ThemePrevious = m_Theme; 
        m_Theme.clear();
      }};
    
    /// Restores the theme from previous theme.
    void restore_theme() {m_Theme = m_ThemePrevious;};
    
    /// Sets the object as the current one, returns the pointer 
    /// to the previous current object 
    /// (both the parameter and returned value may be nullptr). 
    static lexers* set(lexers* lexers);
    
    /// Shows a dialog with all lexers, allowing you to choose one.
    /// Returns true and sets the lexer on the stc component if you selected one.
    bool show_dialog(stc* stc) const;
      
    /// Shows a dialog with all themes, allowing you to choose one.
    /// Returns true and sets current theme if you select one.
    bool show_theme_dialog(wxWindow* parent);

    /// Returns the current theme.
    const auto & theme() const {return m_Theme;};
    
    /// Returns the theme macros for the current theme.
    const auto & theme_macros() {return m_ThemeMacros[m_Theme];};
  private:
    lexers(const path& filename);

    void ParseNodeFolding(const pugi::xml_node& node);
    void ParseNodeglobal(const pugi::xml_node& node);
    void ParseNodeKeyword(const pugi::xml_node& node);
    void ParseNodeMacro(const pugi::xml_node& node);
    void ParseNodeTheme(const pugi::xml_node& node);
    void ParseNodeThemes(const pugi::xml_node& node);

    std::map<std::string, std::string> m_DefaultColours, m_Keywords;

    std::map<std::string, std::map<std::string, std::string> > 
      m_Macros, m_ThemeColours, m_ThemeMacros;

    std::set<indicator> m_Indicators;
    std::set<marker> m_Markers;

    std::vector<property> m_globalProperties;
    std::vector<lexer> m_Lexers;
    std::vector<style> m_Styles, m_StylesHex;
    std::vector<std::pair<std::string, std::string>> m_Texts;

    style m_DefaultStyle;

    const path m_Path;
    
    std::string 
      m_FoldingBackgroundColour, 
      m_FoldingForegroundColour,
      m_Theme,
      m_ThemePrevious;
    
    int 
      m_StyleNoTextMargin {-1}, 
      m_StyleNoTextMarginDay {-1}, 
      m_StyleNoTextMarginWeek {-1},
      m_StyleNoTextMarginMonth {-1},
      m_StyleNoTextMarginYear {-1};

    static inline lexers* m_Self = nullptr;
  };
};
