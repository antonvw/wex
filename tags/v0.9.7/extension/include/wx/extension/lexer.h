/******************************************************************************\
* File:          lexer.h
* Purpose:       Declaration of lexer classes
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _EXLEXER_H
#define _EXLEXER_H

#include <map>
#include <set>
#include <vector>
#include <wx/xml/xml.h>
#include <wx/extension/property.h>
#include <wx/extension/style.h>

class wxStyledTextCtrl;

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

  /// Applies this lexer to stc component
  /// (and colours the component).
  void Apply(wxStyledTextCtrl* stc, bool clear = true) const;

  /// Sets scintilla lexer for specified lexer and stc. 
  /// Returns true if a scintilla lexer has been set.
  /// The is ok member is set as well according to whether the
  /// lexer could be set. Calls Apply.
  bool ApplyLexer(
    /// lexer to use
    const wxString& lexer, 
    /// stc component on which to apply
    wxStyledTextCtrl* stc,
    /// If show_error is true, a log error message is given
    /// if a lexer was specified, but could not be set.
    bool show_error = true,
    /// if clear is true, old styles are reset (including folding)
    bool clear = true);

  /// Gets the comment begin.
  const wxString& GetCommentBegin() const {return m_CommentBegin;};

  /// Gets the comment begin 2.
  const wxString& GetCommentBegin2() const {return m_CommentBegin2;};

  /// Gets the comment end.
  const wxString& GetCommentEnd() const {return m_CommentEnd;};

  /// Gets the comment end 2.
  const wxString& GetCommentEnd2() const {return m_CommentEnd2;};

  /// Gets the extensions.
  const wxString& GetExtensions() const {return m_Extensions;};

  /// Gets the keywords.
  const std::set<wxString>& GetKeywords() const {return m_Keywords;};

  /// Gets the keywords as one large string, 
  /// if keyword_set -1 take all the sets,
  /// otherwise take the specified set.
  const wxString GetKeywordsString(int keyword_set = -1) const;

  /// Gets the scintilla lexer.
  const wxString& GetScintillaLexer() const {return m_ScintillaLexer;};

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

  /// Sets keywords (public for testing only).
  bool SetKeywords(const wxString& text);
  
  /// Override a local property.
  void SetProperty(const wxString& name, const wxString& value);

  /// Returns number of chars that fit on a line, skipping comment chars.
  int UsableCharactersPerLine() const;
private:
  void AutoMatch(const wxString& lexer);
  const wxString GetFormattedText(
    const wxString& lines,
    const wxString& header,
    bool fill_out_with_space,
    bool fill_out) const;
  const wxString GetKeywordsStringSet(const std::set<wxString>& kset) const;
  void Initialize();
  const std::vector<wxExStyle> ParseNodeStyles(const wxXmlNode* node) const;
  void Set(const wxXmlNode* node);

  wxString m_CommentBegin;
  wxString m_CommentBegin2;
  wxString m_CommentEnd;
  wxString m_CommentEnd2;
  wxString m_Extensions;

  // The scintilla name for this lexer.
  // Cannot be const, as in wxExFileName the operator= is used on a lexer.
  wxString m_ScintillaLexer;

  std::set<wxString> m_Keywords;
  std::vector<wxExProperty> m_Properties;
  std::vector<wxExStyle> m_Styles;

  // each keyword set in a separate keyword set
  std::map< int, std::set<wxString> > m_KeywordsSet;
  
  bool m_IsOk;
};
#endif
