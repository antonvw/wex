////////////////////////////////////////////////////////////////////////////////
// Name:      textfile.cpp
// Purpose:   Implementation of wxExTextFile class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <algorithm>
#include <cctype> // for isspace
#include <wx/extension/textfile.h>
#include <wx/extension/frd.h>
#include <wx/extension/util.h>

long wxExFileStatistics::Get(const wxString& key) const
{
#ifdef wxExUSE_CPP0X	
  const auto it = m_Elements.GetItems().find(key);
#else
  std::map<wxString, long>::const_iterator it = m_Elements.GetItems().find(key);  
#endif  

  if (it != m_Elements.GetItems().end())
  {
    return it->second;
  }
  else
  {
#ifdef wxExUSE_CPP0X	
    const auto it = m_Keywords.GetItems().find(key);
#else
    std::map<wxString, long>::const_iterator it = m_Keywords.GetItems().find(key);    
#endif  

    if (it != m_Keywords.GetItems().end())
    {
      return it->second;
    }
  }

  return 0;
}

wxExTextFile::wxExTextFile(
  const wxExFileName& filename,
  const wxExTool& tool)
  : m_FileName(filename)
  , m_LastSyntaxType(SYNTAX_NONE)
  , m_SyntaxType(SYNTAX_NONE)
  , m_Tool(tool)
  , m_IsCommentStatement(false)
  , m_IsString(false)
  , m_Modified(false)
{
}

wxExTextFile::wxExCommentType wxExTextFile::CheckCommentSyntax(
  const wxString& syntax_begin,
  const wxString& syntax_end,
  const wxString& text) const
{
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

  if ( syntax_begin.StartsWith(text) || 
      (!syntax_end.empty() && syntax_end.StartsWith(text)))
  {
    return COMMENT_INCOMPLETE;
  }

  return COMMENT_NONE;
}

wxExTextFile::wxExCommentType wxExTextFile::CheckForComment(
  const wxString& text)
{
  if (m_FileName.GetLexer().GetCommentBegin2().empty())
  {
    return CheckCommentSyntax(
      m_FileName.GetLexer().GetCommentBegin(),
      m_FileName.GetLexer().GetCommentEnd(), text);
  }

  wxExCommentType comment_type1 = COMMENT_NONE;

  if (m_SyntaxType == SYNTAX_NONE || m_SyntaxType == SYNTAX_ONE)
  {
    if ((comment_type1 = CheckCommentSyntax(
      m_FileName.GetLexer().GetCommentBegin(),
      m_FileName.GetLexer().GetCommentEnd(), text)) == COMMENT_BEGIN)
      m_SyntaxType = SYNTAX_ONE;
  }

  wxExCommentType comment_type2 = COMMENT_NONE;

  if (m_SyntaxType == SYNTAX_NONE || m_SyntaxType == SYNTAX_TWO)
  {
    if ((comment_type2 = CheckCommentSyntax(
      m_FileName.GetLexer().GetCommentBegin2(),
      m_FileName.GetLexer().GetCommentEnd2(), text)) == COMMENT_BEGIN)
      m_SyntaxType = SYNTAX_TWO;
  }

  wxExCommentType comment_type = COMMENT_NONE;

  switch (comment_type1)
  {
  case COMMENT_NONE:  comment_type = comment_type2; break;
  case COMMENT_BEGIN: comment_type = COMMENT_BEGIN; break;
  case COMMENT_END:   comment_type = COMMENT_END; break;
  case COMMENT_BOTH:  comment_type = COMMENT_BOTH; break;
  case COMMENT_INCOMPLETE:
    comment_type = (comment_type2 == COMMENT_NONE) ? COMMENT_INCOMPLETE: comment_type2;
    break;
  default: wxFAIL;
  }

  if (comment_type == COMMENT_END)
  {
    // E.g. we have a correct /* */ comment, with */ at the end of the line.
    // Then the end of line itself should not generate a COMMENT_END.
    if (m_SyntaxType == SYNTAX_NONE) comment_type = COMMENT_NONE;
    // Keep the syntax type.
    m_LastSyntaxType = m_SyntaxType;
    m_SyntaxType = SYNTAX_NONE;
  }

  return comment_type;
}

void wxExTextFile::CommentStatementEnd()
{
  m_IsCommentStatement = false;

  // Remove the end of comment characters (as last used) from the buffer.
  m_Comments = m_Comments.Left(
    m_Comments.length() - CommentEnd().length());
}

void wxExTextFile::CommentStatementStart()
{
  m_IsCommentStatement = true;
}

bool wxExTextFile::MatchLine(wxString& line)
{
  bool match = false;
  int count = 1;

  wxExFindReplaceData* frd = wxExFindReplaceData::Get();

  if (!frd->UseRegularExpression() || !frd->GetRegularExpression().IsValid())
  {
    if (m_Tool.GetId() == ID_TOOL_REPORT_FIND)
    {
      std::string search_line(line);

      if (!frd->MatchCase())
      {
        std::transform(
          search_line.begin(), 
          search_line.end(), 
          search_line.begin(), 
          toupper);
      }

      const size_t start = search_line.find(m_FindString);

      if (start != wxString::npos)
      {
        if (frd->MatchWord())
        {
          if (( start == 0 ||
               (start > 0 && !IsWordCharacter(search_line[start - 1]))) &&
              !IsWordCharacter(search_line[start + m_FindString.length()]))
          {
            match = true;
          }
        }
        else
        {
          match = true;
        }
      }
    }
    else
    {
      count = line.Replace(
        frd->GetFindString(), 
        frd->GetReplaceString());

      if (count > 0)
      {
        m_Modified = true;
        match = true;
      }
    }
  }
  else
  {
    match = frd->GetRegularExpression().Matches(line);

    if (match && m_Tool.GetId() == ID_TOOL_REPORT_REPLACE)
    {
      count = frd->GetRegularExpression().ReplaceAll(
        &line, 
        frd->GetReplaceString());

      m_Modified = true;
    }
  }

  if (match)
  {
    IncActionsCompleted(count);
  }

  return match;
}

bool wxExTextFile::Parse()
{
  if (m_Tool.GetId() == ID_TOOL_REPORT_REPLACE &&
      m_FileName.GetStat().IsReadOnly())
  {
    return false;
  }

  if (m_Tool.IsFindType())
  {
    if (wxExFindReplaceData::Get()->GetFindString().empty())
    {
      wxFAIL;
      return false;
    }

    m_FindString = wxExFindReplaceData::Get()->GetFindString();

    if (!wxExFindReplaceData::Get()->MatchCase())
    {
      std::transform(
        m_FindString.begin(), 
        m_FindString.end(), 
        m_FindString.begin(), 
        toupper);
    }
  }

  for (size_t i = 0; i < GetLineCount(); i++)
  {
    wxString& line = GetLine(i);

    if (m_Tool.IsFindType())
    {
      if (MatchLine(line))
      {
        Report(i);
      }
    }
    else
    {
      GoToLine(i);
      
      if (!ParseLine(line))
      {
        return false;
      }
    }
  }

  return true;
}

bool wxExTextFile::ParseLine(const wxString& line)
{
  bool line_contains_code = false, sequence = false;
  wxString codeword;

  for (size_t i = 0; i < line.length(); i++) // no auto
  {
    if (m_IsCommentStatement)
    {
      m_Comments += line[i];
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

      const size_t max_check_size = 
        m_FileName.GetLexer().GetCommentBegin().Length();
      const size_t check_size = (i > max_check_size ? max_check_size: i + 1);

      const wxString text = line.substr(i + 1 - check_size, check_size);

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
          line_contains_code = true;

          if (!IsCodewordSeparator(line[i]))
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
        wxFAIL;
        break;
      }

      if ( sequence && 
          (IsCodewordSeparator(line[i]) || i ==0 || i == line.length() - 1))
      {
        if (m_Tool.GetId() == ID_TOOL_REPORT_KEYWORD)
        {
          if (m_FileName.GetLexer().IsKeyword(codeword))
          {
            m_Stats.m_Keywords.Inc(codeword);
          }
        }

        sequence = false;
        codeword.clear();
      }
    }
    else
    {
      line_contains_code = true;
    }
  }

  if (CheckForComment(wxEmptyString) == COMMENT_END)
  {
    CommentStatementEnd();
  }

  return ParseComments();
}

bool wxExTextFile::RunTool()
{
  if (!wxTextFile::Open(m_FileName.GetFullPath()))
  {
    return false;
  }

  m_Stats.m_Elements.Set(_("Files"), 1);

  if (GetLineCount() > 0)
  {
    if (!Parse())
    {
      Close();

      return false;
    }
  }

  if (m_Tool.GetId() == ID_TOOL_REPORT_KEYWORD)
  {
    if (!m_FileName.GetLexer().GetKeywordsString().empty())
    {
      IncActionsCompleted();
    }

    ReportKeyword();
  }

  if (m_Modified && !m_FileName.GetStat().IsReadOnly())
  {
    if (!Write())
    {
      Close();

      return false;
    }
  }

  Close();

  return true;
}
