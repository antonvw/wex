////////////////////////////////////////////////////////////////////////////////
// Name:      lexers.h
// Purpose:   Declaration of wex::lexers class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <map>
#include <set>
#include <vector>
#include <pugixml.hpp>
#include <wx/extension/indicator.h>
#include <wx/extension/lexer.h>
#include <wx/extension/marker.h>
#include <wx/extension/path.h>
#include <wx/extension/property.h>
#include <wx/extension/style.h>

class wxWindow;

namespace wex
{
  class stc;
  
  /// Collection of all lexers.
  /// The lexers are loaded from lexers.xml, this is done
  /// automatically during the first Get call.
  class lexers
  {
  public:
    /// Applies containers (except global styles) to specified component.
    void Apply(stc* stc) const;

    /// Sets global styles (and colours and indicators) 
    /// for current theme for specified component.
    void ApplyGlobalStyles(stc* stc);

    /// Applies macro to text:
    /// if text is referring to a macro, text is replaced by the macro value.
    /// Otherwise the same text is returned.
    const std::string ApplyMacro(
      const std::string& text, 
      const std::string& lexer = "global");

    /// Applies margin text style to stc line.
    void ApplyMarginTextStyle(stc* stc, int line) const;

    /// Finds a lexer specified by a filename (fullname).
    const lexer FindByFileName(const std::string& fullname) const;

    /// Finds a lexer specified by the (display scintilla) name.
    const lexer FindByName(const std::string& name) const;

    /// Finds a lexer if text starts with some special tokens.
    const lexer FindByText(const std::string& text) const;

    /// Returns the lexers object.
    /// If this is the first invocation, and createOnDemand is true,
    /// it also invokes LoadDocument.
    static lexers* Get(bool createOnDemand = true);

    /// Returns the filename.
    const auto & GetFileName() const {return m_Path;};
    
    /// Returns indicator from loaded indicators,
    /// based on the no of specified indicator.
    const indicator GetIndicator(const indicator& indicator) const;
    
    /// Returns the keywords for the specified named set of keywords.
    /// Returns empty string if set does not exist.
    const std::string GetKeywords(const std::string& set) const;

    /// Returns the lexers.
    const auto & GetLexers() const {return m_Lexers;};

    /// Returns the macros for specified lexer.
    const auto & GetMacros(const std::string& lexer) {return m_Macros[lexer];};

    /// Returns marker from loaded markers,
    /// based on the no of specified marker.
    const marker GetMarker(const marker& marker) const;
    
    /// Returns global properties.
    const auto & GetProperties() const {return m_GlobalProperties;};

    /// Returns the current theme.
    const auto & GetTheme() const {return m_Theme;};
    
    /// Returns the theme macros for the current theme.
    const auto & GetThemeMacros() {return m_ThemeMacros[m_Theme];};

    /// Returns number of themes (should at least contain empty theme).
    auto GetThemes() const {return m_ThemeMacros.size();};
    
    /// Returns true if specified indicator is available.
    bool IndicatorIsLoaded(const indicator& indic) const {
      return m_Indicators.find(indic) != m_Indicators.end();};

    /// Loads all lexers (first clears them) from document.
    /// Returns true if the document is loaded.
    bool LoadDocument();

    /// Returns true if specified marker is available.
    bool MarkerIsLoaded(const marker& marker) const {
      return m_Markers.find(marker) != m_Markers.end();};

    /// Resets the theme.
    void ResetTheme() {
      if (!m_Theme.empty()) 
      {
        m_ThemePrevious = m_Theme; 
        m_Theme.clear();
      }};
    
    /// Restores the theme from previous theme.
    void RestoreTheme() {m_Theme = m_ThemePrevious;};
    
    /// Sets the object as the current one, returns the pointer 
    /// to the previous current object 
    /// (both the parameter and returned value may be nullptr). 
    static lexers* Set(lexers* lexers);
    
    /// Shows a dialog with all lexers, allowing you to choose one.
    /// Returns true and sets the lexer on the stc component if you selected one.
    bool ShowDialog(stc* stc) const;
      
    /// Shows a dialog with all themes, allowing you to choose one.
    /// Returns true and sets current theme if you select one.
    bool ShowThemeDialog(wxWindow* parent);
  private:
    lexers(const path& filename);
    void ParseNodeFolding(const pugi::xml_node& node);
    void ParseNodeGlobal(const pugi::xml_node& node);
    void ParseNodeKeyword(const pugi::xml_node& node);
    void ParseNodeMacro(const pugi::xml_node& node);
    void ParseNodeTheme(const pugi::xml_node& node);
    void ParseNodeThemes(const pugi::xml_node& node);

    std::map<std::string, std::string> m_DefaultColours, m_Keywords;

    std::map<std::string, std::map<std::string, std::string> > 
      m_Macros, m_ThemeColours, m_ThemeMacros;

    std::set<indicator> m_Indicators;
    std::set<marker> m_Markers;

    std::vector<property> m_GlobalProperties;
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
    
    int m_StyleNoTextMargin {-1};

    static inline lexers* m_Self = nullptr;
  };
};
