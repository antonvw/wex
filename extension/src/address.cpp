////////////////////////////////////////////////////////////////////////////////
// Name:      address.cpp
// Purpose:   Implementation of class wxExAddress
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <memory>
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
#include <wx/extension/vi-macros.h>

#if wxUSE_GUI

#define SEPARATE                                                \
  if (separator)                                                \
  {                                                             \
    output += std::string(40, '-') + m_Ex->GetSTC()->GetEOL();  \
  }                                                             \

bool wxExAddress::AdjustWindow(const std::string& text) const
{
  std::vector<std::string> v;
  
  if (wxExMatch("([-+=.^]*)([0-9]+)?(.*)", text, v) != 3)
  {
    return false;
  }
  
  const int count = (v[1].empty() ? 2: std::stoi(v[1]));
  const std::string flags(v[2]);
  
  if (!Flags(flags))
  {
    return false;
  }
  
  int begin = GetLine();
  bool separator = false;
  
  if (const std::string type(v[0]); !type.empty())
  {
    switch ((int)type.at(0))
    {
      case '-': begin -= ((type.length() * count) - 1); break;
      case '+': begin += (((type.length()  - 1) * count) + 1); break;
      case '^': begin += (((type.length()  + 1) * count) - 1); break;
      case '=': 
      case '.': 
        if (count == 0)
        {
          return false;
        }
        separator = (type.at(0) == '=');
        begin -= (count - 1) / 2;
        break;
      default: return false;
    }
  }
  
  std::string output;
  SEPARATE;
  for (int i = begin; i < begin + count; i++)
  {
    char buffer[8];
    sprintf(buffer, "%6d ", i);

    output += (flags.find("#") != std::string::npos ? buffer: "") + 
      m_Ex->GetSTC()->GetLine(i - 1);
  }
  SEPARATE;
    
  m_Ex->GetFrame()->PrintEx(m_Ex, output);
  
  return true;
}
  
bool wxExAddress::Append(const std::string& text) const
{
  if (m_Ex->GetSTC()->GetReadOnly() || m_Ex->GetSTC()->HexMode() || GetLine() <= 0)
  {
    return false;
  }
  
  m_Ex->GetSTC()->InsertText(m_Ex->GetSTC()->PositionFromLine(GetLine()), text);
  
  return true;
}
  
bool wxExAddress::Flags(const std::string& flags) const
{
  if (flags.empty())
  {
    return true;
  }
  
  if (std::vector<std::string> v; wxExMatch("([-+#pl])", flags, v) < 0)
  {
    wxLogStatus("Unsupported flags: %s", flags.c_str());
    return false;
  }
  
  return true;
}
  
int wxExAddress::GetLine() const
{
  // We already have a line number, return that one.
  if (m_Line >= 1)
  {
    return m_Line;
  }

  m_Ex->GetSTC()->SetSearchFlags(m_Ex->GetSearchFlags());

  // If this is a // address, return line with first forward match.
  if (std::vector <std::string> v;
    wxExMatch("/(.*)/$", m_Address, v) > 0)
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
  else if (wxExMatch("\\?(.*)\\?", m_Address, v) > 0)
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
  if (const auto [sum, width] = m_Ex->Calculator(m_Address); std::isnan(sum))
  {
    return 0;
  }
  else if (sum < 0)
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

bool wxExAddress::Insert(const std::string& text) const
{
  if (m_Ex->GetSTC()->GetReadOnly() || m_Ex->GetSTC()->HexMode() || GetLine() <= 0)
  {
    return false;
  }
  
  m_Ex->GetSTC()->InsertText(m_Ex->GetSTC()->PositionFromLine(GetLine() - 1), text);
  
  return true;
}
  
bool wxExAddress::MarkerAdd(char marker) const
{
  return GetLine() > 0 && m_Ex->MarkerAdd(marker, GetLine() - 1);
}
  
bool wxExAddress::MarkerDelete() const
{
  return m_Address.size() > 1 && m_Address[0] == '\'' &&
    m_Ex->MarkerDelete(m_Address[1]);
}

bool wxExAddress::Put(char name) const
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

bool wxExAddress::Read(const std::string& arg) const
{
  if (m_Ex->GetSTC()->GetReadOnly() || m_Ex->GetSTC()->HexMode() || GetLine() <= 0)
  {
    return false;
  }
  
  if (arg.find("!") == 0)
  {
    wxExProcess process;
    
    if (!process.Execute(arg.substr(1), PROCESS_EXEC_WAIT))
    {
      return false;
    }
    
    return Append(process.GetStdOut());
  }
  else
  {
    wxExPath fn(arg);

    if (fn.IsRelative())
    {
      fn.Canonical(m_Ex->GetSTC()->GetFileName().GetPath());
    }
    
    wxExFile file;

    if (!fn.FileExists() || !file.Open(fn.Path().string()))
    {
      wxLogStatus(_("file: %s does not exist"), arg);
      return false;
    }
    
    const auto buffer(file.Read());
    
    if (m_Address == ".")
    {
      m_Ex->GetSTC()->AddTextRaw((const char *)buffer->data(), buffer->length());
    }
    else
    {
      // README: InsertTextRaw does not have length argument.
      m_Ex->GetSTC()->InsertTextRaw(
        m_Ex->GetSTC()->PositionFromLine(GetLine()), (const char *)buffer->data());
    }
      
    return true;
  }
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

bool wxExAddress::WriteLineNumber() const
{
  if (GetLine() <= 0)
  {
    return false;
  }
  
  m_Ex->GetFrame()->ShowExMessage(std::to_string(GetLine()));
  
  return true;
}
#endif // wxUSE_GUI
