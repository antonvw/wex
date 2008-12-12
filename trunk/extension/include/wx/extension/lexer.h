/******************************************************************************\
* File:          lexer.h
* Purpose:       Declaration of lexer classes
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2008, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _EXLEXER_H
#define _EXLEXER_H

#include <set>
#include <vector>

/// This class defines a lexer using file associations,
/// syntax colouring and comment definitions.
/// This lexer is one of the Scintilla lexers.
/// The lexers are read by and kept in the exLexers class.
class exLexer
{
  friend class exLexers;
public:
  /// The comment type.
  enum exCommentType
  {
    COMMENT_NONE = 0,  ///< no comment
    COMMENT_BEGIN,     ///< begin of comment
    COMMENT_END,       ///< end of comment
    COMMENT_BOTH,      ///< begin or end of comment
    COMMENT_INCOMPLETE ///< within a comment
  };

  /// The syntax type.
  enum exSyntaxType
  {
    SYNTAX_NONE = 0, ///< no syntax
    SYNTAX_ONE,      ///< syntax according to comment begin1 and end1
    SYNTAX_TWO,      ///< syntax according to comment begin2 and end2
  };

  /// Check whether specified chars result in a comment.
  exCommentType CheckForComment(wxChar c1, wxChar c2) const;

  /// Gets the actual begin of comment, depending on the syntax type.
  const wxString CommentBegin() const {
    return (m_SyntaxType == SYNTAX_NONE || m_SyntaxType == SYNTAX_ONE) ?
      m_CommentBegin : m_CommentBegin2;};

  /// Gets the actual end of comment, depending on the syntax type.
  const wxString CommentEnd() const {
    return (m_SyntaxType == SYNTAX_NONE || m_SyntaxType == SYNTAX_ONE ) ?
      m_CommentEnd : m_CommentEnd2;};

  /// Gets the last end of comment detected, depending on the last syntax type.
  const wxString CommentEndDetected() const {
    return (m_LastSyntaxType == SYNTAX_NONE || m_LastSyntaxType == SYNTAX_ONE) ?
      m_CommentEnd : m_CommentEnd2;};

  /// Returns a lexer comment string with text formatted.
  const wxString FormatText(
    const wxString& text,
    bool fill_out,
    bool fill_out_with_space = false) const;

  /// Gets the associations, e.g. *.cpp;*.c;*.cc;*.h;*.hpp;.
  const wxString& GetAssociations() const {return m_Associations;};

  /// Gets the colourings.
  const wxString& GetColourings() const {return m_Colourings;};

  /// Gets the keywords.
  const std::set<wxString>& GetKeywords() const {return m_Keywords;};

  /// Gets the keywords set.
  const std::vector< std::set<wxString> >& GetKeywordsSet() const {return m_KeywordsSet;};

  /// Gets the keywords as one large string, if keyword_set -1 take all the sets,
  /// otherwise take the specified set.
  const wxString GetKeywordsString(int keyword_set = -1) const;

  /// Gets the properties.
  const wxString& GetProperties() const {return m_Properties;};

  /// Gets the scintilla lexer.
  const wxString& GetScintillaLexer() const {return m_ScintillaLexer;};

  /// Is this word a keyword (allways all keywords).
  bool IsKeyword(const wxString& word) const;

  /// Does any keyword (allways all keywords) start with this word.
  bool KeywordStartsWith(const wxString& word) const;

  /// Adds the keywords from value to keywords and keywords set.
  void SetKeywords(const wxString& value);

  /// Sets the lexer if text starts with some special tokens.
  void SetLexerFromText(const wxString& text);

  /// Returns number of chars that fit on a line, skipping comment chars.
  int UsableCharactersPerLine() const;
private:
  wxString m_Associations;
  wxString m_Colourings;
  wxString m_CommentBegin;
  wxString m_CommentBegin2;
  wxString m_CommentEnd;
  wxString m_CommentEnd2;
  wxString m_Properties;
  wxString m_ScintillaLexer;

  std::set<wxString> m_Keywords; // all keywords
  std::vector< std::set<wxString> > m_KeywordsSet; // each keyword set in a separate keyword set

  static exSyntaxType m_LastSyntaxType;
  static exSyntaxType m_SyntaxType;
};

/// This class defines our markers, closely related to scintilla markers.
class exMarker
{
public:
  /// Constructor.
  exMarker(
    int markerNumber,
    int markerSymbol,
    const wxColour& foreground = wxNullColour,
    const wxColour& background = wxNullColour)
    : m_MarkerNumber(markerNumber)
    , m_MarkerSymbol(markerSymbol)
    , m_BackgroundColour(background)
    , m_ForegroundColour(foreground) {}

  /// Gets the background colour.
  const wxColour& GetBackgroundColour() const {return m_BackgroundColour;};

  /// Gets the foreground colour.
  const wxColour& GetForegroundColour() const {return m_ForegroundColour;};

  /// Gets the marker number.
  unsigned int GetMarkerNumber() const {return m_MarkerNumber;};

  /// Gets the marker symbol.
  unsigned int GetMarkerSymbol() const {return m_MarkerSymbol;};
private:
  unsigned int m_MarkerNumber;
  unsigned int m_MarkerSymbol;
  wxColour m_BackgroundColour;
  wxColour m_ForegroundColour;
};
#endif
