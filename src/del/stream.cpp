////////////////////////////////////////////////////////////////////////////////
// Name:      stream.cpp
// Purpose:   Implementation of class wex::del::stream
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <cctype> // for isspace
#include <wex/config.h>
#include <wex/core.h>
#include <wex/del/defs.h>
#include <wex/del/frame.h>
#include <wex/del/listview.h>
#include <wex/del/stream.h>
#include <wex/frd.h>
#include <wex/listitem.h>
#include <wex/log.h>

wex::del::stream::stream(const wex::path& path, const tool& tool)
  : wex::stream(find_replace_data::get(), path, tool)
{
}

wex::del::stream::~stream()
{
  if (m_queue_thread != nullptr)
  {
    if (!m_queue_thread->empty())
    {
      log("stream") << "skipped";
    }

    m_queue_thread->stop();
    delete m_queue_thread;
  }
}

wex::del::stream::comment_t wex::del::stream::check_comment_syntax(
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
    return (syntax_end == text) ? COMMENT_BOTH : COMMENT_BEGIN;
  }
  else
  {
    if (syntax_end == text || (syntax_end.empty() && text.empty()))
    {
      return COMMENT_END;
    }
  }

  if (
    syntax_begin.starts_with(text) ||
    (!syntax_end.empty() && syntax_end.starts_with(text)))
  {
    return COMMENT_INCOMPLETE;
  }

  return COMMENT_NONE;
}

wex::del::stream::comment_t
wex::del::stream::check_for_comment(const std::string& text)
{
  if (path().lexer().comment_begin2().empty())
  {
    return check_comment_syntax(
      path().lexer().comment_begin(),
      path().lexer().comment_end(),
      text);
  }

  comment_t comment_t1 = COMMENT_NONE;

  if (m_syntax_type == SYNTAX_NONE || m_syntax_type == SYNTAX_ONE)
  {
    if (
      (comment_t1 = check_comment_syntax(
         path().lexer().comment_begin(),
         path().lexer().comment_end(),
         text)) == COMMENT_BEGIN)
      m_syntax_type = SYNTAX_ONE;
  }

  comment_t comment_t2 = COMMENT_NONE;

  if (m_syntax_type == SYNTAX_NONE || m_syntax_type == SYNTAX_TWO)
  {
    if (
      (comment_t2 = check_comment_syntax(
         path().lexer().comment_begin2(),
         path().lexer().comment_end2(),
         text)) == COMMENT_BEGIN)
      m_syntax_type = SYNTAX_TWO;
  }

  comment_t comment;

  switch (comment_t1)
  {
    case COMMENT_NONE:
      comment = comment_t2;
      break;
    case COMMENT_BEGIN:
      comment = COMMENT_BEGIN;
      break;
    case COMMENT_END:
      comment = COMMENT_END;
      break;
    case COMMENT_BOTH:
      comment = COMMENT_BOTH;
      break;
    case COMMENT_INCOMPLETE:
      comment = (comment_t2 == COMMENT_NONE) ? COMMENT_INCOMPLETE : comment_t2;
      break;
    default:
      assert(0);
  }

  if (comment == COMMENT_END)
  {
    // E.g. we have a correct /* */ comment, with */ at the end of the line.
    // Then the end of line itself should not generate a COMMENT_END.
    if (m_syntax_type == SYNTAX_NONE)
      comment = COMMENT_NONE;
    // Keep the syntax type.
    m_last_syntax_type = m_syntax_type;
    m_syntax_type      = SYNTAX_NONE;
  }

  return comment;
}

void wex::del::stream::comment_statement_end()
{
  m_is_comment_statement = false;
}

void wex::del::stream::comment_statement_start()
{
  m_is_comment_statement = true;
}

bool wex::del::stream::process(std::string& line, size_t line_no)
{
  if (get_tool().id() != ID_TOOL_REPORT_KEYWORD)
  {
    return wex::stream::process(line, line_no);
  }

  bool        sequence = false;
  std::string codeword;

  for (size_t i = 0; i < line.length(); i++) // no auto
  {
    if (m_is_comment_statement)
    {
    }
    else if (line[i] == '"')
    {
      m_is_string = !m_is_string;
    }

    // Comments and codewords only appear outside strings.
    if (!m_is_string)
    {
      if (line.length() == 0)
        continue;

      if (i == 0)
      {
        if (!isspace(line[0]))
        {
          codeword = line[i];
        }

        continue;
      }

      const auto max_check_size = path().lexer().comment_begin().size();
      const auto check_size     = (i > max_check_size ? max_check_size : i + 1);
      const auto text           = line.substr(i + 1 - check_size, check_size);

      switch (check_for_comment(text))
      {
        case COMMENT_BEGIN:
          if (!m_is_comment_statement)
            comment_statement_start();
          break;

        case COMMENT_END:
          comment_statement_end();
          break;

        case COMMENT_BOTH:
          !m_is_comment_statement ? comment_statement_start() :
                                    comment_statement_end();
          break;

        case COMMENT_NONE:
          if (!isspace(line[i]) && !m_is_comment_statement)
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

      if (
        sequence &&
        (is_codeword_separator(line[i]) || i == 0 || i == line.length() - 1))
      {
        if (get_tool().id() == ID_TOOL_REPORT_KEYWORD)
        {
          if (path().lexer().is_keyword(codeword))
          {
            inc_statistics(codeword);
          }
        }

        sequence = false;
        codeword.clear();
      }
    }
  }

  if (check_for_comment(std::string()) == COMMENT_END)
  {
    comment_statement_end();
  }

  return true;
}

bool wex::del::stream::process_begin()
{
  if (m_queue_thread != nullptr)
  {
    m_queue_thread->stop();
    delete m_queue_thread;
  }

#ifdef USE_QUEUE
  try
  {
    m_queue_thread = new queue_thread<path_match>(*m_report);
    m_queue_thread->start();
  }
  catch (std::exception& e)
  {
    log(e) << "while creating the queue";
    return false;
  }
#endif

  if (get_tool().id() != ID_TOOL_REPORT_KEYWORD)
  {
    return wex::stream::process_begin();
  }
  else
  {
    if (
      m_frame == nullptr ||
      (m_report =
         m_frame->activate(listview::type_tool(get_tool()), &path().lexer())) ==
        nullptr)
    {
      return false;
    }
  }

  return true;
}

void wex::del::stream::process_end()
{
  if (m_frame != nullptr && m_frame->is_closing())
  {
    return;
  }

  if (get_tool().id() == ID_TOOL_REPORT_KEYWORD)
  {
    if (!path().lexer().keywords_string().empty())
    {
      inc_actions_completed();
    }

    listitem item(m_report, path());
    item.insert();

    int total = 0;
    int col   = 1;

    for (const auto& setit : path().lexer().keywords())
    {
      const statistics<int>& stat = get_statistics().get_elements();

      if (const auto& it = stat.get_items().find(setit);
          it != stat.get_items().end())
      {
        m_report->SetItem(item.GetId(), col, std::to_string(it->second));

        total += it->second;
      }

      col++;
    }

    m_report->SetItem(item.GetId(), col, std::to_string(total));
  }
}

void wex::del::stream::process_match(const path_match& m)
{
#ifdef USE_QUEUE
  std::unique_ptr<path_match> match(new path_match(m));
  m_queue_thread->emplace(match);
#else
  m_report->process_match(m);
#endif
}

bool wex::del::stream::setup_tool(
  const tool& tool,
  frame*      frame,
  listview*   report)
{
  if (tool.id() == ID_TOOL_REPLACE)
  {
    return true;
  }

  m_frame = frame;

  reset();

  if (report == nullptr)
  {
    if (tool.is_report_type() && tool.id() != ID_TOOL_REPORT_KEYWORD)
    {
      if ((m_report = m_frame->activate(listview::type_tool(tool))) == nullptr)
      {
        log::debug("activate failed");
        return false;
      }
    }
  }
  else
  {
    m_report = report;
  }

  if (m_report != nullptr && m_report->data().type() != data::listview::FIND)
  {
    log::debug("report list type is not data::listview::FIND");
    return false;
  }

  return true;
}
