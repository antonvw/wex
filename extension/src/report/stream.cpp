////////////////////////////////////////////////////////////////////////////////
// Name:      stream.cpp
// Purpose:   Implementation of class wex::listview_stream
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <cctype> // for isspace
#include <wex/report/stream.h>
#include <wex/config.h>
#include <wex/frd.h>
#include <wex/listitem.h>
#include <wex/util.h>
#include <wex/report/defs.h>
#include <wex/report/frame.h>
#include <wex/report/listview.h>
#include <easylogging++.h>

wex::listview* wex::listview_stream::m_Report = nullptr;
wex::history_frame* wex::listview_stream::m_Frame = nullptr;

wex::listview_stream::listview_stream(
  const path& filename,
  const tool& tool)
  : stream(filename, tool)
{
}

wex::listview_stream::comment_t wex::listview_stream::CheckCommentSyntax(
  const std::string& syntax_begin,
  const std::string& syntax_end,
  const std::string& text) const
{
  if (syntax_begin.empty() && syntax_end.empty())
  {
    return COMMENT_NONE;
  }
  
  if (syntax_begin == text)
  {
    return (syntax_end == text) ? COMMENT_BOTH: COMMENT_BEGIN;
  }
  else
  {
    if (syntax_end == text ||
       (syntax_end.empty() && text.empty()))
    {
      return COMMENT_END;
    }
  }

  if (  syntax_begin.find(text) == 0 || 
      (!syntax_end.empty() && syntax_end.find(text) == 0))
  {
    return COMMENT_INCOMPLETE;
  }

  return COMMENT_NONE;
}

wex::listview_stream::comment_t wex::listview_stream::CheckForComment(
  const std::string& text)
{
  if (get_filename().lexer().comment_begin2().empty())
  {
    return CheckCommentSyntax(
      get_filename().lexer().comment_begin(),
      get_filename().lexer().comment_end(), text);
  }

  comment_t comment_t1 = COMMENT_NONE;

  if (m_SyntaxType == SYNTAX_NONE || m_SyntaxType == SYNTAX_ONE)
  {
    if ((comment_t1 = CheckCommentSyntax(
      get_filename().lexer().comment_begin(),
      get_filename().lexer().comment_end(), text)) == COMMENT_BEGIN)
      m_SyntaxType = SYNTAX_ONE;
  }

  comment_t comment_t2 = COMMENT_NONE;

  if (m_SyntaxType == SYNTAX_NONE || m_SyntaxType == SYNTAX_TWO)
  {
    if ((comment_t2 = CheckCommentSyntax(
      get_filename().lexer().comment_begin2(),
      get_filename().lexer().comment_end2(), text)) == COMMENT_BEGIN)
      m_SyntaxType = SYNTAX_TWO;
  }

  comment_t comment_t = COMMENT_NONE;

  switch (comment_t1)
  {
    case COMMENT_NONE:  comment_t = comment_t2; break;
    case COMMENT_BEGIN: comment_t = COMMENT_BEGIN; break;
    case COMMENT_END:   comment_t = COMMENT_END; break;
    case COMMENT_BOTH:  comment_t = COMMENT_BOTH; break;
    case COMMENT_INCOMPLETE:
      comment_t = (comment_t2 == COMMENT_NONE) ? COMMENT_INCOMPLETE: comment_t2;
      break;
    default: assert(0);
  }

  if (comment_t == COMMENT_END)
  {
    // E.g. we have a correct /* */ comment, with */ at the end of the line.
    // Then the end of line itself should not generate a COMMENT_END.
    if (m_SyntaxType == SYNTAX_NONE) comment_t = COMMENT_NONE;
    // Keep the syntax type.
    m_LastSyntaxType = m_SyntaxType;
    m_SyntaxType = SYNTAX_NONE;
  }

  return comment_t;
}

void wex::listview_stream::CommentStatementEnd()
{
  m_IsCommentStatement = false;
}

void wex::listview_stream::CommentStatementStart()
{
  m_IsCommentStatement = true;
}

std::string wex::listview_stream::Context(
  const std::string& line, int pos) const
{
  if (pos == -1 || m_ContextSize <= 0) return line;

  return 
    (m_ContextSize > pos ? std::string(m_ContextSize - pos, ' '): std::string()) +
    line.substr(m_ContextSize < pos ? pos - m_ContextSize: 0); 
}

bool wex::listview_stream::process(std::string& line, size_t line_no)
{
  if (get_tool().id() != ID_TOOL_REPORT_KEYWORD)
  {
    return stream::process(line, line_no);
  }
  
  bool sequence = false;
  std::string codeword;

  for (size_t i = 0; i < line.length(); i++) // no auto
  {
    if (m_IsCommentStatement)
    {
    }
    else if (line[i] == '"')
    {
      m_IsString = !m_IsString;
    }

    // Comments and codewords only appear outside strings.
    if (!m_IsString)
    {
      if (line.length() == 0) continue;

      if (i == 0) 
      {
        if (!isspace(line[0]))
        {
          codeword = line[i];
        }

        continue;
      }

      const auto max_check_size = 
        get_filename().lexer().comment_begin().size();
      const auto check_size = (i > max_check_size ? max_check_size: i + 1);
      const auto text = line.substr(i + 1 - check_size, check_size);

      switch (CheckForComment(text))
      {
      case COMMENT_BEGIN:
        if (!m_IsCommentStatement) CommentStatementStart();
        break;

      case COMMENT_END:
        CommentStatementEnd();
        break;

      case COMMENT_BOTH:
        !m_IsCommentStatement ? CommentStatementStart(): CommentStatementEnd();
        break;

      case COMMENT_NONE:
        if (!isspace(line[i]) && !m_IsCommentStatement)
        {
          if (!is_codeword_separator(line[i]))
          {
            if (!sequence)
            {
              sequence = true;
            }

            codeword += line[i];
          }
        }
      break;

      case COMMENT_INCOMPLETE:
        break;

      default: 
        assert(0);
        break;
      }

      if ( sequence && 
          (is_codeword_separator(line[i]) || i ==0 || i == line.length() - 1))
      {
        if (get_tool().id() == ID_TOOL_REPORT_KEYWORD)
        {
          if (get_filename().lexer().is_keyword(codeword))
          {
            inc_statistics(codeword);
          }
        }

        sequence = false;
        codeword.clear();
      }
    }
  }

  if (CheckForComment(std::string()) == COMMENT_END)
  {
    CommentStatementEnd();
  }

  return true;
}

bool wex::listview_stream::process_begin()
{
  m_ContextSize = config(_("Context size")).get(10);

  if (get_tool().id() != ID_TOOL_REPORT_KEYWORD)
  {
    return stream::process_begin();
  }
  else
  {
    if (
      m_Frame == nullptr ||
     (m_Report = m_Frame->activate(
        history_listview::type_tool(get_tool()),
        &get_filename().lexer())) == nullptr)
    {
      return false;
    }
  }
  
  return true;
}

void wex::listview_stream::process_end()
{
  if (get_tool().id() == ID_TOOL_REPORT_KEYWORD)
  {
    if (!get_filename().lexer().keywords_string().empty())
    {
      inc_actions_completed();
    }

    listitem item(m_Report, get_filename());
    item.insert();

    int total = 0;
    int col = 1;
    
    for (const auto& setit : get_filename().lexer().keywords())
    {
      const statistics<int>& stat = get_statistics().get_elements();
      
      if (const auto& it = stat.get_items().find(setit); it != stat.get_items().end())
      {
        m_Report->SetItem(
          item.GetId(), 
          col, 
          std::to_string(it->second));
          
        total += it->second;
      }
      
      col++;
    }
    
    m_Report->SetItem(
      item.GetId(),
      col,
      std::to_string(total));
  }
}

void wex::listview_stream::process_match(
  const std::string& line, size_t line_no, int pos)
{
  assert(m_Report != nullptr);

  listitem item(m_Report, get_filename());
  item.insert();

  item.set_item(_("Line No").ToStdString(), std::to_string(line_no + 1));
  item.set_item(_("Line").ToStdString(), Context(line, pos));
  item.set_item(_("Match").ToStdString(), find_replace_data::get()->get_find_string());
}

bool wex::listview_stream::setup_tool(
  const tool& tool, 
  history_frame* frame,
  listview* report)
{
  if (tool.id() == ID_TOOL_REPLACE)
  {
    return true;
  }
  
  m_Frame = frame;

  reset();

  if (report == nullptr)
  {
    if (tool.is_report_type() && tool.id() != ID_TOOL_REPORT_KEYWORD)
    {
      if ((m_Report = 
        m_Frame->activate(history_listview::type_tool(tool))) == nullptr)
      {
        VLOG(9) << "activate failed";
        return false;
      }
    }
  }
  else
  {
    m_Report = report;
  } 

  if (m_Report != nullptr && m_Report->data().type() != listview_data::FIND)
  {
    VLOG(9) << "report list type is not listview_data::FIND";
    return false;
  }

  return true;
}
