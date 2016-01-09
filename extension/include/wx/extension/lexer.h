////////////////////////////////////////////////////////////////////////////////
// Name:      lexer.h
// Purpose:   Declaration of wxExLexer class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <map>
#include <set>
#include <vector>
#include <wx/extension/property.h>
#include <wx/extension/style.h>

class wxStyledTextCtrl;
class wxXmlNode;

/// This class defines a lexer using file extensions,
/// syntax colouring and comment definitions.
/// This lexer is one of the Scintilla lexers.
/// The lexers are read by and kept in the wxExLexers class.
class WXDLLIMPEXP_BASE wxExLexer
{
public:
  /// Default constructor.
  wxExLexer();

  /// Constructor using xml node.
  wxExLexer(const wxXmlNode* node);

  /// Constructor using other lexer.
  wxExLexer(
    /// lexer to use
    const wxString& lexer, 
    /// stc component on which to apply
    wxStyledTextCtrl* stc = nullptr,
    /// if clear is true, old styles are reset (including folding)
    bool clear = true);
    
  /// Adds keywords (public for testing only).
  bool AddKeywords(const wxString& text, int setno = 0);
  
  /// Applies this lexer to stc component
  /// (and colours the component).
  void Apply(wxStyledTextCtrl* stc, bool clear = true) const;

  /// Returns a string that completes specified comment,
  /// by adding spaces and a comment end at the end.
  /// If the comment end string is empty, it returns empty string.
  const wxString CommentComplete(const wxString& comment) const;
    
  /// Returns the comment begin.
  const wxString& GetCommentBegin() const {return m_CommentBegin;};

  /// Returns the comment begin 2.
  const wxString& GetCommentBegin2() const {return m_CommentBegin2;};

  /// Returns the comment end.
  const wxString& GetCommentEnd() const {return m_CommentEnd;};

  /// Returns the comment end 2.
  const wxString& GetCommentEnd2() const {return m_CommentEnd2;};

  /// Returns the display lexer (as shown in dialog).
  const wxString& GetDisplayLexer() const {return m_DisplayLexer;};

  /// Returns the extensions.
  const wxString& GetExtensions() const {return m_Extensions;};

  /// Returns the keywords.
  const auto & GetKeywords() const {return m_Keywords;};

  /// Returns the keywords as one large string, 
  const wxString GetKeywordsString(
    /// if keyword_set -1 take all the sets,
    /// otherwise take the specified set.
    int keyword_set = -1,
    /// if min_size 0, use all keywords,
    /// otherwise use keywords with minimim size
    size_t min_size = 0,
    /// prefix keyword should start with
    const wxString& prefix = wxEmptyString) const;

  /// Returns the language.
  const wxString& GetLanguage() const {return m_Language;};
  
  /// Returns the properties.
  const auto & GetProperties() const {return m_Properties;};
  
  /// Returns the scintilla lexer.
  const wxString& GetScintillaLexer() const {return m_ScintillaLexer;};

  /// Returns the styles.
  const auto & GetStyles() const {return m_Styles;};
  
  /// Is this word a keyword (allways all keywords), case sensitive.
  bool IsKeyword(const wxString& word) const;

  /// Is this lexer valid.
  bool IsOk() const {return m_IsOk;};

  /// Does any keyword (allways all keywords) start with this word,
  /// case insensitive.
  bool KeywordStartsWith(const wxString& word) const;

  /// Returns a lexer comment string with text formatted.
  const wxString MakeComment(
    const wxString& text,
    bool fill_out_with_space = true,
    bool fill_out = true) const;

  /// Returns a lexer comment string with prefix.
  const wxString MakeComment(
    const wxString& prefix,
    const wxString& text) const;

  /// Returns a lexer comment string filled out over one line.
  const wxString MakeSingleLineComment(
    const wxString& text,
    bool fill_out_with_space = true,
    bool fill_out = true) const;
    
  /// Resets lexer and if ok applies it to stc.
  /// Returns true if the scintilla lexer has been reset.
  /// The is ok member is set according to whether the
  /// lexer could be reset.
  bool Reset(wxStyledTextCtrl* stc);

  /// Sets scintilla lexer for specified lexer and if ok applies it to stc. 
  /// Returns true if a scintilla lexer has been set.
  /// The is ok member is set according to whether the
  /// lexer could be set. Shows error message when lexer could not be set.
  bool Set(
    /// lexer to use
    const wxString& lexer, 
    /// stc component on which to apply
    wxStyledTextCtrl* stc,
    /// if clear is true, old styles are reset (including folding)
    bool clear = true);
    
  /// Sets lexer to specified lexer.
  /// Returns true if a scintilla lexer has been set.
  /// The is ok member is set according to whether the
  /// lexer could be set.
  bool Set(
    /// lexer to use
    const wxExLexer& lexer, 
    /// stc component on which to apply
    wxStyledTextCtrl* stc);
      
  /// Overrides a local property.
  void SetProperty(const wxString& name, const wxString& value);

  /// Returns number of chars that fit on a line, skipping comment chars.
  int UsableCharactersPerLine() const;
private:
  void ApplyWhenSet(const wxString& lexer, wxStyledTextCtrl* stc, bool clear);
  void AutoMatch(const wxString& lexer);
  const wxString GetFormattedText(
    const wxString& lines,
    const wxString& header,
    bool fill_out_with_space,
    bool fill_out) const;
  const wxString GetKeywordsStringSet(
    const std::set<wxString>& kset, 
    size_t min_size = 0,
    const wxString& prefix = wxEmptyString) const;
  void Initialize();
  void Set(const wxXmlNode* node);

  wxString m_CommentBegin;
  wxString m_CommentBegin2;
  wxString m_CommentEnd;
  wxString m_CommentEnd2;
  wxString m_Extensions;
  wxString m_Language; // e.g. xml

  // The scintilla name for this lexer.
  // Cannot be const, as in wxExFileName the operator= is used on a lexer.
  wxString m_ScintillaLexer;

  // Normally the lexer displayed is the scintilla lexer,
  // however this might be different, as with c#.
  // In that case the scintilla lexer is cpp, whereas the
  // display lexer is c#.  
  wxString m_DisplayLexer;

  std::set<wxString> m_Keywords;
  std::vector<wxExProperty> m_Properties;
  std::vector<wxExStyle> m_Styles;

  // each keyword set in a separate keyword set
  std::map< int, std::set<wxString> > m_KeywordsSet;
  
  bool m_IsOk;
};
