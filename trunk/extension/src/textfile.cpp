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
#include <wx/extension/config.h>
#include <wx/extension/lexers.h>
#include <wx/extension/textfile.h>

/// Gets a word from a string.
const wxString GetWord(
  wxString& text,
  bool use_other_field_separators = false,
  bool use_path_separator = false)
{
  wxString field_separators = " \t";
  if (use_other_field_separators) field_separators += ":";
  if (use_path_separator) field_separators = wxFILE_SEP_PATH;
  wxString token;
  wxStringTokenizer tkz(text, field_separators);
  if (tkz.HasMoreTokens()) token = tkz.GetNextToken();
  text = tkz.GetString();
  text.Trim(false);
  return token;
}

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
  wxString word = GetWord(text);
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
  word = GetWord(text);
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
  default: wxLogError(FILE_INFO("Unhandled"));
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

bool exTextFile::HeaderDialog()
{
  if (wxTheApp == NULL) return false;

  const bool new_header = (m_RCS.m_Description.empty());

  wxTextEntryDialog ted(wxTheApp->GetTopWindow(),
    _("Input") + ":",
    _("Header Description") + ": " + m_FileNameStatistics.GetFullName(),
    m_RCS.m_Description,
    wxOK | wxCANCEL | wxCENTRE | wxTE_MULTILINE);

  if (ted.ShowModal() == wxID_CANCEL)
  {
    return false;
  }

  m_RCS.m_Description = ted.GetValue();

  if (GetLineCount() > 0 && GetCurrentLine() > 1)
  {
    for (size_t i = GetCurrentLine() - 1; i > 0; i--)
    {
      RemoveLine(i);
    }
  }

  GoToLine(0);

  if (!WriteFileHeader())
  {
    return false;
  }

  if (new_header)
  {
    if (!m_Config->GetBool("SVN"))
    {
      RevisionAddComments(wxString(
        (m_FileNameStatistics.GetStat().st_size == 0) ? _("File created and header added.") : _("Header added.")));
    }

    if (m_FileNameStatistics.GetExt() == "h" && m_FileNameStatistics.GetStat().st_size == 0)
    {
      wxString argument = "__" + m_FileNameStatistics.GetName() + "_h";

      InsertLine(wxEmptyString);
      InsertLine("#if !defined (" + argument + ")");
      InsertLine("#define " + argument);
      InsertLine("#endif");
    }
  }

  return true;
}

void exTextFile::InsertFormattedText(
  const wxString& lines,
  const wxString& header,
  bool is_comment)
{
  wxString text = lines, header_to_use = header;
  size_t nCharIndex;

  // Process text between the carriage return line feeds.
  while ((nCharIndex = text.find("\n")) != wxString::npos)
  {
    InsertUnFormattedText(
      text.substr(0, nCharIndex),
      header_to_use,
      is_comment);
    text = text.substr(nCharIndex + 1);
    header_to_use = wxString(' ', header.length());
  }

  if (!text.empty())
  {
    InsertUnFormattedText(
      text,
      header_to_use,
      is_comment);
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

void exTextFile::InsertUnFormattedText(
  const wxString& lines,
  const wxString& header,
  bool is_comment)
{
  const size_t line_length = m_FileNameStatistics.GetLexer().UsableCharactersPerLine();

  // Use the header, with one space extra to separate, or no header at all.
  const wxString header_with_spaces =
    (header.length() == 0) ? wxString(wxEmptyString) : wxString(' ', header.length());

  wxString in = lines, line = header;

  bool at_begin = true;

  while (!in.empty())
  {
    const wxString word = GetWord(in, false, false);

    if (line.length() + 1 + word.length() > line_length)
    {
      const wxString& newline = 
        (is_comment ? m_FileNameStatistics.GetLexer().MakeComment(line, true, true): line);

      InsertLine(newline);
      line = header_with_spaces + word;
      at_begin = true;
    }
    else
    {
      line += (!line.empty() && !at_begin ? " ": wxString(wxEmptyString)) + word;
      at_begin = false;
    }
  }

  const wxString& newline = 
    (is_comment ? m_FileNameStatistics.GetLexer().MakeComment(line, true, true): line);

  InsertLine(newline);
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

  if (!frd->IsRegExp())
  {
    const wxString search_line = frd->MatchCase() ? line: line.Upper();
    const size_t pos = search_line.find(frd->GetFindStringNoCase());

    if (pos != wxString::npos)
    {
      if (frd->MatchWord())
      {
        if (!IsWordCharacter(search_line[pos - 1]) &&
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
    match = frd->GetFindRegularExpression().Matches(line);

    if (match && m_Tool.GetId() == ID_TOOL_REPORT_REPLACE)
    {
      size_t start, len;
      frd->GetFindRegularExpression().GetMatch(&start, &len);
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

bool exTextFile::ParseComments()
{
  if (m_Tool.IsHeaderType())
  {
    if (!m_FinishedAction)
    {
      if (!m_IsCommentStatement && m_EmptyLine)
      {
        m_FinishedAction = true;
      }
      else
      {
        ParseHeader();
      }
    }

    m_Comments.clear();
  }
  else if (m_Tool.IsRCSType())
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

bool exTextFile::ParseForHeader()
{
  for (
    wxString& line = GetFirstLine();
    !Cancelled() && !m_FinishedAction;
    line = GetNextLine())
  {
    if (!ParseLine(line))
    {
      return false;
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
  }

  return true;
}

bool exTextFile::ParseForOther()
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
      wxLogError("Find string is empty");
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

void exTextFile::ParseHeader()
{
  wxString comments(m_Comments);
  comments.Trim(false);

  wxString word = GetWord(comments, true);
  if (word == "Author")
  {
    comments.Trim();
    m_RCS.m_Author = comments;
  }
  else
  {
    // This word contains the description. If length not large enough, try next one.
    // Some descriptions start with * Purpose, or * Function.
    if (word.length() < 7) word = GetWord(comments, true);
    word.MakeLower();
    if (word == "description" || word == "function" || word == "purpose" || m_AllowAction)
    {
      if (!m_AllowAction)
      {
        m_AllowAction = true;
        comments.Trim();
        m_RCS.m_Description = comments;
      }
      else
      {
        // This determines when the description ends.
        wxString check = m_Comments.substr(0, 7);
        check.Trim(false);
        check.Trim();
        m_Comments.Trim(false); // remove formatting
        m_Comments.Trim();
        if ((check.length() > 0 || m_Comments.empty()) && !m_RCS.m_Description.empty())
          m_AllowAction = false;
        else
          m_RCS.m_Description =
            (!m_RCS.m_Description.empty() ? m_RCS.m_Description + ' ': wxString(wxEmptyString)) + m_Comments;
      }
    }
  }
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

    if (m_Tool.IsHeaderType())
    {
      m_FinishedAction = true;
    }

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
  WriteTextWithPrefix(comments,
    m_RCS.SetNextRevisionNumber() + wxDateTime::Now().Format(m_RCS.m_RevisionFormat) + " " +
    m_Config->Get("RCS/User", wxGetUserName()), !m_IsCommentStatement);
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
      if (!ParseForOther())
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

    if (m_Tool.IsHeaderType())
    {
      if (ParseForHeader())
      {
        if (m_Tool.GetId() == ID_TOOL_HEADER)
        {
          if (!HeaderDialog())
          {
            Close();

            return false;
          }
        }
      }
      else
      {
        return false;
      }
    }
    else
    {
      if (!ParseForOther())
      {
        Close();

        return false;
      }
    }
  }

  if (!Cancelled())
  {
    GetStatisticElements().Set(_("Files Passed"), 1);

    if (m_Tool.IsStatisticsType())
    {
      if (m_Tool.GetId() == ID_TOOL_REPORT_HEADER)
      {
        if (!m_RCS.m_Description.empty())
        {
          GetStatisticElements().Inc(_("Actions Completed"));
        }

      }
      else if (m_Tool.GetId() == ID_TOOL_REPORT_KEYWORD)
      {
        if (!m_FileNameStatistics.GetLexer().GetKeywordsString().empty())
        {
          GetStatisticElements().Inc(_("Actions Completed"));
        }

      }

      ReportStatistics();
    }
    else
    {
      if (m_Modified && !m_FileNameStatistics.GetStat().IsReadOnly())
      {
        if (!Write())
        {
          Close();

          return false;
        }
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
  InsertLine(m_FileNameStatistics.GetLexer().MakeComment(text, fill_out, fill_out_with_space));
}

bool exTextFile::WriteFileHeader()
{
  const wxString actual_author = (m_RCS.m_Author.empty() ?
    m_Config->Get("Header/Author"):
    m_RCS.m_Author);

  const wxString address = m_Config->Get("Header/Address");
  const wxString company = m_Config->Get("Header/Company");
  const wxString country = m_Config->Get("Header/Country");
  const wxString place = m_Config->Get("Header/Place");
  const wxString zipcode = m_Config->Get("Header/Zipcode");

  WriteComment(wxEmptyString, true);
  WriteComment("File:        " + m_FileNameStatistics.GetFullName(), true);
  WriteTextWithPrefix(m_RCS.m_Description, "Purpose:     ");
  WriteComment("Author:      " + actual_author, true);
  WriteComment("Created:     " + wxDateTime::Now().Format("%Y/%m/%d %H:%M:%S"), true);

  if (m_Config->GetBool("SVN"))
  {
    // Prevent the Id to be expanded by SVN itself here.
    WriteComment("RCS-ID:      $" + wxString("Id$"), true);
  }

  WriteComment(wxEmptyString, true, true);
  WriteComment(
    "Copyright (c) " + wxDateTime::Now().Format("%Y") + (!company.empty() ? " " + company: wxString(wxEmptyString))
    + ". All rights reserved.", true);

  if (!address.empty() && !country.empty() && !place.empty() && !zipcode.empty())
  {
    WriteComment(address + ", " + zipcode + " " + place + ", " + country, true);
  }

  WriteComment(wxEmptyString, true);

  InsertLine(wxEmptyString);

  return true;
}
