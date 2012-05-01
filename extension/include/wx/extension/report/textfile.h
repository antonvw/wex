////////////////////////////////////////////////////////////////////////////////
// Name:      textfile.h
// Purpose:   Declaration of class 'wxExTextFileWithListView'
// Author:    Anton van Wezenbeek
// Copyright: (c) 2012 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EX_REPORT_TEXTFILE_H
#define _EX_REPORT_TEXTFILE_H

#include <wx/extension/textfile.h>
#include <wx/extension/otl.h>

class wxExFrameWithHistory;
class wxExListView;

/// Offers a wxExTextFile with reporting to a listview.
class WXDLLIMPEXP_BASE wxExTextFileWithListView : public wxExTextFile
{
public:
  /// Constructor.
  wxExTextFileWithListView(
    const wxExFileName& filename,
    const wxExTool& tool);

  /// Sets up the tool.
  static bool SetupTool(
    /// tool to use
    const wxExTool& tool, 
    /// frame
    wxExFrameWithHistory* frame,
    /// listview to which is reported, if NULL,
    /// calls Activate on frame to find report
    wxExListView* report = NULL);
private:
  /// The comment type.
  enum wxExCommentType
  {
    COMMENT_NONE = 0,  ///< no comment
    COMMENT_BEGIN,     ///< begin of comment
    COMMENT_END,       ///< end of comment
    COMMENT_BOTH,      ///< begin or end of comment
    COMMENT_INCOMPLETE ///< within a comment
  };

  /// The syntax type.
  enum wxExSyntaxType
  {
    SYNTAX_NONE = 0, ///< no syntax
    SYNTAX_ONE,      ///< syntax according to comment begin1 and end1
    SYNTAX_TWO       ///< syntax according to comment begin2 and end2
  };

  wxExCommentType CheckCommentSyntax(
    const wxString& syntax_begin,
    const wxString& syntax_end,
    const wxString& text) const;

  /// Gets the actual begin of comment, depending on the syntax type.
  const wxString CommentBegin() const {
    return (m_SyntaxType == SYNTAX_NONE || m_SyntaxType == SYNTAX_ONE) ?
      GetFileName().GetLexer().GetCommentBegin() :
      GetFileName().GetLexer().GetCommentBegin2();};

  /// Gets the last end of comment detected, depending on the last syntax type.
  const wxString CommentEnd() const {
    return (m_LastSyntaxType == SYNTAX_NONE || m_LastSyntaxType == SYNTAX_ONE) ?
      GetFileName().GetLexer().GetCommentEnd() :
      GetFileName().GetLexer().GetCommentEnd2();};

  /// Check whether specified text result in a comment.
  wxExCommentType CheckForComment(const wxString& text);
  void CommentStatementEnd();
  void CommentStatementStart();
  
  /// Returns true if char is a brace open or close character.
  bool IsBrace(int c) const {
    return c == '[' || c == ']' ||
           c == '(' || c == ')' ||
           c == '{' || c == '}' ||
           c == '<' || c == '>';};
  /// Returns true if char is a code word separator.
  bool IsCodewordSeparator(int c) const {
    return (isspace(c) || IsBrace(c) || c == ',' || c == ';' || c == ':');};

  /// Inserts a line at current line (or at end if at end),
  /// make that line current and sets modified.
  void InsertLine(const wxString& line);

  /// Parses the specified line, and invokes actions depending on the tool,
  /// and fills the comments if any on the line.
  /// At the end it calls ParseComments.
  bool ParseLine(const wxString& line);
  
  // Implement interface from wxExTextFile.
  virtual bool Parse();
  virtual void Report(size_t line);
  
  void ReportKeyword();

  static wxExListView* m_Report;
  static wxExFrameWithHistory* m_Frame;

  bool m_IsCommentStatement;
  bool m_IsString;

  wxString m_Comments;
  
  wxExSyntaxType m_LastSyntaxType;
  wxExSyntaxType m_SyntaxType;
  
#if wxExUSE_EMBEDDED_SQL
  bool ParseComments();
  bool ParseSQL();
  bool SetSQLQuery();

  bool m_SQLResultsParsing;

  static wxExOTL m_otl;

  wxString m_SQLQuery;
  wxString m_SQLQueryRunTime;
#endif
};
#endif
