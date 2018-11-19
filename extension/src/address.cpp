////////////////////////////////////////////////////////////////////////////////
// Name:      address.cpp
// Purpose:   Implementation of class wex::address
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <memory>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/address.h>
#include <wex/ex.h>
#include <wex/managedframe.h>
#include <wex/process.h>
#include <wex/stc.h>
#include <wex/util.h>
#include <wex/vi-macros.h>

#define SEPARATE                                          \
  if (separator)                                          \
  {                                                       \
    output += std::string(40, '-') + m_Ex->stc()->eol();  \
  }                                                       \

bool wex::address::adjust_window(const std::string& text) const
{
  std::vector<std::string> v;
  
  if (match("([-+=.^]*)([0-9]+)?(.*)", text, v) != 3)
  {
    return false;
  }
  
  const auto count = (v[1].empty() ? 2: std::stoi(v[1]));
  const auto flags(v[2]);
  
  if (!flags_supported(flags))
  {
    return false;
  }
  
  int begin = get_line();
  bool separator = false;
  
  if (const auto type(v[0]); !type.empty())
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
      m_Ex->stc()->GetLine(i - 1);
  }
  SEPARATE;
    
  m_Ex->frame()->print_ex(m_Ex, output);
  
  return true;
}
  
bool wex::address::append(const std::string& text) const
{
  if (m_Ex->stc()->GetReadOnly() || m_Ex->stc()->is_hexmode() || get_line() <= 0)
  {
    return false;
  }
  
  m_Ex->stc()->InsertText(m_Ex->stc()->PositionFromLine(get_line()), text);
  
  return true;
}
  
bool wex::address::flags_supported(const std::string& flags) const
{
  if (flags.empty())
  {
    return true;
  }
  
  if (std::vector<std::string> v; match("([-+#pl])", flags, v) < 0)
  {
    log_status("Unsupported flags: " + flags);
    return false;
  }
  
  return true;
}
  
int wex::address::get_line() const
{
  // We already have a line number, return that one.
  if (m_Line >= 1)
  {
    return m_Line;
  }

  m_Ex->stc()->set_search_flags(m_Ex->search_flags());

  // If this is a // address, return line with first forward match.
  if (std::vector <std::string> v;
    match("/(.*)/$", m_Address, v) > 0)
  {
    m_Ex->stc()->SetTargetStart(m_Ex->stc()->GetCurrentPos());
    m_Ex->stc()->SetTargetEnd(m_Ex->stc()->GetTextLength());
    
    if (m_Ex->stc()->SearchInTarget(v[0]) != -1)
    {
      return m_Ex->stc()->LineFromPosition(m_Ex->stc()->GetTargetStart()) + 1;
    }
    
    m_Ex->stc()->SetTargetStart(0);
    m_Ex->stc()->SetTargetEnd(m_Ex->stc()->GetCurrentPos());
    
    if (m_Ex->stc()->SearchInTarget(v[0]) != -1)
    {
      return m_Ex->stc()->LineFromPosition(m_Ex->stc()->GetTargetStart()) + 1;
    }
    
    return 0;
  }
  // If this is a ?? address, return line with first backward match.
  else if (match("\\?(.*)\\?", m_Address, v) > 0)
  {
    m_Ex->stc()->SetTargetStart(m_Ex->stc()->GetCurrentPos());
    m_Ex->stc()->SetTargetEnd(0);
    
    if (m_Ex->stc()->SearchInTarget(v[0]) != -1)
    {
      return m_Ex->stc()->LineFromPosition(m_Ex->stc()->GetTargetStart()) + 1;
    }
    
    m_Ex->stc()->SetTargetStart(m_Ex->stc()->GetTextLength());
    m_Ex->stc()->SetTargetEnd(m_Ex->stc()->GetCurrentPos());
    
    if (m_Ex->stc()->SearchInTarget(v[0]) != -1)
    {
      return m_Ex->stc()->LineFromPosition(m_Ex->stc()->GetTargetStart()) + 1;
    }
    
    return 0;
  }
  
  // Try address calculation. studio not yet: [[maybe_unused]]
  if (const auto [sum, width] = m_Ex->calculator(m_Address); std::isnan(sum))
  {
    return 0;
  }
  else if (sum < 0)
  {
    return 1;
  }
  else if (sum > m_Ex->stc()->GetLineCount())
  {
    return m_Ex->stc()->GetLineCount();
  }  
  else
  {
    return sum;
  }
}

bool wex::address::insert(const std::string& text) const
{
  if (m_Ex->stc()->GetReadOnly() || m_Ex->stc()->is_hexmode() || get_line() <= 0)
  {
    return false;
  }
  
  m_Ex->stc()->InsertText(m_Ex->stc()->PositionFromLine(get_line() - 1), text);
  
  return true;
}
  
bool wex::address::marker_add(char marker) const
{
  return get_line() > 0 && m_Ex->marker_add(marker, get_line() - 1);
}
  
bool wex::address::marker_delete() const
{
  return m_Address.size() > 1 && m_Address[0] == '\'' &&
    m_Ex->marker_delete(m_Address[1]);
}

bool wex::address::put(char name) const
{
  if (m_Ex->stc()->GetReadOnly() || m_Ex->stc()->is_hexmode() || get_line() <= 0)
  {
    return false;
  }
  
  m_Ex->stc()->InsertText(
    m_Ex->stc()->PositionFromLine(get_line()), 
    m_Ex->get_macros().get_register(name));

  return true;
}

bool wex::address::read(const std::string& arg) const
{
  if (m_Ex->stc()->GetReadOnly() || m_Ex->stc()->is_hexmode() || get_line() <= 0)
  {
    return false;
  }
  
  if (arg.find("!") == 0)
  {
    process process;
    
    if (!process.execute(arg.substr(1), process::EXEC_WAIT))
    {
      return false;
    }
    
    return append(process.get_stdout());
  }
  else
  {
    path::current(m_Ex->stc()->get_filename().get_path());
    
    if (file file(arg); !file.is_opened())
    {
      wxLogStatus(_("file: %s open error"), file.get_filename().data().string());
      return false;
    }
    else if (const auto buffer(file.read()); buffer != nullptr)
    {
      if (m_Address == ".")
      {
        m_Ex->stc()->AddTextRaw(buffer->data(), buffer->size());
      }
      else
      {
        // README: InsertTextRaw does not have length argument.
        m_Ex->stc()->InsertTextRaw(
          m_Ex->stc()->PositionFromLine(get_line()), buffer->data());
      }
    }
    else
    {
      wxLogStatus(_("file: %s read failed"), arg);
      return false;
    }
      
    return true;
  }
}
  
void wex::address::SetLine(int line)
{
  if (line > m_Ex->stc()->GetLineCount())
  {
    m_Line = m_Ex->stc()->GetLineCount();
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

bool wex::address::write_line_number() const
{
  if (get_line() <= 0)
  {
    return false;
  }
  
  m_Ex->frame()->show_ex_message(std::to_string(get_line()));
  
  return true;
}
