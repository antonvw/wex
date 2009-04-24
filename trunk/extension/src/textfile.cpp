/******************************************************************************\
* File:          textfile.cpp
* Purpose:       Implementation of exTextFile class
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <algorithm>
#include <ctype.h> // for isspace
#include <wx/stdpaths.h>
#include <wx/tokenzr.h>
#include <wx/extension/textfile.h>
#include <wx/extension/config.h>
#include <wx/extension/lexers.h>
#include <wx/extension/util.h>

const wxString REV_DATE_FORMAT = "%y%m%d";

exRCS::exRCS()
  : m_RevisionFormat(REV_DATE_FORMAT)
  // By default rev 1.1 is the first revision of a file, so start with 1.0 here.
  , m_RevisionNumber("1.0")
{
}

const wxString exRCS::GetRevision() const
{
  wxString logtext;

  if (!m_RevisionNumber.empty()) logtext << m_RevisionNumber << ' ';
  if (m_RevisionTime.IsValid()) logtext << m_RevisionTime.Format(m_RevisionFormat) << ' ';
  if (!m_User.empty()) logtext << m_User << ' ';
  logtext << m_Description;

  return logtext;
}

const wxString exRCS::SetNextRevisionNumber()
{
  // If the m_RevisionNumber member is valid, and this is a new comment (check-in or out),
  // then the revision can be incremented. Always increment the rightmost part.
  // E.g.
  // 1.1   -> 1.2
  // 1.2.3 -> 1.2.4
  if (!m_RevisionNumber.empty())
  {
    const int pos = m_RevisionNumber.rfind('.');

    const wxString leftpart = m_RevisionNumber.substr(0, pos + 1);
    const wxString rightpart = m_RevisionNumber.substr(pos + 1);
    const int new_number = atoi(rightpart.c_str()) + 1;

    m_RevisionNumber = leftpart + wxString::Format("%d", new_number);
    m_RevisionNumber.Append(' ', 6 - m_RevisionNumber.length());
  }

  return m_RevisionNumber;
}

bool exRCS::SetRevision(wxString& text)
{
  // ClassBuilder lines start with '* ', these characters are skipped here.
  wxRegEx("^\\* ").ReplaceFirst(&text, wxEmptyString);
  // If there is a revision in the first word, store it.
  wxString word = exGetWord(text);
  if (word.find('.') != wxString::npos)
  {
    m_RevisionNumber = word;
  }
  else
  {
    m_RevisionNumber = wxEmptyString;
    text = word + " " + text; // put back the word!
  }

  const wxString REV_CBD_FORMAT = "%B %d, %Y %H:%M";
  const wxString REV_TIMESTAMP_FORMAT = "%y%m%d %H%M%S";

  wxString::const_iterator end;

  if      (m_RevisionTime.ParseFormat(text, REV_TIMESTAMP_FORMAT, &end))
    m_RevisionFormat = REV_TIMESTAMP_FORMAT;
  else if (m_RevisionTime.ParseFormat(text, REV_DATE_FORMAT, &end))
    m_RevisionFormat = REV_DATE_FORMAT;
  else if (m_RevisionTime.ParseFormat(text, REV_CBD_FORMAT, &end))
    m_RevisionFormat = REV_CBD_FORMAT;
  else
  {
    // At this moment we support no other formats.
    return false;
  }

  text = wxString(end, text.end());
  word = exGetWord(text);
  m_User = word;
  text.Trim();
  m_Description = text;

  return true;
}

exTextFile::exTextFile(
  const exFileName& filename,
  exConfig* config,
  const exLexers* lexers)
  : m_FileNameStatistics(filename)
  , m_LastSyntaxType(SYNTAX_NONE)
  , m_SyntaxType(SYNTAX_NONE)
  , m_Tool(ID_TOOL_LOWEST)
  , m_Config(config)
  , m_Lexers(lexers)
{
  m_AllowAction = false;
  m_EmptyLine = false;
  m_FinishedAction = false;
  m_IsCommentStatement = false;
  m_IsString = false;
  m_Modified = false;
  m_RevisionActive = false;
  m_LineMarker = 0;
  m_LineMarkerEnd = 0;
  m_VersionLine = 0;
}

exTextFile::exCommentType exTextFile::CheckCommentSyntax(
  const wxString& syntax_begin,
  const wxString& syntax_end,
  wxChar c1,
  wxChar c2) const
{
  const wxString comp = ((syntax_begin.length() == 1) ? wxString(c1) : wxString(c2) + wxString(c1));

  if (syntax_begin == comp)
  {
    return (syntax_end == comp) ? COMMENT_BOTH: COMMENT_BEGIN;
  }
  else
  {
    if (syntax_end == comp ||
        // If syntax_end was empty, we assume the terminating 0 ends the comment.
       (syntax_end.empty() && c1 == 0))
    {
      return COMMENT_END;
    }
  }

  if ((syntax_begin.length() > 1 && syntax_begin[0] == c1) ||
      (syntax_end.length() > 1 && syntax_end[0] == c1) ||
      (c1 == 0))
  {
    return COMMENT_INCOMPLETE;
  }

  return COMMENT_NONE;
}

exTextFile::exCommentType exTextFile::CheckForComment(wxChar c1, wxChar c2)
{
  if (m_FileNameStatistics.GetLexer().GetCommentBegin2().empty())
  {
    return CheckCommentSyntax(
      m_FileNameStatistics.GetLexer().GetCommentBegin(),
      m_FileNameStatistics.GetLexer().GetCommentEnd(), c1, c2);
  }

  exCommentType comment_type1 = COMMENT_NONE;

  if (m_SyntaxType == SYNTAX_NONE || m_SyntaxType == SYNTAX_ONE)
  {
    if ((comment_type1 = CheckCommentSyntax(
      m_FileNameStatistics.GetLexer().GetCommentBegin(),
      m_FileNameStatistics.GetLexer().GetCommentEnd(), c1, c2)) == COMMENT_BEGIN)
      m_SyntaxType = SYNTAX_ONE;
  }

  exCommentType comment_type2 = COMMENT_NONE;

  if (m_SyntaxType == SYNTAX_NONE || m_SyntaxType == SYNTAX_TWO)
  {
    if ((comment_type2 = CheckCommentSyntax(
      m_FileNameStatistics.GetLexer().GetCommentBegin2(),
      m_FileNameStatistics.GetLexer().GetCommentEnd2(), c1, c2)) == COMMENT_BEGIN)
      m_SyntaxType = SYNTAX_TWO;
  }

  exCommentType comment_type = COMMENT_NONE;

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

void exTextFile::CommentStatementEnd()
{
  m_IsCommentStatement = false;

  // Remove the end of comment characters from the buffer.
  // The special function CommentEndDetected is used here,
  // as we might already have parsed a new line, and syntax type is
  // already reset, whereas in the buffer the really used end
  // of comment characters should be removed.
  m_Comments = m_Comments.Left(
    m_Comments.length() - CommentEndDetected().length());
}

void exTextFile::CommentStatementStart()
{
  m_IsCommentStatement = true;

  GetStatisticElements().Inc(_("Comments"));
  GetStatisticElements().Inc(
    _("Comment Size"),
    CommentBegin().length());
}

void exTextFile::EndCurrentRevision()
{
  if (m_RevisionActive)
  {
    GetStatisticElements().Inc(_("Actions Completed"));

    if (m_Tool.GetId() == ID_TOOL_REPORT_REVISION)
    {
      Report();
    }

    m_RevisionActive = false;
  }
}

void exTextFile::InsertLine(const wxString& line)
{
  if (GetCurrentLine() == GetLineCount())
  {
    AddLine(line);
  }
  else
  {
    wxTextFile::InsertLine(line, GetCurrentLine());
  }

  m_Modified = true;

  GoToLine(GetCurrentLine() + 1);
}

bool exTextFile::IsBrace(int c) const
{
  return c == '[' || c == ']' ||
         c == '(' || c == ')' ||
         c == '{' || c == '}' ||
         c == '<' || c == '>';
};

bool exTextFile::IsCodewordSeparator(int c) const
{
  return (isspace(c) || IsBrace(c) || c == ',' || c == ';' || c == ':');
}

bool exTextFile::IsWordCharacter(int c) const
{
  return isalnum(c) || c == '_';
}

bool exTextFile::MatchLine(wxString& line)
{
  bool match = false;

  exFindReplaceData* frd = m_Config->GetFindReplaceData();

  if (!frd->IsRegularExpression())
  {
    const wxString search_line = frd->MatchCase() ? line: line.Upper();
    const size_t pos = search_line.find(frd->GetFindStringNoCase());

    if (pos != wxString::npos)
    {
      if (frd->MatchWord())
      {
        if (( pos == 0 ||
             (pos > 0 && !IsWordCharacter(search_line[pos - 1]))) &&
            !IsWordCharacter(search_line[pos + frd->GetFindStringNoCase().length()]))
        {
          match = true;
        }
      }
      else
      {
        match = true;
      }

      if (match && m_Tool.GetId() == ID_TOOL_REPORT_REPLACE)
      {
        line.replace(pos, frd->GetReplaceString().length(), frd->GetReplaceString());
        m_Modified = true;
      }
    }
  }
  else
  {
    match = frd->GetRegularExpression().Matches(line);

    if (match && m_Tool.GetId() == ID_TOOL_REPORT_REPLACE)
    {
      size_t start, len;
      frd->GetRegularExpression().GetMatch(&start, &len);
      line.replace(start, len, frd->GetReplaceString());
      m_Modified = true;
    }
  }

  if (match)
  {
    m_RCS.m_Description = line;
  }

  return match;
}

bool exTextFile::Parse()
{
  if (m_Tool.GetId() == ID_TOOL_REPORT_REPLACE)
  {
    if (m_FileNameStatistics.GetStat().IsReadOnly())
    {
      return false;
    }
  }

  if (m_Tool.IsFindType())
  {
    if (m_Config->GetFindReplaceData()->GetFindStringNoCase().empty())
    {
      wxFAIL;
      return false;
    }
  }

  GoToLine(0); // When using GetFirstLine, etc. replae in files does not work.

  while (!Cancelled() && !m_FinishedAction)
  {
    wxString& line = GetLine(GetCurrentLine());

    if (m_Tool.IsFindType())
    {
      if (MatchLine(line))
      {
        GetStatisticElements().Inc(_("Actions Completed"));
        Report();
      }
    }
    else
    {
      if (!ParseLine(line))
      {
        return false;
      }
    }

    if (wxIsMainThread() && wxTheApp != NULL)
    {
      wxTheApp->Yield();
    }
    else
    {
      wxThread::This()->Yield();
    }

    if (Eof()) break;
    if (GetCurrentLine() == GetLineCount() - 1) break;
    else GoToLine(GetCurrentLine() + 1);
  }

  return true;
}

bool exTextFile::ParseComments()
{
  if (m_Tool.IsRCSType())
  {
    if (m_Tool.GetId() == ID_TOOL_REPORT_REVISION || m_VersionLine <= 1)
    {
      // this is the minimal prefix
      if (m_Comments.length() >= 3)
      {
        const bool insert = (wxRegEx("^[\\*| ]   *").ReplaceFirst(&m_Comments, wxEmptyString) == 0);
        m_Comments.Trim(); // from right, default

        if (insert)
        {
          EndCurrentRevision();

          if (!PrepareRevision())
          {
            m_Comments.clear();
            return true;
          }

          m_RevisionActive = true;
        }
        else
        {
          if (m_VersionLine >= 1 && GetCurrentLine() == m_LineMarkerEnd + 1)
          {
            m_LineMarkerEnd = GetCurrentLine();
            m_RCS.m_Description += (!m_RCS.m_Description.empty() ? " ": wxString(wxEmptyString)) + m_Comments;
          }
          else
          {
            m_Comments.clear();
            return true;
          }
        }
      }
    }

    if (  m_AllowAction ||
         // In case there is one revision comment, this is ended by an empty line.
        (m_Tool.GetId() == ID_TOOL_REVISION_RECENT &&
         !m_FinishedAction && m_VersionLine == 1 && m_EmptyLine) ||

         // In case commiting, if we have come at the code, there
         // was not a revision, so create a new one.
        (m_Tool.GetId () == ID_TOOL_COMMIT &&
          GetStatisticElements().Get(_("Lines Of Code")) > 1)
       )
    {
      if (m_Tool.GetId() == ID_TOOL_REVISION_RECENT)
      {
        m_AllowAction = false;
        // We are not yet finished, there might still be RCS keywords somewhere!
        // m_FinishedAction = true;
      }

      if (m_Tool.GetId() == ID_TOOL_COMMIT)
      {
        if (m_LineMarker > 1)
        {
          GoToLine(m_LineMarker);
        }
        else
        {
          wxLogError("Line marker not set, cannot add revision comments: "
            + m_Config->Get(_("Revision comment")));
          return false;
        }

        RevisionAddComments(m_Config->Get(_("Revision comment")));
        m_AllowAction = false;

        // We are not yet finished, there might still be RCS keywords somewhere!
        m_FinishedAction = true;
      }
    }

    m_Comments.clear();
  }

  return true;
}

bool exTextFile::ParseLine(const wxString& line)
{
  bool line_contains_code = false, sequence = false;
  wxString codeword;

  for (size_t i = 0; i <= line.length(); i++)
  {
    if (m_IsCommentStatement)
    {
      if (i < line.length())
      {
        if (m_Tool.IsCountType())
        {
          GetStatisticElements().Inc(_("Comment Size"));
        }

        if (m_Tool.GetId() >= ID_TOOL_COMMIT)
        {
          m_Comments += line[i];
        }
      }
    }
    else if (i < line.length() && line[i] == '"')
    {
      m_IsString = !m_IsString;
    }

    // Comments and codewords only appear outside strings.
    if (!m_IsString)
    {
      if (line.length() == 0) continue;

      wxChar cc = line[i];
      wxChar pc = 0;
      if (i > 0) pc = line[i - 1];

      switch (CheckForComment(cc, pc))
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
        if (cc > 0 && !isspace(cc) && !m_IsCommentStatement)
        {
          line_contains_code = true;

          if (!IsCodewordSeparator(cc))
          {
            if (!sequence)
            {
              if (m_Tool.IsCountType())
              {
                GetStatisticElements().Inc(_("Words Of Code"));
              }

              sequence = true;
            }

            codeword += cc;
          }
        }
      break;

      default: break;
      }

      if (sequence && (IsCodewordSeparator(cc) || i == line.length()))
      {
        if (m_Tool.GetId() == ID_TOOL_REPORT_KEYWORD)
        {
          if (m_FileNameStatistics.GetLexer().IsKeyword(codeword))
          {
            GetStatisticKeywords().Inc(codeword);
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

  if (line_contains_code)
  {
    GetStatisticElements().Inc(_("Lines Of Code"));

    if (GetStatisticElements().Get(_("Lines Of Code")) == 1 &&
        m_Tool.GetId() == ID_TOOL_COMMIT)
    {
      if (m_LineMarker == 0)
      {
        m_LineMarker = GetCurrentLine();
      }
    }

    if (m_Tool.GetId() == ID_TOOL_LINE || m_Tool.GetId() == ID_TOOL_LINE_CODE)
    {
      ReportLine(line);
    }

    // Finish action here for REVISION_TOOL_REPORT.
    // However, some sources might contain revisions at the end of the file, these are not reported.
    if (GetStatisticElements().Get(_("Lines Of Code")) > 5 &&
        m_Tool.GetId() == ID_TOOL_REPORT_REVISION)
    {
      m_FinishedAction = true;
    }
  }

  m_EmptyLine = (line.length() == 0);

  if (m_EmptyLine)
  {
    if (m_Tool.GetId() == ID_TOOL_LINE && !m_IsCommentStatement)
    {
      ReportLine(line);
    }

    GetStatisticElements().Inc(_("Empty Lines"));
  }

  if (m_IsCommentStatement && GetCurrentLine() < GetLineCount() - 1)
  {
    if (m_Tool.GetId() == ID_TOOL_LINE_COMMENT)
    {
      ReportLine(line);
    }

    // End of lines are included in comment size as well.
    if (m_Tool.IsCountType())
    {
      GetStatisticElements().Inc(_("Comment Size"), wxString(GetEOL()).length());
    }
  }

  return ParseComments();
}

bool exTextFile::PrepareRevision()
{
  if (m_Tool.GetId() == ID_TOOL_REVISION_RECENT ||
      m_Config->GetBool("RCS/RecentOnly"))
  {
    if (m_VersionLine == 1)
    {
      if (!m_FinishedAction)
      {
        m_AllowAction = true;
      }

      return false;
    }
  }

  if (!m_RCS.SetRevision(m_Comments))
  {
    return false;
  }

  if (m_LineMarker == 0)
  {
    m_LineMarker = GetCurrentLine();
    m_LineMarkerEnd = GetCurrentLine();
  }

  if (m_Tool.GetId() == ID_TOOL_REPORT_REVISION)
  {
    m_LineMarkerEnd = GetCurrentLine();
  }

  if (m_Tool.GetId() != ID_TOOL_REVISION_RECENT)
  {
    m_AllowAction = true;
  }

  m_VersionLine++;

  return true;
}

void exTextFile::RevisionAddComments(const wxString& comments)
{
  InsertLine(m_FileNameStatistics.GetLexer().MakeComment(
    m_RCS.SetNextRevisionNumber() + wxDateTime::Now().Format(m_RCS.m_RevisionFormat) + " " +
      m_Config->Get("RCS/User", wxGetUserName()),
    comments));
}

bool exTextFile::RunTool(const exTool& tool)
{
  m_Tool = tool;

  if (!wxTextFile::Open(m_FileNameStatistics.GetFullPath()))
  {
    return false;
  }

  GetStatisticElements().Set(_("Files"), 1);

  if (m_Tool.IsCountType())
  {
    GetStatisticElements().Inc(_("Total Size"), m_FileNameStatistics.GetStat().st_size);
    GetStatisticElements().Inc(_("Lines"), GetLineCount());
  }

  if (m_Tool.GetId() == ID_TOOL_COMMIT)
  {
    if (!m_Config->Get(_("Revision comment")).empty())
    {
      if (!Parse())
      {
        Close();

        return false;
      }
    }
  }
  else if (GetLineCount() > 0)
  {
    if (m_FileNameStatistics.GetLexer().GetScintillaLexer().empty())
    {
      m_FileNameStatistics.GetLexer() = m_Lexers->FindByText(GetLine(0));
    }

    if (!Parse())
    {
      Close();

      return false;
    }
  }

  if (!Cancelled())
  {
    GetStatisticElements().Set(_("Files Passed"), 1);

    if (m_Tool.IsStatisticsType())
    {
      if (m_Tool.GetId() == ID_TOOL_REPORT_KEYWORD)
      {
        if (!m_FileNameStatistics.GetLexer().GetKeywordsString().empty())
        {
          GetStatisticElements().Inc(_("Actions Completed"));
        }

      }

      ReportStatistics();
    }

    if (m_Modified && !m_FileNameStatistics.GetStat().IsReadOnly())
    {
      if (!Write())
      {
        Close();

        return false;
      }
    }
  }

  Close();

  return true;
}

void exTextFile::WriteComment(
  const wxString& text,
  const bool fill_out,
  const bool fill_out_with_space)
{
  InsertLine(m_FileNameStatistics.GetLexer().MakeComment(
    text, 
    fill_out, 
    fill_out_with_space));
}
