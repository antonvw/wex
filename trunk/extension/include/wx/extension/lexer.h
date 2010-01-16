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

class wxExLexers;

/// This class defines a lexer using file associations,
/// syntax colouring and comment definitions.
/// This lexer is one of the Scintilla lexers.
/// The lexers are read by and kept in the wxExLexers class.
class wxExLexer
{
  friend class wxExLexers;
public:
  /// Gets the colourings.
  const wxString& GetColourings() const {return m_Colourings;};

  /// Gets the comment begin.
  const wxString& GetCommentBegin() const {return m_CommentBegin;};

  /// Gets the comment begin 2.
  const wxString& GetCommentBegin2() const {return m_CommentBegin2;};

  /// Gets the comment end.
  const wxString& GetCommentEnd() const {return m_CommentEnd;};

  /// Gets the comment end 2.
  const wxString& GetCommentEnd2() const {return m_CommentEnd2;};

  /// Gets the keywords.
  const std::set<wxString>& GetKeywords() const {return m_Keywords;};

  /// Gets all the keywords sets.
  const std::map< int, std::set<wxString> >& GetKeywordsSet() const {return m_KeywordsSet;};

  /// Gets the keywords as one large string, if keyword_set -1 take all the sets,
  /// otherwise take the specified set.
  const wxString GetKeywordsString(int keyword_set = -1) const;

  /// Gets the properties.
  const wxString& GetProperties() const {return m_Properties;};

  /// Gets the scintilla lexer.
  const wxString& GetScintillaLexer() const {return m_ScintillaLexer;};

  /// Is this word a keyword (allways all keywords), case sensitive.
  bool IsKeyword(const wxString& word) const;

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

  /// Adds the specified keywords to the keywords map and the keywords set.
  /// The text might contain the keyword set after a ':'.
  /// Returns true if keyword could be added and false if specified set is illegal.
  /// Empties existing keywords.
  bool SetKeywords(const wxString& text);

  /// Returns number of chars that fit on a line, skipping comment chars.
  int UsableCharactersPerLine() const;
private:
  const wxString GetFormattedText(
    const wxString& lines,
    const wxString& header,
    bool fill_out_with_space,
    bool fill_out) const;
  const wxString GetKeywordsStringSet(const std::set<wxString>& kset) const;

  wxString m_Colourings;
  wxString m_CommentBegin;
  wxString m_CommentBegin2;
  wxString m_CommentEnd;
  wxString m_CommentEnd2;
  wxString m_Extensions;
  wxString m_Properties;
  wxString m_ScintillaLexer;

  std::set<wxString> m_Keywords; // all keywords
  std::map< int, std::set<wxString> > m_KeywordsSet; // each keyword set in a separate keyword set
};
#endif
