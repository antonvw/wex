////////////////////////////////////////////////////////////////////////////////
// Name:      variable.cpp
// Purpose:   Implementation of class wxExVariable
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <easylogging++.h>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/variable.h>
#include <wx/extension/ex.h>
#include <wx/extension/stc.h>
#include <wx/extension/stcdlg.h>
#include <wx/extension/util.h>
#include <wx/extension/vi-macros.h>
#include <wx/extension/vi-macros-mode.h>

#if wxUSE_GUI

wxExSTCEntryDialog* wxExVariable::m_Dialog = nullptr;

wxExVariable::wxExVariable(const pugi::xml_node& node)
  : m_Name(node.attribute("name").value())
  , m_Value(node.text().get())
  , m_Prefix(node.attribute("prefix").value())
{
  const std::string type = node.attribute("type").value();

  if (!type.empty())
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
      LOG(ERROR) << "variable type is not supported: " << type;
    }
  }
}

bool wxExVariable::CheckLink(std::string& value) const
{
  std::vector <std::string> v;

  if (wxExMatch("@([a-zA-Z].+)@", m_Value, v) > 0)
  {
    const auto& it = wxExViMacros::GetVariables().find(v[0]);

    if (it != wxExViMacros::GetVariables().end())
    {
      if (!it->second.Expand(value))
      {
        if (!IsInput())
        {
          LOG(ERROR) << "variable: " << m_Name << " (" << v[0] << ") could not be expanded";
        }
      }
      else
      {
        if (!value.empty())
        {
          VLOG(9) << "variable: " << m_Name << " (" << v[0] << ") expanded: " << value;
          return true;
        }
      }
    }
    else
    {
      LOG(ERROR) << "variable: " << m_Name << " (" << v[0] << ") is not found";
    }
  }
  
  return false;
}

bool wxExVariable::Expand(wxExEx* ex)
{
  std::string value;

  if (CheckLink(value))
  {
    m_Value = value;
    value.clear();
  }

  if (!Expand(value, ex))
  {
    if (!IsInput())
    // Now only show log status if this is no input variable,
    // as it was cancelled in that case.    
    {
      wxLogStatus(_("Could not expand variable") + ": "  +  m_Name);
    }

    return false;
  }
  
  // If there is a prefix, make a comment out of it.
  std::string commented(value);

  if (ex != nullptr)
  { 
    if (ex->GetSTC()->GetReadOnly() || ex->GetSTC()->HexMode())
    {
      return false;
    }

    if (!m_Prefix.empty())
    {
      commented = ex->GetSTC()->GetLexer().MakeComment(
        m_Prefix == "WRAP" ? std::string(): m_Prefix, value);
    }

    ex->AddText(commented);
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

bool wxExVariable::Expand(std::string& value, wxExEx* ex) const
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
      {
      wxString val;
      if (!wxGetEnv(m_Name, &val))
      {
        return false;
      }
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
      if (!wxExViMacros::Mode()->Expand(ex, *this, value))
      {
        return false;
      }
      break;
      
    default: wxFAIL; break;
  }
  
  return true;
}

bool wxExVariable::ExpandBuiltIn(wxExEx* ex, std::string& expanded) const
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
      expanded = ex->GetSTC()->GetLexer().GetCommentBegin();
    }
    else if (m_Name == "Cc")
    {
      const int line = ex->GetSTC()->GetCurrentLine();
      const int startPos = ex->GetSTC()->PositionFromLine(line);
      const int endPos = ex->GetSTC()->GetLineEndPosition(line);
      expanded = ex->GetSTC()->GetLexer().CommentComplete(
        ex->GetSTC()->GetTextRange(startPos, endPos).ToStdString());
    }
    else if (m_Name == "Ce")
    {
      expanded = ex->GetSTC()->GetLexer().GetCommentEnd();
    }
    else if (m_Name == "Cl")
    {
      expanded = ex->GetSTC()->GetLexer().MakeComment(std::string(), false);
    }
    else if (m_Name == "Created")
    {
      wxExPath file(ex->GetSTC()->GetFileName());
      
      if (ex->GetSTC()->GetFileName().GetStat().IsOk())
      {
        expanded = wxDateTime(file.GetStat().st_ctime).FormatISODate();
      }
      else
      {
        expanded = wxDateTime::Now().FormatISODate();
      }
    }
    else if (m_Name == "Filename")
    {
      expanded = ex->GetSTC()->GetFileName().GetName();
    }
    else if (m_Name == "Fullname")
    {
      expanded = ex->GetSTC()->GetFileName().GetFullName();
    }
    else if (m_Name == "Fullpath")
    {
      expanded = ex->GetSTC()->GetFileName().Path().string();
    }
    else if (m_Name == "Nl")
    {
      expanded = ex->GetSTC()->GetEOL();
    }
    else if (m_Name == "Path")
    {
      expanded = ex->GetSTC()->GetFileName().GetPath();
    }
    else
    {
      return false;
    }
  }
  
  return true;
}

bool wxExVariable::ExpandInput(std::string& expanded)  const
{
  if (m_AskForInput)
  {
    const std::string use(!expanded.empty() ? expanded: m_Value);

    if (m_Dialog == nullptr)
    {
      m_Dialog = new wxExSTCEntryDialog(
        use,
        std::string(),
        wxExWindowData().Title(m_Name + ":"));
        
      m_Dialog->GetSTC()->GetVi().Use(false);
      m_Dialog->GetSTC()->SetWrapMode(wxSTC_WRAP_WORD);
    }

    m_Dialog->SetTitle(m_Name);
    m_Dialog->GetSTC()->SetText(use);
    m_Dialog->GetSTC()->SetFocus();
    
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
      
    const wxString value = m_Dialog->GetSTC()->GetText();
    
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

void wxExVariable::Save(pugi::xml_node& node, const std::string* value)
{
  wxASSERT(!m_Name.empty());

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

      default: wxFAIL; break;
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

void wxExVariable::SetAskForInput(bool value) 
{
  if (!value)
  {
    m_AskForInput = value;
  }
  else if (IsInput() && m_Type != VARIABLE_INPUT_ONCE)
  {
    m_AskForInput = value;
  }
}
#endif // wxUSE_GUI
