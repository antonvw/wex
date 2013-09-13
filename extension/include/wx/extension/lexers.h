////////////////////////////////////////////////////////////////////////////////
// Name:      lexers.h
// Purpose:   Declaration of wxExLexers class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2013 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXLEXERS_H
#define _EXLEXERS_H

#include <map>
#include <set>
#include <vector>
#include <wx/filename.h>
#include <wx/extension/indicator.h>
#include <wx/extension/lexer.h>
#include <wx/extension/marker.h>
#include <wx/extension/property.h>
#include <wx/extension/style.h>

class wxStyledTextCtrl;
class wxWindow;
class wxXmlNode;

/// Collection of all lexers.
/// The lexers are loaded from lexers.xml, this is done
/// automatically during the first Get call.
class WXDLLIMPEXP_BASE wxExLexers
{
public:
  /// Sets global styles (and colours and indicators) 
  /// for current theme for specified component.
  void ApplyGlobalStyles(wxStyledTextCtrl* stc);

  /// Sets hex styles for specified component.
  void ApplyHexStyles(wxStyledTextCtrl* stc) const;

  /// Sets indicators for specified component.
  void ApplyIndicators(wxStyledTextCtrl* stc) const;

  /// Applies macro to text:
  /// if text is referring to a macro, text is replaced by the macro value.
  /// Otherwise the same text is returned.
  const wxString ApplyMacro(
    const wxString& text, 
    const wxString& lexer = "global");

  /// Sets markers for specified component.
  void ApplyMarkers(wxStyledTextCtrl* stc) const;

  /// Sets properties for specified component.
  void ApplyProperties(wxStyledTextCtrl* stc) const;

  /// Builds a wildcard string from available lexers using specified filename.
  const wxString BuildWildCards(const wxFileName& filename) const;

  /// Finds a lexer specified by a filename.
  const wxExLexer FindByFileName(const wxFileName& filename) const;

  /// Finds a lexer specified by the (display scintilla) name.
  const wxExLexer FindByName(const wxString& name) const;

  /// Finds a lexer if text starts with some special tokens.
  const wxExLexer FindByText(const wxString& text) const;

  /// Returns the lexers object.
  /// If this is the first invocation, and createOnDemand is true,
  /// it also invokes LoadDocument.
  static wxExLexers* Get(bool createOnDemand = true);

  /// Returns the number of lexers.
  size_t GetCount() const {return m_Lexers.size();};

  /// Returns the default style.
  const wxExStyle& GetDefaultStyle() const {return m_DefaultStyle;};

  /// Gets the filename.
  const wxFileName& GetFileName() const {return m_FileName;};
  
  /// Gets the macros for specified lexer.
  const std::map<wxString, wxString>& GetMacros(const wxString& lexer) {
    return m_Macros[lexer];};

  /// Gets global properties.
  const std::vector<wxExProperty> & GetProperties() const {
    return m_GlobalProperties;};

  /// Returns the current theme.
  const wxString& GetTheme() const {return m_Theme;};
  
  /// Returns whether the current theme is not the empty theme.
  bool GetThemeOk() const {return GetTheme() != m_NoTheme;};
  
  /// Gets the theme macros for the current theme.
  const std::map<wxString, wxString>& GetThemeMacros() {
    return m_ThemeMacros[m_Theme];};

  /// Returns true if specified indicator is available.
  bool IndicatorIsLoaded(const wxExIndicator& indic) const;

  /// Loads all lexers (first clears them) from document.
  /// Returns true if the document is loaded.
  bool LoadDocument();

  /// Returns true if specified marker is available.
  bool MarkerIsLoaded(const wxExMarker& marker) const;

  /// Sets the object as the current one, returns the pointer 
  /// to the previous current object 
  /// (both the parameter and returned value may be NULL). 
  static wxExLexers* Set(wxExLexers* lexers);

  /// Shows a dialog with all lexers, allowing you to choose one.
  /// Returns true and fills the lexer if you selected one.
  bool ShowDialog(
    /// parent
    wxWindow* parent,
    /// If you specify an existing lexer, it is selected
    /// in the list. If you press OK, the lexer is 
    /// set to the selected lexer.
    wxString& lexer,
    /// caption
    const wxString& caption = _("Enter Lexer"),
    /// shows modal dialog
    bool show_modal = true) const;
    
  /// Shows a dialog with all themes, allowing you to choose one.
  /// Returns true and sets current theme if you select one.
  bool ShowThemeDialog(
    /// parent
    wxWindow* parent,
    /// caption
    const wxString& caption = _("Enter Theme"),
    /// shows modal dialog
    bool show_modal = true);
private:
  wxExLexers(const wxFileName& filename);
  const wxString GetLexerExtensions() const;
  void Initialize();
  void ParseNodeGlobal(const wxXmlNode* node);
  void ParseNodeMacro(const wxXmlNode* node);
  void ParseNodeTheme(const wxXmlNode* node);
  void ParseNodeThemes(const wxXmlNode* node);

  std::map<wxString, wxString> m_DefaultColours;
  std::map<wxString, std::map<wxString, wxString> > m_Macros;
  std::map<wxString, std::map<wxString, wxString> > m_ThemeColours;
  std::map<wxString, std::map<wxString, wxString> > m_ThemeMacros;

  std::set<wxExIndicator> m_Indicators;
  std::set<wxExMarker> m_Markers;

  std::vector<wxExProperty> m_GlobalProperties;
  std::vector<wxExLexer> m_Lexers;
  std::vector<wxExStyle> m_Styles;
  std::vector<wxExStyle> m_StylesHex;

  wxExStyle m_DefaultStyle;

  const wxFileName m_FileName;
  const wxString m_NoTheme;
  wxString m_Theme;

  static wxExLexers* m_Self;
};
#endif
