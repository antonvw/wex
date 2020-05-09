////////////////////////////////////////////////////////////////////////////////
// Name:      stream.h
// Purpose:   Declaration of class 'wex::report::stream'
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/stream.h>

namespace wex
{
  class listview;
};

namespace wex::report
{
  class frame;

  /// Offers a stream with reporting to a listview.
  class stream : public wex::stream
  {
  public:
    /// Constructor.
    stream(const path& filename, const tool& tool);

    /// Sets up the tool.
    static bool setup_tool(
      /// tool to use
      const tool& tool,
      /// frame
      report::frame* frame,
      /// listview to which is reported, if nullptr,
      /// calls activate on frame to find report
      wex::listview* report = nullptr);

  private:
    /// The comment type.
    enum comment_t
    {
      COMMENT_NONE = 0,  ///< no comment
      COMMENT_BEGIN,     ///< begin of comment
      COMMENT_END,       ///< end of comment
      COMMENT_BOTH,      ///< begin or end of comment
      COMMENT_INCOMPLETE ///< within a comment
    };

    /// The syntax type.
    enum syntax_t
    {
      SYNTAX_NONE = 0, ///< no syntax
      SYNTAX_ONE,      ///< syntax according to comment begin1 and end1
      SYNTAX_TWO       ///< syntax according to comment begin2 and end2
    };

    comment_t check_comment_syntax(
      const std::string& syntax_begin,
      const std::string& syntax_end,
      const std::string& text) const;

    /// Returns the actual begin of comment, depending on the syntax type.
    const std::string comment_begin() const
    {
      return (m_syntax_type == SYNTAX_NONE || m_syntax_type == SYNTAX_ONE) ?
               get_filename().lexer().comment_begin() :
               get_filename().lexer().comment_begin2();
    };

    /// Returns the last end of comment detected, depending on the last syntax
    /// type.
    const std::string comment_end() const
    {
      return (m_last_syntax_type == SYNTAX_NONE ||
              m_last_syntax_type == SYNTAX_ONE) ?
               get_filename().lexer().comment_end() :
               get_filename().lexer().comment_end2();
    };

    /// Check whether specified text result in a comment.
    comment_t   check_for_comment(const std::string& text);
    void        comment_statement_end();
    void        comment_statement_start();
    std::string context(const std::string& line, int pos) const;

    // Overridden methods from stream.

    bool process(std::string& line, size_t line_no) override;
    bool process_begin() override;
    void process_end() override;
    void
    process_match(const std::string& line, size_t line_no, int pos) override;

    static wex::listview* m_report;
    static report::frame* m_frame;

    bool m_is_comment_statement{false}, m_is_string{false};

    int m_context_size;

    syntax_t m_last_syntax_type{SYNTAX_NONE}, m_syntax_type{SYNTAX_NONE};
  };
}; // namespace wex::report
