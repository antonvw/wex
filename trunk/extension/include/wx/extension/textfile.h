/******************************************************************************\
* File:          textfile.h
* Purpose:       Declaration of wxExTextFile class
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _EXTEXTFILE_H
#define _EXTEXTFILE_H

#include <wx/datetime.h>
#include <wx/textfile.h>
#include <wx/extension/statistics.h>
#include <wx/extension/tool.h>

class wxExConfig;
class wxExLexers;
class wxExTextFile;

/// Class for keeping RCS information.
class wxExRCS
{
  friend class wxExTextFile;
public:
  /// Default constructor.
  wxExRCS();

  /// Gets the description.
  const wxString& GetDescription() const {return m_Description;};

  /// Gets a revision string from number, time, user and description.
  const wxString GetRevision() const;

  /// Gets the revision format.
  const wxString& GetRevisionFormat() const {return m_RevisionFormat;};

  /// Gets the revision number.
  const wxString& GetRevisionNumber() const {return m_RevisionNumber;};

  /// Gets the revision time.
  const wxDateTime& GetRevisionTime() const {return m_RevisionTime;};

  /// Gets the user.
  const wxString& GetUser() const {return m_User;};

  /// Sets revision info from specified text.
  bool SetRevision(wxString& text);
private:
  wxString m_Description;
  wxString m_RevisionFormat;
  wxString m_RevisionNumber;
  wxDateTime m_RevisionTime;
  wxString m_User;
};

/// Adds file tool methods to wxTextFile.
/// In your derived class just implement the Report or ReportStatistics, and take
/// care that the strings are added to your component.
class wxExTextFile : public wxTextFile
{
public:
  /// Constructor.
  wxExTextFile(
    const wxExFileName& filename,
    const wxExTool& tool,
    wxExConfig* config,
    const wxExLexers* lexers);

  /// Gets the filename.
  const wxExFileName& GetFileName() const {return m_FileNameStatistics;};

  /// Gets the RCS data.
  const wxExRCS& GetRCS() const {return m_RCS;};

  /// Gets the statistics.
  const wxExFileNameStatistics& GetStatistics() const {return m_FileNameStatistics;}

  /// Gets the tool.
  const wxExTool& GetTool() const {return m_Tool;};

  /// Inserts a line at current line (or at end if at end),
  /// make that line current and sets modified.
  void InsertLine(const wxString& line);

  /// Runs the tool (opens the file before running and closes afterwards).
  bool RunTool();

  /// Writes a comment to the current line.
  void WriteComment(
    const wxString& text,
    const bool fill_out = false,
    const bool fill_out_with_space = false) {
    InsertLine(m_FileNameStatistics.GetLexer().MakeComment(
      text,
      fill_out,
      fill_out_with_space));};
protected:
  // Interface.
  /// If it returns true, the operation is cancelled.
  virtual bool Cancelled() {return false;};

  /// Called after comments have been found.
  virtual bool ParseComments();

  // Virtual report generators.
  /// This one is invoked during parsing of lines.
  virtual void Report() {;};

  /// This one is invoked for ID_TOOL_LINE_CODE,
  /// for each line that contains code, or
  /// for ID_TOOL_LINE_COMMENTS for each line that contains a comment.
  virtual void ReportLine(const wxString& WXUNUSED(line)) {;};
public:
  /// This one is invoked at the end, when statistics are completed.
  /// It is made public, as it can be useful from outside.
  virtual void ReportStatistics() {;};
protected:
  /// Clears the comments.
  void ClearComments() {m_Comments.clear();}

  /// Your derived class is allowed to update statistics.
  wxExStatistics<long>& GetStatisticElements() {return m_FileNameStatistics.GetElements();};

  /// Your derived class is allowed to update statistics.
  wxExStatistics<long>& GetStatisticKeywords() {return m_FileNameStatistics.GetKeywords();};

  /// Gets the current comments.
  const wxString& GetComments() const {return m_Comments;};

  /// Parses the specified line, and invokes actions depending on the tool,
  /// and fills the comments if any on the line.
  /// At the end it calls ParseComments.
  bool ParseLine(const wxString& line);
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
    SYNTAX_TWO,      ///< syntax according to comment begin2 and end2
  };

  wxExCommentType CheckCommentSyntax(
    const wxString& syntax_begin,
    const wxString& syntax_end,
    const wxString& text) const;

  /// Gets the actual begin of comment, depending on the syntax type.
  const wxString CommentBegin() const {
    return (m_SyntaxType == SYNTAX_NONE || m_SyntaxType == SYNTAX_ONE) ?
      m_FileNameStatistics.GetLexer().GetCommentBegin() :
      m_FileNameStatistics.GetLexer().GetCommentBegin2();};

  /// Gets the last end of comment detected, depending on the last syntax type.
  const wxString CommentEnd() const {
    return (m_LastSyntaxType == SYNTAX_NONE || m_LastSyntaxType == SYNTAX_ONE) ?
      m_FileNameStatistics.GetLexer().GetCommentEnd() :
      m_FileNameStatistics.GetLexer().GetCommentEnd2();};

  /// Check whether specified text result in a comment.
  wxExCommentType CheckForComment(const wxString& text);
  void CommentStatementEnd();
  void CommentStatementStart();
  void EndCurrentRevision();
  void Initialize();

  /// Returns true if char is a brace open or close character.
  bool IsBrace(int c) const {
    return c == '[' || c == ']' ||
           c == '(' || c == ')' ||
           c == '{' || c == '}' ||
           c == '<' || c == '>';};
  /// Returns true if char is a code word separator.
  bool IsCodewordSeparator(int c) const {
    return (isspace(c) || IsBrace(c) || c == ',' || c == ';' || c == ':');};
  /// Returns true if char is alphanumeric or a _ sign.
  bool IsWordCharacter(int c) const {
    return isalnum(c) || c == '_';};

  bool MatchLine(wxString& line);
  bool Parse();
  bool PrepareRevision();

  bool m_AllowAction;
  bool m_EmptyLine;
  bool m_FinishedAction;
  bool m_IsCommentStatement;
  bool m_IsString;
  bool m_Modified;
  bool m_RevisionActive;

  wxExFileNameStatistics m_FileNameStatistics;
  wxExRCS m_RCS;
  wxExSyntaxType m_LastSyntaxType;
  wxExSyntaxType m_SyntaxType;
  const wxExTool m_Tool;
  wxExConfig* m_Config;
  const wxExLexers* m_Lexers;

  size_t m_LineMarker;
  size_t m_LineMarkerEnd;
  size_t m_VersionLine;

  wxString m_Comments;
};
#endif
