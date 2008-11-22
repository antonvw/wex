/******************************************************************************\
* File:          textfile.cpp
* Purpose:       Implementation of exTextFile class
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2008 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <algorithm>
#include <ctype.h> // for isspace
#include <wx/stdpaths.h>
#include <wx/extension/extension.h>

const wxString REV_DATE_FORMAT = "%2y%2m%2d";

exTool exTextFile::m_Tool = ID_TOOL_LOWEST;

exTextFile::exTextFile(const exFileName& filename)
  : m_FileNameStatistics(filename)
{
  Initialize();
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
    m_Comments.length() - m_FileNameStatistics.GetLexer().CommentEndDetected().length());
}

void exTextFile::CommentStatementStart()
{
  m_IsCommentStatement = true;

  GetStatisticElements().Inc(_("Comments"));
  GetStatisticElements().Inc(
    _("Comment Size"), 
    m_FileNameStatistics.GetLexer().CommentBegin().length());
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

const wxString exTextFile::GetNextRevisionNumber()
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

bool exTextFile::HeaderDialog()
{
  const bool new_header = (m_Description.empty());
  if (new_header && m_FileNameStatistics.GetStat().IsReadOnly())
  {
    return false;
  }

  wxTextEntryDialog ted(wxTheApp->GetTopWindow(),
    _("Input") + ":",
    _("Header Description") + ": " + m_FileNameStatistics.GetFullName(),
    m_Description,
    wxOK | wxCANCEL | wxCENTRE | wxTE_MULTILINE);

  if (ted.ShowModal() == wxID_CANCEL)
  {
    return false;
  }

  if (GetLineCount() > 0 && GetCurrentLine() > 1)
  {
    for (size_t i = GetCurrentLine() - 1; i > 0; i--)
    {
      RemoveLine(i);
    }
  }

  GoToLine(0);
  m_Description = ted.GetValue();

  if (!WriteFileHeader())
  {
    return false;
  }

  if (new_header)
  {
    // By default rev 1.1 is the first revision of a file, so start with 1.0 here.
    m_RevisionNumber = "1.0";
    RevisionAddComments(wxString(
      (m_FileNameStatistics.GetStat().st_size == 0) ? _("File created and header added.") : _("Header added.")));

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

void exTextFile::Initialize()
{
  m_LineMarker = 0;
  m_AllowAction = false;
  m_DialogShown = false;
  m_EmptyLine = false;
  m_FinishedAction = false;
  m_IsCommentStatement = false;
  m_IsString = false;
  m_Modified = false;
  m_RevisionActive = false;
  m_LineMarkerEnd = 0;
  m_VersionLine = 0;
  m_RevisionFormat = REV_DATE_FORMAT;
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

bool exTextFile::MatchLine(wxString& line)
{
  bool match = false;

  exFindReplaceData* frd = exApp::GetConfig()->GetFindReplaceData();

  if (!frd->IsRegExp())
  {
    const wxString search_line = frd->MatchCase() ? line: line.Upper();
    const size_t pos = search_line.find(frd->GetFindStringNoCase());

    if (pos != wxString::npos)
    {
      if (frd->MatchWord())
      {
        if (!exIsWordCharacter(search_line[pos - 1]) &&
            !exIsWordCharacter(search_line[pos + frd->GetFindStringNoCase().length()]))
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
    m_Description = line;
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
            m_Description += (!m_Description.empty() ? " ": wxString(wxEmptyString)) + m_Comments;
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
        (m_Tool.GetId () == ID_TOOL_COMMIT) &&
          GetStatisticElements().Get(_("Lines Of Code")) > 1
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
            + exApp::GetConfig(_("Revision comment")));
          return false;
        }

        RevisionAddComments(exApp::GetConfig(_("Revision comment")));
        m_AllowAction = false;

        // We are not yet finished, there might still be RCS keywords somewhere!
        // TODO: this does not work. Run inside debugger.
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

    if (wxIsMainThread())
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
    if (exApp::GetConfig()->GetFindReplaceData()->GetFindStringNoCase().empty())
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

    if (wxIsMainThread())
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

  wxString word = exGetWord(comments, true);
  if (word == "Author")
  {
    comments.Trim();
    m_Author = comments;
  }
  else
  {
    // This word contains the description. If length not large enough, try next one.
    // Some descriptions start with * Purpose, or * Function.
    if (word.length() < 7) word = exGetWord(comments, true);
    word.MakeLower();
    if (word == "description" || word == "function" || word == "purpose" || m_AllowAction)
    {
      if (!m_AllowAction)
      {
        m_AllowAction = true;
        comments.Trim();
        m_Description = comments;
      }
      else
      {
        // This determines when the description ends.
        wxString check = m_Comments.substr(0, 7);
        check.Trim(false);
        check.Trim();
        m_Comments.Trim(false); // remove formatting
        m_Comments.Trim();
        if ((check.length() > 0 || m_Comments.empty()) && !m_Description.empty())
          m_AllowAction = false;
        else
          m_Description =
            (!m_Description.empty() ? m_Description + ' ': wxString(wxEmptyString)) + m_Comments;
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

      switch (m_FileNameStatistics.GetLexer().CheckForComment(cc, pc))
      {
      case exLexer::COMMENT_BEGIN:
        if (!m_IsCommentStatement) CommentStatementStart();
        break;

      case exLexer::COMMENT_END:
        CommentStatementEnd();
        break;

      case exLexer::COMMENT_BOTH:
        !m_IsCommentStatement ? CommentStatementStart(): CommentStatementEnd();
        break;

      case exLexer::COMMENT_NONE:
        if (cc > 0 && !isspace(cc) && !m_IsCommentStatement)
        {
          line_contains_code = true;

          if (!exIsCodewordSeparator(cc))
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

      if (sequence && (exIsCodewordSeparator(cc) || i == line.length()))
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
      exApp::GetConfigBool("RCS/RecentOnly"))
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

  // ClassBuilder lines start with '* ', these characters are skipped here.
  wxRegEx("^\\* ").ReplaceFirst(&m_Comments, wxEmptyString);
  // If there is a revision in the first word, store it.
  wxString word = exGetWord(m_Comments);
  if (word.find('.') != wxString::npos)
  {
    m_RevisionNumber = word;
  }
  else
  {
    m_RevisionNumber = wxEmptyString;
    m_Comments = word + " " + m_Comments; // put back the word!
  }

  const wxString REV_CBD_FORMAT = "%B %d, %Y %H:%M";
  const wxString REV_TIMESTAMP_FORMAT = "%2y%2m%2d %2H%2M%2S";

  const char* rest;
  if      ((rest = m_RevisionTime.ParseFormat(m_Comments, REV_TIMESTAMP_FORMAT, wxDefaultDateTime)) != NULL)
    m_RevisionFormat = REV_TIMESTAMP_FORMAT;
  else if ((rest = m_RevisionTime.ParseFormat(m_Comments, REV_DATE_FORMAT, wxDefaultDateTime)) != NULL)
    m_RevisionFormat = REV_DATE_FORMAT;
  else if ((rest = m_RevisionTime.ParseFormat(m_Comments, REV_CBD_FORMAT, wxDefaultDateTime)) != NULL)
    m_RevisionFormat = REV_CBD_FORMAT;
  else
  {
    // At this moment we support no other formats.
    return false;
  }

  // TODO: This seems like a bug.
  m_RevisionFormat.Replace("2", wxEmptyString);
  m_Comments = m_Comments.substr(m_Comments.Find(rest));
  word = exGetWord(m_Comments);

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
  m_User = word;
  m_Comments.Trim();
  m_Description = m_Comments;

  return true;
}

const wxString exTextFile::ProcessFormattedText(
  const wxString& lines,
  const wxString& header,
  bool is_comment)
{
  wxString text = lines, header_to_use = header, out;
  size_t nCharIndex;

  // ftProcess text between the carriage return line feeds.
  while ((nCharIndex = text.find("\n")) != wxString::npos)
  {
    const wxString& line = ProcessUnFormattedText(
      text.substr(0, nCharIndex),
      header_to_use,
      is_comment);
    InsertLine(line);
    out +=  line + "\n";
    text = text.substr(nCharIndex + 1);
    header_to_use = wxString(' ', header.length());
  }

  if (!text.empty())
  {
    const wxString& line = ProcessUnFormattedText(
      text,
      header_to_use,
      is_comment);
    InsertLine(line);
    out += line;
  }

  return out;
}

const wxString exTextFile::ProcessUnFormattedText(
  const wxString& lines,
  const wxString& header,
  bool is_comment)
{
  const size_t line_length = m_FileNameStatistics.GetLexer().UsableCharactersPerLine();
  // Use the header, with one space extra to separate, or no header at all.
  const wxString header_with_spaces =
    (header.length() == 0) ? wxString(wxEmptyString) : wxString(' ', header.length() + 1);
  wxString in = lines, line = header, out;

  while (!in.empty())
  {
    const wxString word = exGetWord(in, false, false);
    if (line.length() + 1 + word.length() > line_length)
    {
      const wxString& newline = (is_comment ? m_FileNameStatistics.GetLexer().FormatText(line, true, true): line);
      InsertLine(newline);
      out += newline + "\n";
      line = header_with_spaces + word;
    }
    else
    {
      line += (!line.empty() ? " ": wxString(wxEmptyString)) + word;
    }
  }

  const wxString& newline = (is_comment ? m_FileNameStatistics.GetLexer().FormatText(line, true, true): line);
  InsertLine(newline);
  out += newline;

  return out;
}

void exTextFile::Report()
{
  wxString logtext;
  int line = GetCurrentLine() + 1;

  switch (m_Tool.GetId())
  {
  case ID_TOOL_REPORT_REPLACE:
    logtext << exApp::GetConfig()->GetFindReplaceData()->GetReplaceString();
  case ID_TOOL_REPORT_FIND:
    logtext << _("Line: ") << GetDescription().Strip(wxString::both);
    logtext << _("Match: ") << exApp::GetConfig()->GetFindReplaceData()->GetFindString();
  break;
  case ID_TOOL_REPORT_REVISION:
    if (!GetRevisionNumber().empty()) logtext << GetRevisionNumber() << ' ';
    if (GetRevisionTime().IsValid()) logtext << GetRevisionTime().Format(m_RevisionFormat) << ' ';
    if (!GetUser().empty()) logtext << GetUser() << ' ';
    logtext << GetDescription();
    line--;
  break;
  default: wxLogError(FILE_INFO("Unhandled id: %d"), m_Tool.GetId());
  }

  logtext << _("Line No: ") << wxString::Format("%d", line);

  wxLogMessage(logtext);
}

void exTextFile::ReportLine(const wxString& line)
{
  wxLogMessage(line);
}

void exTextFile::ReportStatistics()
{
  switch (m_Tool.GetId())
  {
  case ID_TOOL_REPORT_HEADER: wxLogMessage(m_Description); break;
  case ID_TOOL_REPORT_KEYWORD: wxLogMessage(GetStatisticKeywords().Get()); break;
  default:
    // This used to be done for ID_TOOL_REPORT_COUNT,
    // however might be usefull for others as well.
    m_FileNameStatistics.Log();
  }
}

void exTextFile::RevisionAddComments(const wxString& comments)
{
  // TODO: This seems like a bug.
  m_RevisionFormat.Replace("2", wxEmptyString);
  WriteTextWithPrefix(comments,
    GetNextRevisionNumber() + wxDateTime::Now().Format(m_RevisionFormat) + " " +
    exApp::GetConfig("RCS/User", wxGetUserName()), !m_IsCommentStatement);
}

bool exTextFile::RevisionDialog()
{
  if (m_DialogShown)
  {
    return true;
  }

  long cancel_button = 0;
  if (!m_FileNameStatistics.GetStat().IsReadOnly())
  {
    cancel_button = wxCANCEL;
  }

  wxTextEntryDialog dlg(wxTheApp->GetTopWindow(),
    m_RevisionTime.Format(m_RevisionFormat) + "\n" + m_User,
    m_FileNameStatistics.GetFullName(),
    m_Description,
    wxOK | cancel_button | wxCENTRE | wxTE_MULTILINE);

  if (dlg.ShowModal() == wxID_CANCEL)
  {
    return false;
  }

  m_DialogShown = true;

  if (m_FileNameStatistics.GetStat().IsReadOnly())
  {
    return true;
  }

  // Replace the old revision with this revision.
  if (m_LineMarker == 0 || m_LineMarkerEnd == 0)
  {
    wxLogError("Not all markers set (%d, %d), cannot save", m_LineMarker, m_LineMarkerEnd);
    return false;
  }

  if (!m_Description.empty())
  {
    for (size_t i = m_LineMarkerEnd; i >= m_LineMarker; i--)
    {
      RemoveLine(i);
    }

    GoToLine(m_LineMarker);
    m_Description = dlg.GetValue();
    RevisionAddComments(m_Description);
  }

  return true;
}

bool exTextFile::RunTool()
{
  if (m_Tool.GetId() == ID_TOOL_LOWEST)
  {
    wxLogError("You should call SetupTool first and not use ID_TOOL_LOWEST");
    return false;
  }

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
    if (!exApp::GetConfig(_("Revision comment")).empty() &&
         m_Tool.GetId() == ID_TOOL_COMMIT)
    {
      if (!ParseForOther())
      {
        return false;
      }
    }
  }
  else if (GetLineCount() > 0)
  {
    m_FileNameStatistics.GetLexer().SetLexerFromText(GetLine(0));

    if (m_Tool.IsHeaderType())
    {
      if (ParseForHeader())
      {
        if (m_Tool.GetId() == ID_TOOL_HEADER)
        {
          if (!HeaderDialog())
          {
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
        return false;
      }
    }
  }

  if (!Cancelled())
  {
    if (m_Tool.GetId() == ID_TOOL_COMMIT)
    {
      if (!m_FileNameStatistics.GetStat().SetReadOnly(true))
      {
        return false;
      }
    }

    GetStatisticElements().Set(_("Files Passed"), 1);

    if (m_Tool.IsStatisticsType())
    {
      if (m_Tool.GetId() == ID_TOOL_REPORT_HEADER)
      {
        if (m_Description.empty())
        {
          return false;
        }

        GetStatisticElements().Inc(_("Actions Completed"));
      }
      else if (m_Tool.GetId() == ID_TOOL_REPORT_KEYWORD)
      {
        if (m_FileNameStatistics.GetLexer().GetKeywordsString().empty())
        {
          return false;
        }

        GetStatisticElements().Inc(_("Actions Completed"));
      }

      ReportStatistics();
    }
    else
    {
      if (m_Modified && !m_FileNameStatistics.GetStat().IsReadOnly())
      {
        if (!Write())
        {
          return false;
        }
      }
    }
  }

  Close();

  return true;
}

bool exTextFile::SetupTool(const exTool& tool)
{
  m_Tool = tool;

  switch (tool.GetId())
  {
  case ID_TOOL_COMMIT:
    return exCommitDialog(_("Commit"));
  break;
  default: return true;
  }
}

void exTextFile::WriteComment(
  const wxString& text,
  const bool fill_out,
  const bool fill_out_with_space)
{
  InsertLine(m_FileNameStatistics.GetLexer().FormatText(text, fill_out, fill_out_with_space));
}

bool exTextFile::WriteFileHeader()
{
  const wxString actual_author = (m_Author.empty() || m_Author.Contains("$") ?
    exApp::GetConfig("Header/Author"):
    m_Author);
  const wxString address = exApp::GetConfig("Header/Address");
  const wxString company = exApp::GetConfig("Header/Company");
  const wxString country = exApp::GetConfig("Header/Country");
  const wxString place = exApp::GetConfig("Header/Place");
  const wxString zipcode = exApp::GetConfig("Header/Zipcode");

  if (address.empty() || company.empty() || country.empty() || place.empty() || zipcode.empty())
  {
    wxLogError("One of the header components is empty");
    return false;
  }

  WriteComment(wxEmptyString, true);
  WriteComment("File:        " + m_FileNameStatistics.GetFullName(), true);
  WriteComment("Author:      " + actual_author, true);
  WriteComment("Date:        " + wxDateTime::Now().Format("%Y/%m/%d %H:%M:%S"), true);
  WriteTextWithPrefix(m_Description, "Description:");
  WriteComment(wxEmptyString, true, true);
  WriteComment(
    "Copyright (c) " + wxDateTime::Now().Format("%Y") + " " + company
    + ". All rights reserved.", true);
  WriteComment(address + ", " + zipcode + " " + place + ", " + country, true);
  WriteComment(wxEmptyString, true);

  InsertLine(wxEmptyString);

  return true;
}

void exTextFile::WriteTextWithPrefix(const wxString& text, const wxString& prefix, bool is_comment)
{
  // Normally lines contains unformatted text, however in case of a header
  // it can contain CR LF characters, these should directly invoke
  // a WriteComment.
  text.find("\n") != wxString::npos ?
    ProcessFormattedText(text, prefix, is_comment):
    ProcessUnFormattedText(text, prefix, is_comment);
}
