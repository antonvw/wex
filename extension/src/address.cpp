////////////////////////////////////////////////////////////////////////////////
// Name:      address.cpp
// Purpose:   Implementation of class wxExAddress
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
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
  , m_Line(0)
{
}

wxExAddress::~wxExAddress()
{
}
  
bool wxExAddress::Append(const wxString& text) const
{
  if (m_Ex->GetSTC()->GetReadOnly() || m_Ex->GetSTC()->HexMode() || GetLine() <= 0)
  {
    return false;
  }
  
  m_Ex->GetSTC()->InsertText(m_Ex->GetSTC()->PositionFromLine(GetLine()), text);
  
  return true;
}
  
int wxExAddress::GetLine() const
{
  // We already have a line number, return that one.
  if (m_Line >= 1)
  {
    return m_Line;
  }

  // If this is a // address, return line with first forward match.
  std::vector <wxString> v;
  
  if (wxExMatch("/(.*)/", ToStdString(), v))
  {
    m_Ex->GetSTC()->SetTargetStart(m_Ex->GetSTC()->GetCurrentPos());
    m_Ex->GetSTC()->SetTargetEnd(m_Ex->GetSTC()->GetTextLength());
    
    if (m_Ex->GetSTC()->SearchInTarget(v[0]) != -1)
    {
      return m_Ex->GetSTC()->LineFromPosition(m_Ex->GetSTC()->GetTargetStart()) + 1;
    }
    
    m_Ex->GetSTC()->SetTargetStart(0);
    m_Ex->GetSTC()->SetTargetEnd(m_Ex->GetSTC()->GetCurrentPos());
    
    if (m_Ex->GetSTC()->SearchInTarget(v[0]) != -1)
    {
      return m_Ex->GetSTC()->LineFromPosition(m_Ex->GetSTC()->GetTargetStart()) + 1;
    }
    
    return 0;
  }

  // If this is a ?? address, return line with first backward match.
  if (wxExMatch("\\?(.*)\\?", ToStdString(), v))
  {
    m_Ex->GetSTC()->SetTargetStart(m_Ex->GetSTC()->GetCurrentPos());
    m_Ex->GetSTC()->SetTargetEnd(0);
    
    if (m_Ex->GetSTC()->SearchInTarget(v[0]) != -1)
    {
      return m_Ex->GetSTC()->LineFromPosition(m_Ex->GetSTC()->GetTargetStart()) + 1;
    }
    
    m_Ex->GetSTC()->SetTargetStart(m_Ex->GetSTC()->GetTextLength());
    m_Ex->GetSTC()->SetTargetEnd(m_Ex->GetSTC()->GetCurrentPos());
    
    if (m_Ex->GetSTC()->SearchInTarget(v[0]) != -1)
    {
      return m_Ex->GetSTC()->LineFromPosition(m_Ex->GetSTC()->GetTargetStart()) + 1;
    }
    
    return 0;
  }
  
  // Try address calculation.
  int width = 0;
  const int sum = wxExCalculator(ToStdString(), m_Ex, width);
  
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

bool wxExAddress::Insert(const wxString& text) const
{
  if (m_Ex->GetSTC()->GetReadOnly() || m_Ex->GetSTC()->HexMode() || GetLine() <= 0)
  {
    return false;
  }
  
  m_Ex->GetSTC()->InsertText(m_Ex->GetSTC()->PositionFromLine(GetLine() - 1), text);
  
  return true;
}
  
bool wxExAddress::MarkerAdd(const wxUniChar& marker) const
{
  if (GetLine() <= 0)
  {
    return false;
  }
  
  return m_Ex->MarkerAdd(marker, GetLine() - 1);
}
  
bool wxExAddress::MarkerDelete() const
{
  if (StartsWith("'") && size() > 1)
  {
    return m_Ex->MarkerDelete(GetChar(1));
  }
  
  return false;
}

bool wxExAddress::Put(const char name) const
{
  if (m_Ex->GetSTC()->GetReadOnly() || m_Ex->GetSTC()->HexMode() || GetLine() <= 0)
  {
    return false;
  }
  
  m_Ex->GetSTC()->InsertText(
    m_Ex->GetSTC()->PositionFromLine(GetLine()), 
    m_Ex->GetMacros().GetRegister(name));

  return true;
}

bool wxExAddress::Read(const wxString& arg) const
{
  if (m_Ex->GetSTC()->GetReadOnly() || m_Ex->GetSTC()->HexMode() || GetLine() <= 0)
  {
    return false;
  }
  
  if (arg.StartsWith("!"))
  {
    wxExProcess process;
    
    if (!process.Execute(arg.AfterFirst('!'), wxEXEC_SYNC))
    {
      return false;
    }
    
    return Append(process.GetOutput());
  }
  else
  {
    wxFileName fn(arg);

    if (fn.IsRelative())
    {
      fn.Normalize(wxPATH_NORM_ALL, m_Ex->GetSTC()->GetFileName().GetPath());
    }
    
    wxExFile file;

    if (!file.Exists(fn.GetFullPath()) || !file.Open(fn.GetFullPath()))
    {
      wxLogStatus(_("file: %s does not exist"), arg);
      return false;
    }
    
    const wxCharBuffer& buffer = file.Read();
    
    if (*this == ".")
    {
      m_Ex->GetSTC()->AddTextRaw((const char *)buffer.data(), buffer.length());
    }
    else
    {
      // README: InsertTextRaw does not have length argument.
      m_Ex->GetSTC()->InsertTextRaw(
        m_Ex->GetSTC()->PositionFromLine(GetLine()), (const char *)buffer.data());
    }
      
    return true;
  }
}
  
bool wxExAddress::Show() const
{
  if (GetLine() <= 0)
  {
    return false;
  }
  
  m_Ex->GetFrame()->ShowExMessage(std::to_string(GetLine()));
  
  return true;
}
  
void wxExAddress::SetLine(int line)
{
  if (line > m_Ex->GetSTC()->GetLineCount())
  {
    m_Line = m_Ex->GetSTC()->GetLineCount();
  }
  else if (line < 1)
  {
    m_Line = 1;
  }
  else
  {
    m_Line = line;
  }
}
#endif // wxUSE_GUI
