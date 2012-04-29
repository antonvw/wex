////////////////////////////////////////////////////////////////////////////////
// Name:      textfile.h
// Purpose:   Declaration of wxExTextFile class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2012 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXTEXTFILE_H
#define _EXTEXTFILE_H

#include <wx/textfile.h>
#include <wx/extension/filename.h>
#include <wx/extension/statistics.h>
#include <wx/extension/tool.h>

class wxExTextFile;

/// Offers file statistics for elements and keywords.
/// Used in wxExTextFile to keep statistics like comments and lines of code.
/// These are stored as elements.
class WXDLLIMPEXP_BASE wxExFileStatistics
{
  friend class wxExTextFile;
public:
  /// Adds other statistics.
  wxExFileStatistics& operator+=(const wxExFileStatistics& s) {
    m_Elements += s.m_Elements;
    m_Keywords += s.m_Keywords;
    return *this;}

  /// Gets all items as a string. All items are returned as a string,
  /// with newlines separating items.
  const wxString Get() const {
    return m_Elements.Get() + m_Keywords.Get();};

  /// Gets the key, first the elements are tried,
  /// if not present, the keywords are tried, if not present
  /// 0 is returned.
  long Get(const wxString& key) const;

  /// Gets the const elements.
  const wxExStatistics<long>& GetElements() const {return m_Elements;};

  /// Gets the const keywords.
  const wxExStatistics<long>& GetKeywords() const {return m_Keywords;};
private:
  wxExStatistics<long> m_Elements;
  wxExStatistics<long> m_Keywords;
};

/// Adds file tool methods to wxTextFile.
/// In your derived class just implement the Report or ReportKeyword, and take
/// care that the strings are added to your component.
class WXDLLIMPEXP_BASE wxExTextFile : public wxTextFile
{
public:
  /// Constructor.
  wxExTextFile(
    const wxExFileName& filename,
    const wxExTool& tool);

  /// Gets the filename.
  const wxExFileName& GetFileName() const {return m_FileName;};

  /// Gets the statistics.
  const wxExFileStatistics& GetStatistics() const {return m_Stats;}

  /// Gets the tool.
  const wxExTool& GetTool() const {return m_Tool;};

  /// Runs the tool (opens the file before running and closes afterwards).
  bool RunTool();
protected:
  /// Invoked after comments have been found.
  virtual bool ParseComments() {return true;};

  // Virtual report generators.
  /// This one is invoked during parsing of lines.
  virtual void Report(size_t WXUNUSED(line)) {;};
public:
  /// This one is invoked at the end, when keywords are completed.
  /// It is made public, as it can be useful from outside.
  virtual void ReportKeyword() {;};
protected:
  /// Clears the comments.
  void ClearComments() {m_Comments.clear();}

  /// Gets the current comments.
  const wxString& GetComments() const {return m_Comments;};

  /// Increments the actions completed.
  void IncActionsCompleted(long inc_value = 1) {
    m_Stats.m_Elements.Inc(_("Actions Completed"), inc_value);};

  /// Parses the specified line, and invokes actions depending on the tool,
  /// and fills the comments if any on the line.
  /// At the end it calls ParseComments.
  bool ParseLine(const wxString& line);
  
  bool m_Modified;
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
      m_FileName.GetLexer().GetCommentBegin() :
      m_FileName.GetLexer().GetCommentBegin2();};

  /// Gets the last end of comment detected, depending on the last syntax type.
  const wxString CommentEnd() const {
    return (m_LastSyntaxType == SYNTAX_NONE || m_LastSyntaxType == SYNTAX_ONE) ?
      m_FileName.GetLexer().GetCommentEnd() :
      m_FileName.GetLexer().GetCommentEnd2();};

  /// Check whether specified text result in a comment.
  wxExCommentType CheckForComment(const wxString& text);
  void CommentStatementEnd();
  void CommentStatementStart();
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

  bool m_IsCommentStatement;
  bool m_IsString;

  wxExFileName m_FileName;
  wxExFileStatistics m_Stats;
  wxExSyntaxType m_LastSyntaxType;
  wxExSyntaxType m_SyntaxType;
  const wxExTool m_Tool;

  std::string m_FindString;

  wxString m_Comments;
};
#endif
