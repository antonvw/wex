////////////////////////////////////////////////////////////////////////////////
// Name:      address.cpp
// Purpose:   Implementation of class wex::address
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <memory>
#include <wex/address.h>
#include <wex/ex.h>
#include <wex/log.h>
#include <wex/managedframe.h>
#include <wex/process.h>
#include <wex/stc.h>
#include <wex/util.h>
#include <wex/vi-macros.h>

#define SEPARATE                                          \
  if (separator)                                          \
  {                                                       \
    output += std::string(40, '-') + m_ex->get_stc()->eol();  \
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
      m_ex->get_stc()->GetLine(i - 1);
  }
  SEPARATE;
    
  m_ex->frame()->print_ex(m_ex, output);
  
  return true;
}
  
bool wex::address::append(const std::string& text) const
{
  if (m_ex->get_stc()->GetReadOnly() || m_ex->get_stc()->is_hexmode() || get_line() <= 0)
  {
    return false;
  }
  
  m_ex->get_stc()->InsertText(m_ex->get_stc()->PositionFromLine(get_line()), text);
  
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
    log::status("Unsupported flags") << flags;
    return false;
  }
  
  return true;
}
  
int wex::address::get_line() const
{
  // We already have a line number, return that one.
  if (m_line >= 1)
  {
    return m_line;
  }

  m_ex->get_stc()->set_search_flags(m_ex->search_flags());

  // If this is a // address, return line with first forward match.
  if (std::vector <std::string> v;
    match("/(.*)/$", m_address, v) > 0)
  {
    m_ex->get_stc()->SetTargetStart(m_ex->get_stc()->GetCurrentPos());
    m_ex->get_stc()->SetTargetEnd(m_ex->get_stc()->GetTextLength());
    
    if (m_ex->get_stc()->SearchInTarget(v[0]) != -1)
    {
      return m_ex->get_stc()->LineFromPosition(m_ex->get_stc()->GetTargetStart()) + 1;
    }
    
    m_ex->get_stc()->SetTargetStart(0);
    m_ex->get_stc()->SetTargetEnd(m_ex->get_stc()->GetCurrentPos());
    
    if (m_ex->get_stc()->SearchInTarget(v[0]) != -1)
    {
      return m_ex->get_stc()->LineFromPosition(m_ex->get_stc()->GetTargetStart()) + 1;
    }
    
    return 0;
  }
  // If this is a ?? address, return line with first backward match.
  else if (match("\\?(.*)\\?", m_address, v) > 0)
  {
    m_ex->get_stc()->SetTargetStart(m_ex->get_stc()->GetCurrentPos());
    m_ex->get_stc()->SetTargetEnd(0);
    
    if (m_ex->get_stc()->SearchInTarget(v[0]) != -1)
    {
      return m_ex->get_stc()->LineFromPosition(m_ex->get_stc()->GetTargetStart()) + 1;
    }
    
    m_ex->get_stc()->SetTargetStart(m_ex->get_stc()->GetTextLength());
    m_ex->get_stc()->SetTargetEnd(m_ex->get_stc()->GetCurrentPos());
    
    if (m_ex->get_stc()->SearchInTarget(v[0]) != -1)
    {
      return m_ex->get_stc()->LineFromPosition(m_ex->get_stc()->GetTargetStart()) + 1;
    }
    
    return 0;
  }
  
  // Try address calculation.
  if (const auto sum = m_ex->calculator(m_address); sum < 0)
  {
    return 1;
  }
  else if (sum > m_ex->get_stc()->GetLineCount())
  {
    return m_ex->get_stc()->GetLineCount();
  }  
  else
  {
    return sum;
  }
}

bool wex::address::insert(const std::string& text) const
{
  if (m_ex->get_stc()->GetReadOnly() || m_ex->get_stc()->is_hexmode() || get_line() <= 0)
  {
    return false;
  }
  
  m_ex->get_stc()->InsertText(m_ex->get_stc()->PositionFromLine(get_line() - 1), text);
  
  return true;
}
  
bool wex::address::marker_add(char marker) const
{
  return get_line() > 0 && m_ex->marker_add(marker, get_line() - 1);
}
  
bool wex::address::marker_delete() const
{
  return m_address.size() > 1 && m_address[0] == '\'' &&
    m_ex->marker_delete(m_address[1]);
}

bool wex::address::put(char name) const
{
  if (m_ex->get_stc()->GetReadOnly() || m_ex->get_stc()->is_hexmode() || get_line() <= 0)
  {
    return false;
  }
  
  m_ex->get_stc()->InsertText(
    m_ex->get_stc()->PositionFromLine(get_line()), 
    m_ex->get_macros().get_register(name));

  return true;
}

bool wex::address::read(const std::string& arg) const
{
  if (m_ex->get_stc()->GetReadOnly() || m_ex->get_stc()->is_hexmode() || get_line() <= 0)
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
    path::current(m_ex->get_stc()->get_filename().get_path());
    
    if (file file(arg, std::ios_base::in); !file.is_open())
    {
      log::status(_("File")) << file.get_filename() << "open error";
      return false;
    }
    else if (const auto buffer(file.read()); buffer != nullptr)
    {
      if (m_address == ".")
      {
        m_ex->get_stc()->add_text(*buffer);
      }
      else
      {
        // README: InsertTextRaw does not have length argument.
        m_ex->get_stc()->InsertTextRaw(
          m_ex->get_stc()->PositionFromLine(get_line()), buffer->data());
      }
      return true;
    }
    else
    {
      return false;
    }
  }
}
  
void wex::address::set_line(int line)
{
  if (line > m_ex->get_stc()->GetLineCount())
  {
    m_line = m_ex->get_stc()->GetLineCount();
  }
  else if (line < 1)
  {
    m_line = 1;
  }
  else
  {
    m_line = line;
  }
}

bool wex::address::write_line_number() const
{
  if (get_line() <= 0)
  {
    return false;
  }
  
  m_ex->frame()->show_ex_message(std::to_string(get_line()));
  
  return true;
}
