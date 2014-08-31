////////////////////////////////////////////////////////////////////////////////
// Name:      address.cpp
// Purpose:   Implementation of class wxExAddress and wxExAddressRange
// Author:    Anton van Wezenbeek
// Copyright: (c) 2014 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <sstream>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/textfile.h>
#include <wx/tokenzr.h>
#include <wx/extension/address.h>
#include <wx/extension/ex.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/process.h>
#include <wx/extension/stc.h>
#include <wx/extension/util.h>
#include <wx/extension/vimacros.h>

#if wxUSE_GUI

wxExAddress::wxExAddress(wxExEx* ex, const wxString& address)
  : wxString(address)
  , m_Ex(ex)
  , m_Line(-1)
{
}

int wxExAddress::ToLine() const
{
  if (m_Line >= 0)
  {
    return m_Line;
  }
   
  if (Contains("/"))
  {
    return 0;
  }

  int width = 0;
  const int sum = wxExCalculator(*this, m_Ex, width);
  
  // Limit the range of what is returned.
  if (sum < 0)
  {
    return 1;
  }
  else if (sum > m_Ex->GetSTC()->GetLineCount())
  {
    return m_Ex->GetSTC()->GetLineCount();
  }  
  else
  {
    return sum;
  }
}

wxExAddressRange::wxExAddressRange(wxExEx* ex, const wxString& range)
  : m_Begin(ex, "")
  , m_End(ex, "")
  , m_Ex(ex)
  , m_STC(ex->GetSTC())
{
  if (range == ".")
  {
    Set(".", ".");
  }
  else if (range == "%")
  {
    Set("1", "$");
  }
  else if (range == "*")
  {
    std::stringstream ss;
    ss << m_STC->GetFirstVisibleLine() + 1;
    std::stringstream tt;
    tt << m_STC->GetFirstVisibleLine() + m_STC->LinesOnScreen() + 1;
    Set(wxString(ss.str()), wxString(tt.str()));
  }
  else if (range.Contains(","))
  {
    Set(range.BeforeFirst(','), range.AfterFirst(','));
  }
  else
  {
    Set(range, range);
  }
}
  
wxExAddressRange::wxExAddressRange(wxExEx* ex, int lines)
  : m_Begin(ex, "")
  , m_End(ex, "")
  , m_Ex(ex)
  , m_STC(ex->GetSTC())
{
  if (lines > 0)
  {
    m_Begin.m_Line = m_STC->LineFromPosition(m_STC->GetCurrentPos()) + 1;
    m_End.m_Line = m_Begin.m_Line + lines - 1; 
    
    if (m_End.m_Line > m_STC->GetLineCount())
    {
      m_End.m_Line = m_STC->GetLineCount();
    }
  }
  else if (lines == 0)
  {
    // this is illegal
    m_Begin.m_Line = 0;
    m_End.m_Line = 0;
  }
  else
  {
    m_End.m_Line = m_STC->LineFromPosition(m_STC->GetCurrentPos()) + 1;
    m_Begin.m_Line = m_End.m_Line + lines - 1;
    
    if (m_Begin.m_Line < 1)
    {
      m_Begin.m_Line = 1;
    }
  }
}

bool wxExAddressRange::Delete(bool show_message) const
{
  if (!IsOk() || m_STC->GetReadOnly() || m_STC->HexMode())
  {
    return false;
  }

  if (m_Ex->GetSelectedText().empty())
  {
    if (!SetSelection())
    {
      return false;
    }
  }

  const std::string sel(m_Ex->GetSelectedText());
  const int lines = wxExGetNumberOfLines(sel);

  m_STC->ReplaceSelection(wxEmptyString);
  m_Ex->SetRegistersDelete(sel);

  if (m_Begin.StartsWith("'"))
  {
    if (m_Begin.size() > 1)
    {
      m_Ex->MarkerDelete(m_Begin.GetChar(1));
    }
  }

  if (m_End.StartsWith("'"))
  {
    if (m_End.size() > 1)
    {
      m_Ex->MarkerDelete(m_End.GetChar(1));
    }
  }

  if (lines >= 3 && show_message)
  {
    m_Ex->GetFrame()->ShowExMessage(wxString::Format(_("%d fewer lines"), lines - 1));
  }

  return true;
}

bool wxExAddressRange::Filter(const wxString& command) const
{
  const int begin_line = m_Begin.ToLine();
  const int end_line = m_End.ToLine();

  if (!IsOk() || end_line < begin_line)
  {
#ifdef DEBUG
    wxLogMessage("Range error");
#endif
    return false;
  }

  const wxString filename("__TMPFILE__");
  
  wxTextFile file(filename);
  
#ifdef DEBUG
  wxLogMessage("Working dir: " + wxGetCwd());
#endif
  
  if (file.Exists() || !file.Create())
  {
#ifdef DEBUG
    wxLogMessage("File error: " + filename);
#endif
    return false;
  }

  for (int i = begin_line - 1; i <= end_line - 1; i++)
  {
    file.AddLine(m_STC->GetLine(i).Trim());
  }
  
  if (!file.Write())
  {
#ifdef DEBUG
    wxLogMessage("File write error: " + filename);
#endif
    return false;
  }
    
  wxExProcess process;
  
  const bool ok = process.Execute(command + " " + filename, wxEXEC_SYNC);
  
  remove(filename);
  
  if (ok)
  {
    if (!process.HasStdError())
    {      
      m_STC->BeginUndoAction();

      if (Delete(false))
      {
        m_STC->AddText(process.GetOutput());
        m_STC->AddText(m_STC->GetEOL());
      }
      
      m_STC->EndUndoAction();
      
      return true;
    }
    else
    {
      m_Ex->GetFrame()->ShowExMessage(process.GetOutput());
    }
  }
  
#ifdef DEBUG
    wxLogMessage("Process error: " + command);
#endif

  return false;
}

bool wxExAddressRange::Indent(bool forward) const
{
  if (!IsOk() || m_STC->GetReadOnly() || m_STC->HexMode())
  {
    return false;
  }
  
  if (m_Ex->GetSelectedText().empty())
  {
    if (!SetSelection())
    {
      return false;
    }
  }
  else
  {
    if (wxExGetNumberOfLines(m_Ex->GetSelectedText()) == 1)
    {
      // TODO: Replaces the selection.
      return false;
    }
  }

  m_STC->SendMsg(forward ? wxSTC_CMD_TAB: wxSTC_CMD_BACKTAB);
  
  return true;
}

bool wxExAddressRange::IsOk() const
{
  if (m_Begin.ToLine() == 0 || m_End.ToLine() == 0)
  {
    return false;
  }

  return true;
}

bool wxExAddressRange::Move(const wxExAddress& destination) const
{
  if (!IsOk() || m_STC->GetReadOnly() || m_STC->HexMode())
  {
    return false;
  }

  const int dest_line = destination.ToLine();

  if (
    dest_line == 0 || 
    (dest_line >= m_Begin.ToLine() && dest_line <= m_End.ToLine()))
  {
    return false;
  }

  if (m_Ex->GetSelectedText().empty())
  {
    if (!SetSelection())
    {
      return false;
    }
  }

  if (m_Begin.StartsWith("'"))
  {
    if (m_Begin.size() > 1)
    {
      m_Ex->MarkerDelete(m_Begin.GetChar(1));
    }
  }

  if (m_End.StartsWith("'"))
  {
    if (m_End.size() > 1)
    {
      m_Ex->MarkerDelete(m_End.GetChar(1));
    }
  }

  const int lines = wxExGetNumberOfLines(m_Ex->GetSelectedText());

  m_STC->BeginUndoAction();

  m_STC->ReplaceSelection(wxEmptyString);
  m_STC->GotoLine(dest_line - 1);
  m_Ex->AddText(m_Ex->GetRegisterText());

  m_STC->EndUndoAction();
  
  if (lines >= 2)
  {
    m_Ex->GetFrame()->ShowExMessage(wxString::Format(_("%d lines moved"), lines));
  }

  return true;
}

bool wxExAddressRange::Parse(
  const wxString& command_org, 
  wxString& pattern, wxString& replacement, wxString& options) const
{
  // If there are escaped / chars in the text,
  // temporarily replace them to an unused char, so
  // we can use string tokenizer with / as separator.
  bool escaped = false;
  
  wxString command(command_org);
  
  if (command.Contains("\\/"))
  {
    if (!command.Contains(wxChar(1)))
    {
      command.Replace("\\/", wxChar(1));
      escaped = true;
    }
    else
    {
      wxLogStatus("Cannot substitute, internal char exists");
      return false;
    }
  }
  
  wxStringTokenizer next(command, "/");

  if (!next.HasMoreTokens())
  {
    wxLogStatus("Cannot substitute, missing slash");
    return false;
  }

  next.GetNextToken(); // skip empty token
  pattern = next.GetNextToken();
  replacement = next.GetNextToken();
  options = next.GetNextToken();
  
  if (pattern.empty())
  {
    wxLogStatus("Cannot substitute, pattern is empty");
    return false;
  }

  if (!options.empty())
  {
    wxString filter(options);
    filter.Replace("c", "");
    filter.Replace("g", "");
    filter.Replace("i", "");
    
    if (!filter.empty())
    {
      wxLogStatus("Cannot substitute, unsupported flags: " + filter);
      return false;
    }
  }
  
  // Restore a / for all occurrences of the special char.
  if (escaped)
  {  
    pattern.Replace(wxChar(1), "/");
    replacement.Replace(wxChar(1), "/");
  }

  return true;
}
    
void wxExAddressRange::Set(const wxString& begin, const wxString& end)
{
  m_Begin.assign(begin);
  m_End.assign(end);
}

bool wxExAddressRange::SetSelection(
  int begin_line, int end_line, bool line_end_pos) const
{
  if (begin_line == 0 || end_line == 0 || end_line < begin_line)
  {
    return false;
  }

  if (line_end_pos)
  {
    m_STC->SetSelection(
      m_STC->PositionFromLine(begin_line - 1),
      m_STC->GetLineEndPosition(end_line - 1));
  }
  else 
  {
    m_STC->SetSelection(
      m_STC->PositionFromLine(begin_line - 1),
      m_STC->PositionFromLine(end_line));
  }

  const wxCharBuffer b(m_STC->GetSelectedTextRaw());
  
  if (m_Ex->GetRegister())
  {
    m_Ex->GetMacros().SetRegister(
      m_Ex->GetRegister(), m_Ex->GetSelectedText());
  }
  else
  {
    m_Ex->SetRegisterYank(std::string(b.data(), b.length() - 1));
  }

  return true;
}

bool wxExAddressRange::SetSelection(bool line_end_position) const
{
  return IsOk() && 
    SetSelection(m_Begin.ToLine(), m_End.ToLine(), line_end_position);
}

bool wxExAddressRange::Substitute(const wxString& command)
{
  if (
    m_STC->GetReadOnly() || m_STC->HexMode() ||
    // Currently this ignores rectangles, so
    // better do nothing at all.
    m_STC->SelectionIsRectangle())
  {
    return false;
  }
  
  wxString patt;
  wxString repl;
  wxString options;
    
  if (!Parse(command, patt, repl, options))
  {
    return false;
  }
    
  const int begin_line = m_Begin.ToLine();
  const int end_line = m_End.ToLine();

  if (begin_line == 0 || end_line == 0 || end_line < begin_line)
  {
    return false;
  }
  
  if (!m_Ex->MarkerAdd('$', end_line - 1))
  {
    return false;
  }

  const bool selected = !m_Ex->GetSelectedText().empty();

  wxExIndicator indicator(0, 0);

  m_STC->SetIndicatorCurrent(indicator.GetNo());
    
  const wxString pattern = (patt == "~" ? m_Replacement: patt);
  
  m_Replacement = repl; 
      
  int searchFlags = m_Ex->GetSearchFlags();
  
  if (options.Contains("i")) searchFlags &= ~wxSTC_FIND_MATCHCASE;
    
  m_STC->SetSearchFlags(searchFlags);
  m_STC->BeginUndoAction();
  m_STC->SetTargetStart(m_STC->PositionFromLine(begin_line - 1));
  m_STC->SetTargetEnd(m_STC->GetLineEndPosition(m_Ex->MarkerLine('$')));

  int nr_replacements = 0;
  int result = wxID_YES;
       
  while (m_STC->SearchInTarget(pattern) != -1 && result != wxID_CANCEL)
  {
    wxString replacement(repl);
    
    if (replacement.Contains("&"))
    {
      wxString target = m_STC->GetTextRange(
        m_STC->GetTargetStart(),
        m_STC->GetTargetEnd());
        
      if (replacement.StartsWith("\\L"))
      {
        target.MakeLower();
        replacement.Replace("\\L", wxEmptyString);
      }
      else if (replacement.StartsWith("\\U"))
      {
        target.MakeUpper();
        replacement.Replace("\\U", wxEmptyString);
      }
    
      replacement.Replace("&", target);
    }
    
    if (options.Contains("c"))
    {
      wxMessageDialog msgDialog(m_STC, 
        _("Replace") + " " + pattern + " " + _("with") + " " + replacement, 
        _("Replace"), 
        wxCANCEL | wxYES_NO);
        
      const int line = m_STC->LineFromPosition(m_STC->GetTargetStart());
      
      msgDialog.SetExtendedMessage(wxString::Format("Line %d: %s", 
        line + 1, m_STC->GetLineText(line).c_str()));
        
      m_STC->GotoLine(line);
      m_STC->EnsureVisible(line);
      m_STC->SetIndicator(
        indicator, m_STC->GetTargetStart(), m_STC->GetTargetEnd());
      
      result = msgDialog.ShowModal();
        
      if (result == wxID_YES)
      {
        m_STC->ReplaceTargetRE(replacement); // always RE!
      }
    }
    else
    {
      m_STC->ReplaceTargetRE(replacement); // always RE!
    }
        
    if (result != wxID_CANCEL)
    {
      if (options.Contains("g"))
      {
        m_STC->SetTargetStart(m_STC->GetTargetEnd());
      }
      else
      {
        m_STC->SetTargetStart(
          m_STC->GetLineEndPosition(m_STC->LineFromPosition(
            m_STC->GetTargetEnd())));
      }
  
      m_STC->SetTargetEnd(m_STC->GetLineEndPosition(m_Ex->MarkerLine('$')));
    
      if (result == wxID_YES)
      {
        nr_replacements++;
      }
    
      if (m_STC->GetTargetStart() >= m_STC->GetTargetEnd())
      {
        result = wxID_CANCEL;
      }
    }
  }


  m_STC->EndUndoAction();
  m_Ex->MarkerDelete('$');
  m_Ex->GetFrame()->ShowExMessage(wxString::Format(_("Replaced: %d occurrences of: %s"),
    nr_replacements, pattern.c_str()));

  m_STC->IndicatorClearRange(0, m_STC->GetTextLength() - 1);
  
  if (selected)
  {
    SetSelection(begin_line, end_line, true);
  }

  return true;
}

bool wxExAddressRange::Write(const wxString& filename) const
{
  if (!IsOk())
  {
    return false;
  }

  if (m_Ex->GetSelectedText().empty())
  {
    if (!SetSelection())
    {
      return false;
    }
  }

  wxFile file(filename, wxFile::write);

  return 
    file.IsOpened() && 
    file.Write(m_Ex->GetSelectedText());
}

bool wxExAddressRange::Yank() const
{
  if (!IsOk())
  {
    return false;
  }

  if (m_Ex->GetSelectedText().empty())
  {
    if (!SetSelection())
    {
      return false;
    }
  }

  const std::string range(m_Ex->GetSelectedText());
  
  if (m_Ex->GetRegister())
  {
    m_Ex->GetMacros().SetRegister(
      m_Ex->GetRegister(), range);
  }
  else
  {
    m_Ex->SetRegisterYank(range);
  }

  const int lines = wxExGetNumberOfLines(range);
  
  if (lines >= 3)
  {
    m_Ex->GetFrame()->ShowExMessage(wxString::Format(_("%d lines yanked"), lines - 1));
  }

  return true;
}

#endif // wxUSE_GUI
