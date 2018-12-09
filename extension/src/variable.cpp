////////////////////////////////////////////////////////////////////////////////
// Name:      variable.cpp
// Purpose:   Implementation of class wex::variable
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/variable.h>
#include <wex/ex.h>
#include <wex/log.h>
#include <wex/stc.h>
#include <wex/stcdlg.h>
#include <wex/util.h>
#include <wex/vi-macros.h>
#include <wex/vi-macros-mode.h>
#include <easylogging++.h>

wex::stc_entry_dialog* wex::variable::m_Dialog = nullptr;

wex::variable::variable(const pugi::xml_node& node)
  : m_Name(node.attribute("name").value())
  , m_Value(node.text().get())
  , m_Prefix(node.attribute("prefix").value())
{
  if (const std::string type = node.attribute("type").value(); !type.empty())
  {
    if (type == "BUILTIN")
    {
      m_Type = VARIABLE_BUILTIN;
    }
    else if (type == "ENVIRONMENT")
    {
      m_Type = VARIABLE_ENVIRONMENT;
    }
    else if (type == "INPUT")
    {
      m_Type = VARIABLE_INPUT;
    }
    else if (type == "INPUT-SAVE")
    {
      m_Type = VARIABLE_INPUT_SAVE;
    }
    else if (type == "INPUT-ONCE")
    {
      m_Type = VARIABLE_INPUT_ONCE;
    }
    else if (type == "TEMPLATE")
    {
      m_Type = VARIABLE_TEMPLATE;
    }
    else
    {
      VLOG(9) << "variable type is not supported:" << type;
    }
  }
}

bool wex::variable::CheckLink(std::string& value) const
{
  if (std::vector <std::string> v;
    match("@([a-zA-Z].+)@", m_Value, v) > 0)
  {
    if (const auto& it = vi_macros::get_variables().find(v[0]);
      it != vi_macros::get_variables().end())
    {
      if (!it->second.expand(value))
      {
        if (!is_input())
        {
          log() << "variable:" << m_Name << "(" << v[0] << ") could not be expanded";
        }
      }
      else
      {
        if (!value.empty())
        {
          VLOG(9) << "variable:" << m_Name << " (" << v[0] << ") expanded: " << value;
          return true;
        }
      }
    }
    else
    {
      log() << "variable:" << m_Name << "(" << v[0] << ") is not found";
    }
  }
  
  return false;
}

bool wex::variable::expand(ex* ex)
{
  std::string value;

  if (CheckLink(value))
  {
    m_Value = value;
    value.clear();
  }

  if (!expand(value, ex))
  {
    if (!is_input())
    // Now only show log status if this is no input variable,
    // as it was cancelled in that case.    
    {
      wxLogStatus(_("Could not expand variable") + ": "  +  m_Name);
    }

    return false;
  }
  
  // If there is a prefix, make a comment out of it.
  auto commented(value);

  if (ex != nullptr)
  { 
    if (ex->get_stc()->GetReadOnly() || ex->get_stc()->is_hexmode())
    {
      return false;
    }

    if (!m_Prefix.empty())
    {
      commented = ex->get_stc()->get_lexer().make_comment(
        m_Prefix == "WRAP" ? std::string(): m_Prefix, value);
    }

    ex->add_text(commented);
  }
  
  if (m_Type == VARIABLE_INPUT_SAVE || m_Type == VARIABLE_INPUT_ONCE)
  {
    m_Value = value;

    VLOG(9) << "variable: " << m_Name << " expanded and saved: " << m_Value;
  }
  else 
  {
    VLOG(9) << "variable: " << m_Name << " expanded to: " << value;
  }

  if (m_Type == VARIABLE_INPUT_ONCE && !m_Value.empty())
  {
    m_AskForInput = false;
  }

  return true;
}

bool wex::variable::expand(std::string& value, ex* ex) const
{
  CheckLink(value);

  switch (m_Type)
  {
    case VARIABLE_BUILTIN:
      if (!ExpandBuiltIn(ex, value))
      {
        return false;
      }
      break;
      
    case VARIABLE_ENVIRONMENT:
      if (wxString val; !wxGetEnv(m_Name, &val))
      {
        return false;
      }
      else 
      {
        value = val;
      }
      break;
      
    case VARIABLE_INPUT:
    case VARIABLE_INPUT_ONCE:
    case VARIABLE_INPUT_SAVE:
      if (!ExpandInput(value))
      {
        return false;
      }
      break;
      
    case VARIABLE_READ:
      if (m_Value.empty())
      {
        return false;
      }
      value = m_Value;
      break;
      
    case VARIABLE_TEMPLATE:
      if (!vi_macros::mode()->expand(ex, *this, value))
      {
        return false;
      }
      break;
      
    default: assert(0); break;
  }
  
  return true;
}

bool wex::variable::ExpandBuiltIn(ex* ex, std::string& expanded) const
{
  if (m_Name == "Date")
  {
    expanded = wxDateTime::Now().FormatISODate();
  }
  else if (m_Name == "Datetime")
  {
    expanded = wxDateTime::Now().FormatISOCombined(' ');
  }
  else if (m_Name == "Time")
  {
    expanded = wxDateTime::Now().FormatISOTime();
  }
  else if (m_Name == "Year")
  {
    expanded = wxDateTime::Now().Format("%Y");
  }
  else if (ex != nullptr)
  {
    if (m_Name == "Cb")
    {
      expanded = ex->get_stc()->get_lexer().comment_begin();
    }
    else if (m_Name == "Cc")
    {
      const int line = ex->get_stc()->GetCurrentLine();
      const int startPos = ex->get_stc()->PositionFromLine(line);
      const int endPos = ex->get_stc()->GetLineEndPosition(line);
      expanded = ex->get_stc()->get_lexer().comment_complete(
        ex->get_stc()->GetTextRange(startPos, endPos).ToStdString());
    }
    else if (m_Name == "Ce")
    {
      expanded = ex->get_stc()->get_lexer().comment_end();
    }
    else if (m_Name == "Cl")
    {
      expanded = ex->get_stc()->get_lexer().make_comment(std::string(), false);
    }
    else if (m_Name == "Created")
    {
      if (path file(ex->get_stc()->get_filename());
        ex->get_stc()->get_filename().stat().is_ok())
      {
        expanded = wxDateTime(file.stat().st_ctime).FormatISODate();
      }
      else
      {
        expanded = wxDateTime::Now().FormatISODate();
      }
    }
    else if (m_Name == "Filename")
    {
      expanded = ex->get_stc()->get_filename().name();
    }
    else if (m_Name == "Fullname")
    {
      expanded = ex->get_stc()->get_filename().fullname();
    }
    else if (m_Name == "Fullpath")
    {
      expanded = ex->get_stc()->get_filename().data().string();
    }
    else if (m_Name == "Nl")
    {
      expanded = ex->get_stc()->eol();
    }
    else if (m_Name == "Path")
    {
      expanded = ex->get_stc()->get_filename().get_path();
    }
    else
    {
      return false;
    }
  }
  
  return true;
}

bool wex::variable::ExpandInput(std::string& expanded)  const
{
  if (m_AskForInput)
  {
    const auto use(!expanded.empty() ? expanded: m_Value);

    if (m_Dialog == nullptr)
    {
      m_Dialog = new stc_entry_dialog(
        use,
        std::string(),
        window_data().title(m_Name + ":"));
        
      m_Dialog->get_stc()->get_vi().use(false);
      m_Dialog->get_stc()->SetWrapMode(wxSTC_WRAP_WORD);
    }

    m_Dialog->SetTitle(m_Name);
    m_Dialog->get_stc()->set_text(use);
    m_Dialog->get_stc()->SetFocus();
    
    bool ended = false;
    
    if (wxIsBusy())
    {
      ended = true;
      wxEndBusyCursor();
    }
    
    const int result = m_Dialog->ShowModal();
    
    if (ended)
    {
      wxBeginBusyCursor();
    }
    
    if (result == wxID_CANCEL)
    {
      return false;
    }
      
    const wxString value = m_Dialog->get_stc()->GetText();
    
    if (value.empty())
    {
      return false;
    }
    
    expanded = value;
  }
  else
  {
    expanded = m_Value;
  }
  
  return true;
}

void wex::variable::save(pugi::xml_node& node, const std::string* value)
{
  assert(!m_Name.empty());

  if (!node.attribute("name"))
  {
    node.append_attribute("name") = m_Name.c_str();
  }

  if (!node.attribute("type"))
  {
    pugi::xml_attribute type = node.append_attribute("type");
    
    switch (m_Type)
    {
      case VARIABLE_BUILTIN: type.set_value("BUILTIN"); break;
      case VARIABLE_ENVIRONMENT: type.set_value("ENVIRONMENT"); break;
      case VARIABLE_INPUT: type.set_value("INPUT"); break;
      case VARIABLE_INPUT_ONCE: type.set_value("INPUT-ONCE"); break;
      case VARIABLE_INPUT_SAVE: type.set_value("INPUT-SAVE"); break;
      case VARIABLE_READ: break;
      case VARIABLE_TEMPLATE: type.set_value("TEMPLATE"); break;

      default: assert(0); break;
    }
  }
  
  if (!m_Prefix.empty() && !node.attribute("prefix"))
  {
    node.append_attribute("prefix") = m_Prefix.c_str();
  }

  if (value != nullptr)
  {
    m_Value = *value;
  }
    
  if (!m_Value.empty() && m_Type != VARIABLE_INPUT)
  {
    node.text().set(m_Value.c_str());
  }
} 

void wex::variable::set_ask_for_input(bool value) 
{
  if (!value)
  {
    m_AskForInput = value;
  }
  else if (is_input() && m_Type != VARIABLE_INPUT_ONCE)
  {
    m_AskForInput = value;
  }
}
