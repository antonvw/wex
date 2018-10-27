////////////////////////////////////////////////////////////////////////////////
// Name:      stream.h
// Purpose:   Declaration of class 'wex::listview_stream'
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/stream.h>

namespace wex
{
  class history_frame;
  class listview;

  /// Offers a stream with reporting to a listview.
  class listview_stream : public stream
  {
  public:
    /// Constructor.
    listview_stream(
      const path& filename,
      const tool& tool);

    /// Sets up the tool.
    static bool SetupTool(
      /// tool to use
      const tool& tool, 
      /// frame
      history_frame* frame,
      /// listview to which is reported, if nullptr,
      /// calls Activate on frame to find report
      listview* report = nullptr);
  private:
    /// The comment type.
    enum comment_type
    {
      COMMENT_NONE = 0,  ///< no comment
      COMMENT_BEGIN,     ///< begin of comment
      COMMENT_END,       ///< end of comment
      COMMENT_BOTH,      ///< begin or end of comment
      COMMENT_INCOMPLETE ///< within a comment
    };

    /// The syntax type.
    enum syntax_type
    {
      SYNTAX_NONE = 0, ///< no syntax
      SYNTAX_ONE,      ///< syntax according to comment begin1 and end1
      SYNTAX_TWO       ///< syntax according to comment begin2 and end2
    };

    comment_type CheckCommentSyntax(
      const std::string& syntax_begin,
      const std::string& syntax_end,
      const std::string& text) const;

    /// Returns the actual begin of comment, depending on the syntax type.
    const std::string CommentBegin() const {
      return (m_SyntaxType == SYNTAX_NONE || m_SyntaxType == SYNTAX_ONE) ?
        GetFileName().GetLexer().GetCommentBegin() :
        GetFileName().GetLexer().GetCommentBegin2();};

    /// Returns the last end of comment detected, depending on the last syntax type.
    const std::string CommentEnd() const {
      return (m_LastSyntaxType == SYNTAX_NONE || m_LastSyntaxType == SYNTAX_ONE) ?
        GetFileName().GetLexer().GetCommentEnd() :
        GetFileName().GetLexer().GetCommentEnd2();};

    /// Check whether specified text result in a comment.
    comment_type CheckForComment(const std::string& text);
    void CommentStatementEnd();
    void CommentStatementStart();
    std::string Context(const std::string& line, int pos) const;
    
    // Implement interface from stream.
    virtual bool Process(std::string& line, size_t line_no) override;
    virtual bool ProcessBegin() override;
    virtual void ProcessEnd() override;
    virtual void ProcessMatch(const std::string& line, size_t line_no, int pos) override;
    
    static listview* m_Report;
    static history_frame* m_Frame;

    bool m_IsCommentStatement = false;
    bool m_IsString = false;

    int m_ContextSize;
    
    syntax_type m_LastSyntaxType = SYNTAX_NONE;
    syntax_type m_SyntaxType = SYNTAX_NONE;
  };
};
