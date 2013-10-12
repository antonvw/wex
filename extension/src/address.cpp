////////////////////////////////////////////////////////////////////////////////
// Name:      address.cpp
// Purpose:   Implementation of class wxExAddress and wxExAddressRange
// Author:    Anton van Wezenbeek
// Copyright: (c) 2013 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

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
  , m_Pos(-1)
{
}

// Cannot be const because of Replace.
bool wxExAddress::Substitute(
 wxString& pattern, wxString& replacement, wxString& options)
{
  // If there are escaped / chars in the text,
  // temporarily replace them to an unused char, so
  // we can use string tokenizer with / as separator.
  bool escaped = false;
  
  if (Contains("\\/"))
  {
    if (!Contains(wxChar(1)))
    {
      Replace("\\/", wxChar(1));
      escaped = true;
    }
    else
    {
      wxLogStatus("Cannot substitute, internal char exists");
      return false;
    }
  }
  
  wxStringTokenizer next(*this, "/");

  if (!next.HasMoreTokens())
  {
    return false;
  }

  next.GetNextToken(); // skip empty token
  pattern = next.GetNextToken();
  replacement = next.GetNextToken();
  options = next.GetNextToken();
  
  // Restore a / for all occurrences of the special char.
  if (escaped)
  {  
    pattern.Replace(wxChar(1), "/");
    replacement.Replace(wxChar(1), "/");
  }

  return true;
}
    
int wxExAddress::ToLine() const
{
  if (m_Line >= 0)
  {
    return m_Line;
  }
  
  wxString filtered_address(wxExSkipWhiteSpace(*this, ""));
  
  if (filtered_address.empty())
  {
    return 0;
  }

  // Filter all markers.
  int markers = 0;

  while (filtered_address.Contains("'"))
  {
    const wxString oper = filtered_address.BeforeFirst('\'');
    
    int pos = filtered_address.Find('\'');
    int size = 2;
    
    const wxString marker = filtered_address.AfterFirst('\'');
    
    if (marker.empty())
    {
      return 0;
    }
    
    const int line = m_Ex->MarkerLine(marker.GetChar(0)) + 1;
    
    if (line == 0)
    {
      return 0;
    }

    if (oper == "-")
    {
      markers -= line;
      pos--;
      size++;
    }
    else if (oper == "+")
    {
      markers += line;
      pos--;
      size++;
    }
    else 
    {
      markers += line;
    }

    filtered_address.replace(pos, size, "");
  }

  int dot = 0;
  int stc_used = 0;

  if (filtered_address.Contains("."))
  {
    dot = m_Ex->GetSTC()->GetCurrentLine();
    filtered_address.Replace(".", "");
    stc_used = 1;
  }

  // Filter $.
  int dollar = 0;

  if (filtered_address.Contains("$"))
  {
    dollar = m_Ex->GetSTC()->GetLineCount();
    filtered_address.Replace("$", "");
    stc_used = 1;
  }

  // Now we should have a number.
  if (!filtered_address.IsNumber()) 
  {
    return 0;
  }

  // Convert this number.
  int i = 0;
  
  if (
  !filtered_address.empty() &&
   filtered_address != "-0" && 
   filtered_address != "+0")
  {
    if ((i = atoi(filtered_address.c_str())) == 0)
    {
      return 0;
    }
  }
  
  // Calculate the line.
  const int line_no = markers + dot + dollar + i + stc_used;
  
  // Limit the range of what is returned.
  if (line_no <= 0)
  {
    return 1;
  }
  else if (line_no > m_Ex->GetSTC()->GetLineCount())
  {
    return m_Ex->GetSTC()->GetLineCount();
  }  
  else
  {
    return line_no;
  }
}

wxExAddressRange::wxExAddressRange(wxExEx* ex)
  : m_Begin(ex)
  , m_End(ex)
  , m_Ex(ex)
  , m_STC(ex->GetSTC())
{
  if (!m_STC->GetSelectedText().empty())
  {
    m_STC->GetSelection(&m_Begin.m_Pos, &m_End.m_Pos);
  }
}

wxExAddressRange::wxExAddressRange(wxExEx* ex, int lines)
  : m_Begin(ex)
  , m_End(ex)
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

wxExAddressRange::wxExAddressRange(wxExEx* ex, 
  const wxString& begin, const wxString& end)
  : m_Begin(ex, begin)
  , m_End(ex, end)
  , m_Ex(ex)
  , m_STC(ex->GetSTC())
{
}

bool wxExAddressRange::Delete() const
{
  if (!IsOk() || m_STC->GetReadOnly() || m_STC->HexMode())
  {
    return false;
  }

  if (m_STC->GetSelectedText().empty())
  {
    if (!SetSelection())
    {
      return false;
    }
  }

  const int lines = wxExGetNumberOfLines(m_STC->GetSelectedText());
  
  if (m_STC->GetSelectedText().empty())
  {
    m_STC->DeleteBack();
  }
  else
  {
    if (!m_Ex->GetRegister().empty())
    {
      m_Ex->GetMacros().SetRegister(
        m_Ex->GetRegister(), m_STC->GetSelectedText());
      m_STC->ReplaceSelection(wxEmptyString);
    }
    else
    {
      m_STC->Cut();
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

  if (lines >= 3)
  {
    m_Ex->GetFrame()->ShowExMessage(wxString::Format(_("%d fewer lines"), lines - 1));
  }

  return true;
}

bool wxExAddressRange::Filter(const wxString& command) const
{
  // For example, the command:
  // :96,99!sort
  // will pass lines 96 through 99 through the sort (36.1) filter and 
  // replace those lines with the output of sort.  
  const int begin_line = m_Begin.ToLine();
  const int end_line = m_End.ToLine();

  if (!IsOk() || begin_line == 0 || end_line == 0 || end_line < begin_line)
  {
    return false;
  }

  char buffer[255];
  tmpnam(buffer);
  
  wxTextFile file(buffer);
  
  if (!file.Create())
  {
    return false;
  }

  for (int i = begin_line - 1; i <= end_line - 1; i++)
  {
    file.AddLine(m_STC->GetLine(i).Trim());
  }
  
  if (!file.Write())
  {
    return false;
  }
    
  wxExProcess process;
  
  const bool ok = process.Execute(command + " " + buffer, wxEXEC_SYNC);
  
  remove(buffer);
  
  if (ok)
  {
    if (!process.HasStdError())
    {      
      m_STC->BeginUndoAction();
      
      Delete();
      m_STC->AddText(process.GetOutput());
      
      m_STC->EndUndoAction();
      
      return true;
    }
    else
    {
      m_Ex->GetFrame()->ShowExMessage(process.GetOutput());
    }
  }
  
  return false;
}

bool wxExAddressRange::Indent(bool forward) const
{
  if (!IsOk() || m_STC->GetReadOnly() || m_STC->HexMode())
  {
    return false;
  }
  
  if (m_STC->GetSelectedText().empty())
  {
    if (!SetSelection())
    {
      return false;
    }
  }
  else
  {
    if (wxExGetNumberOfLines(m_STC->GetSelectedText()) == 1)
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

  if (m_STC->GetSelectedText().empty())
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

  m_STC->BeginUndoAction();

  m_STC->Cut();
  m_STC->GotoLine(dest_line - 1);
  m_STC->Paste();

  m_STC->EndUndoAction();
  
  const int lines = wxExGetNumberOfLines(m_STC->GetSelectedText());
  
  if (lines >= 2)
  {
    m_Ex->GetFrame()->ShowExMessage(wxString::Format(_("%d lines moved"), lines));
  }

  return true;
}

void wxExAddressRange::Set(const wxString& value)
{
  m_Begin.assign(value);
  m_End.assign(value);
}

void wxExAddressRange::Set(const wxString& begin, const wxString& end)
{
  m_Begin.assign(begin);
  m_End.assign(end);
}

bool wxExAddressRange::SetSelection() const
{
  const int begin_line = m_Begin.ToLine();
  const int end_line = m_End.ToLine();

  if (!IsOk() || end_line < begin_line)
  {
    return false;
  }

  m_STC->SetSelection(
    m_STC->PositionFromLine(begin_line - 1),
    m_STC->PositionFromLine(end_line));

  return true;
}

bool wxExAddressRange::Write(const wxString& filename) const
{
  int begin = m_Begin.m_Pos;
  int end = m_End.m_Pos;
    
  if (m_STC->GetSelectedText().empty())
  {
    const int begin_line = m_Begin.ToLine();
    const int end_line = m_End.ToLine();

    if (!IsOk() || end_line < begin_line)
    {
      return false;
    }
   
    begin = m_STC->PositionFromLine(begin_line - 1);
    end = m_STC->PositionFromLine(end_line);
  }

  wxFile file(filename, wxFile::write);

  return 
    file.IsOpened() && 
    file.Write(m_STC->GetTextRange(begin, end));
}

bool wxExAddressRange::Yank() const
{
  int begin = m_Begin.m_Pos;
  int end = m_End.m_Pos;
    
  if (m_STC->GetSelectedText().empty())
  {
    const int begin_line = m_Begin.ToLine();
    const int end_line = m_End.ToLine();

    if (!IsOk())
    {
      return false;
    }

    begin = m_STC->PositionFromLine(begin_line - 1);
    end = m_STC->PositionFromLine(end_line);
  
    if (begin == end)
    {
      return false;
    }
  }

  if (!m_Ex->GetRegister().empty())
  {
    m_Ex->GetMacros().SetRegister(
      m_Ex->GetRegister(), 
      m_STC->GetTextRange(begin, end));
  }
  else
  {
    m_STC->CopyRange(begin, end);
  }

  const wxString range(m_STC->GetTextRange(begin, end));
  m_Ex->SetRegisterYank(range);
  
  const int lines = wxExGetNumberOfLines(range);
  
  if (lines >= 3)
  {
    m_Ex->GetFrame()->ShowExMessage(wxString::Format(_("%d lines yanked"), lines - 1));
  }

  return true;
}

#endif // wxUSE_GUI
