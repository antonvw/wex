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
#include <wx/extension/vi-macros.h>
#include <wx/extension/vi-macros-mode.h>

#if wxUSE_GUI

wxExSTCEntryDialog* wxExVariable::m_Dialog = nullptr;

wxExVariable::wxExVariable(
  const std::string& name,
  const std::string& value,
  const std::string& prefix,
  int type,
  bool ask_for_input)
  : m_Type(type)
  , m_AskForInput(ask_for_input)
  , m_Name(name)
  , m_Prefix(prefix)
  , m_Value(value)
{
}

wxExVariable::wxExVariable(const pugi::xml_node& node)
  : wxExVariable(
      node.attribute("name").value(),
      node.text().get(),
      node.attribute("prefix").value(),
      VARIABLE_READ, true)
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
      m_AskForInput = false;
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

void wxExVariable::AskForInput() 
{
  if (m_Type == VARIABLE_INPUT || m_Type == VARIABLE_INPUT_SAVE)
  {
    m_AskForInput = true;
  }
}

bool wxExVariable::Expand(wxExEx* ex)
{
  if (ex->GetSTC()->GetReadOnly() || ex->GetSTC()->HexMode())
  {
    return false;
  }
  
  std::string text;
  
  if (!Expand(ex, text))
  {
    return false;
  }
  
  ex->AddText(text);
    
  return true;
}

bool wxExVariable::Expand(wxExEx* ex, std::string& value)
{
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
  
  // If there is a prefix, make a comment out of it.
  if (!m_Prefix.empty())
  {
    value = ex->GetSTC()->GetLexer().MakeComment(
      m_Prefix == "WRAP" ? std::string(): m_Prefix, value);
  }
  
  return true;
}

bool wxExVariable::ExpandBuiltIn(wxExEx* ex, std::string& expanded) const
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
  else if (m_Name == "Date")
  {
    expanded = wxDateTime::Now().FormatISODate();
  }
  else if (m_Name == "Datetime")
  {
    expanded = wxDateTime::Now().FormatISOCombined(' ');
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
  else if (m_Name == "Time")
  {
    expanded = wxDateTime::Now().FormatISOTime();
  }
  else if (m_Name == "Year")
  {
    expanded = wxDateTime::Now().Format("%Y");
  }
  else
  {
    return false;
  }
  
  return true;
}

bool wxExVariable::ExpandInput(std::string& expanded)
{
  if (m_AskForInput || m_Value.empty())
  {
    if (m_Dialog == nullptr)
    {
      m_Dialog = new wxExSTCEntryDialog(
        m_Value,
        std::string(),
        wxExWindowData().Title(m_Name + ":"));
        
      m_Dialog->GetSTC()->GetVi().Use(false);
      m_Dialog->GetSTC()->SetWrapMode(wxSTC_WRAP_WORD);
    }
    else
    {
      m_Dialog->SetTitle(m_Name);
      m_Dialog->GetSTC()->SetText(m_Value);
    }
        
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
    
    if (m_Value != value)
    {
      m_Value = value;
      m_IsModified = true;
    }

    if (m_Type == VARIABLE_INPUT_ONCE)
    {
      m_AskForInput = false;
    }
  }
  else
  {
    expanded = m_Value;
  }
  
  return true;
}

bool wxExVariable::IsInput() const
{
  return 
     m_Type == VARIABLE_INPUT || 
     m_Type == VARIABLE_INPUT_ONCE ||
     m_Type == VARIABLE_INPUT_SAVE;
}

void wxExVariable::Save(pugi::xml_node& node) const
{
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
    
  if (!m_Value.empty() && m_Type != VARIABLE_INPUT)
  {
    node.text().set(m_Value.c_str());
  }
} 

void wxExVariable::SkipInput()
{
  if (m_Type == VARIABLE_INPUT || m_Type == VARIABLE_INPUT_SAVE)
  {
    m_AskForInput = false;
  }
}
#endif // wxUSE_GUI
