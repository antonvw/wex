////////////////////////////////////////////////////////////////////////////////
// Name:      lexers.h
// Purpose:   Declaration of wxExLexers class
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
class wxExSTC;

/// Collection of all lexers.
/// The lexers are loaded from lexers.xml, this is done
/// automatically during the first Get call.
class WXDLLIMPEXP_BASE wxExLexers
{
public:
  /// Applies containers (except global styles) to specified component.
  void Apply(wxExSTC* stc) const;

  /// Sets global styles (and colours and indicators) 
  /// for current theme for specified component.
  void ApplyGlobalStyles(wxExSTC* stc);

  /// Applies macro to text:
  /// if text is referring to a macro, text is replaced by the macro value.
  /// Otherwise the same text is returned.
  const std::string ApplyMacro(
    const std::string& text, 
    const std::string& lexer = "global");

  /// Applies margin text style to stc line.
  void ApplyMarginTextStyle(wxExSTC* stc, int line) const;

  /// Finds a lexer specified by a filename (fullname).
  const wxExLexer FindByFileName(const std::string& fullname) const;

  /// Finds a lexer specified by the (display scintilla) name.
  const wxExLexer FindByName(const std::string& name) const;

  /// Finds a lexer if text starts with some special tokens.
  const wxExLexer FindByText(const std::string& text) const;

  /// Returns the lexers object.
  /// If this is the first invocation, and createOnDemand is true,
  /// it also invokes LoadDocument.
  static wxExLexers* Get(bool createOnDemand = true);

  /// Returns the filename.
  const auto & GetFileName() const {return m_Path;};
  
  /// Returns indicator from loaded indicators,
  /// based on the no of specified indicator.
  const wxExIndicator GetIndicator(const wxExIndicator& indicator) const;
  
  /// Returns the keywords for the specified named set of keywords.
  /// Returns empty string if set does not exist.
  const std::string GetKeywords(const std::string& set) const;

  /// Returns the lexers.
  const auto & GetLexers() const {return m_Lexers;};

  /// Returns the macros for specified lexer.
  const auto & GetMacros(const std::string& lexer) {return m_Macros[lexer];};

  /// Returns marker from loaded markers,
  /// based on the no of specified marker.
  const wxExMarker GetMarker(const wxExMarker& marker) const;
  
  /// Returns global properties.
  const auto & GetProperties() const {return m_GlobalProperties;};

  /// Returns the current theme.
  const auto & GetTheme() const {return m_Theme;};
  
  /// Returns the theme macros for the current theme.
  const auto & GetThemeMacros() {return m_ThemeMacros[m_Theme];};

  /// Returns whether the current theme is not the empty theme.
  bool GetThemeOk() const {return GetTheme() != m_NoTheme;};
  
  /// Returns number of themes (should at least contain empty theme).
  auto GetThemes() const {return m_ThemeMacros.size();};
  
  /// Returns true if specified indicator is available.
  bool IndicatorIsLoaded(const wxExIndicator& indic) const {
    return m_Indicators.find(indic) != m_Indicators.end();};

  /// Loads all lexers (first clears them) from document.
  /// Returns true if the document is loaded.
  bool LoadDocument();

  /// Returns true if specified marker is available.
  bool MarkerIsLoaded(const wxExMarker& marker) const {
    return m_Markers.find(marker) != m_Markers.end();};

  /// Restores theme from config (after SetThemeNone).
  void RestoreTheme();
  
  /// Sets the object as the current one, returns the pointer 
  /// to the previous current object 
  /// (both the parameter and returned value may be nullptr). 
  static wxExLexers* Set(wxExLexers* lexers);
  
  /// Temporary use the no theme, until you do RestoreTheme.
  void SetThemeNone() {m_Theme = m_NoTheme;};
  
  /// Shows a dialog with all lexers, allowing you to choose one.
  /// Returns true and sets the lexer on the stc component if you selected one.
  bool ShowDialog(wxExSTC* stc) const;
    
  /// Shows a dialog with all themes, allowing you to choose one.
  /// Returns true and sets current theme if you select one.
  bool ShowThemeDialog(wxWindow* parent);
private:
  wxExLexers(const wxExPath& filename);
  void ParseNodeFolding(const pugi::xml_node& node);
  void ParseNodeGlobal(const pugi::xml_node& node);
  void ParseNodeKeyword(const pugi::xml_node& node);
  void ParseNodeMacro(const pugi::xml_node& node);
  void ParseNodeTheme(const pugi::xml_node& node);
  void ParseNodeThemes(const pugi::xml_node& node);

  std::map<std::string, std::string> m_DefaultColours;
  std::map<std::string, std::string> m_Keywords;
  std::map<std::string, std::map<std::string, std::string> > 
    m_Macros, m_ThemeColours, m_ThemeMacros;

  std::set<wxExIndicator> m_Indicators;
  std::set<wxExMarker> m_Markers;

  std::vector<wxExProperty> m_GlobalProperties;
  std::vector<wxExLexer> m_Lexers;
  std::vector<wxExStyle> m_Styles, m_StylesHex;
  std::vector<std::pair<std::string, std::string>> m_Texts;

  wxExStyle m_DefaultStyle;

  const wxExPath m_Path;
  const std::string m_NoTheme;
  std::string m_FoldingBackgroundColour, m_FoldingForegroundColour, m_Theme;
  int m_StyleNoTextMargin {-1};

  static inline wxExLexers* m_Self = nullptr;
};
