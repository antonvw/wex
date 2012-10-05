////////////////////////////////////////////////////////////////////////////////
// Name:      variable.cpp
// Purpose:   Implementation of class wxExVariable
// Author:    Anton van Wezenbeek
// Copyright: (c) 2012 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/variable.h>
#include <wx/extension/ex.h>
#include <wx/extension/stc.h>
#include <wx/extension/stcdlg.h>

#if wxUSE_GUI

/// Several types of variables are supported.
/// See xml file.
enum
{
  VARIABLE_BUILTIN,        ///< a builtin variable
  VARIABLE_ENVIRONMENT,    ///< an environment variable
  VARIABLE_INPUT,          ///< input from user
  VARIABLE_INPUT_ONCE,     ///< input once from user, save value in xml file
  VARIABLE_INPUT_SAVE,     ///< input from user, save value in xml file
  VARIABLE_READ,           ///< read value from macros xml  file
  VARIABLE_TEMPLATE        ///< read value from a template file
};

wxExSTCEntryDialog* wxExVariable::m_Dialog = NULL;


wxExVariable::wxExVariable()
  : m_Type(VARIABLE_READ)
  , m_AskForInput(true)
  , m_IsModified(false)
{
}
  
wxExVariable::wxExVariable(const wxXmlNode* node)
  : m_Type(VARIABLE_READ)
  , m_AskForInput(true)
  , m_IsModified(false)
{
  const wxString type = node->GetAttribute("type");
   
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
      wxLogError("Variable type is not supported: " + type);
    }
  }

  m_Name = node->GetAttribute("name");
  m_Prefix = node->GetAttribute("prefix");
  m_Value = node->GetNodeContent().Strip(wxString::both);
}

wxExVariable::wxExVariable(const wxString& name)
  : m_IsModified(false)
  , m_Type(VARIABLE_INPUT_SAVE)
  , m_AskForInput(true)
  , m_Name(name)
  , m_Prefix()
  , m_Value()
{
}

wxExVariable::~wxExVariable()
{
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
  
  wxString text;
  
  if (!Expand(ex, text))
  {
    return false;
  }
  
  ex->GetSTC()->AddText(text);
    
  return true;
}

bool wxExVariable::Expand(wxExEx* ex, wxString& value)
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
      if (!wxGetEnv(m_Name, &value))
      {
        return false;
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
      if (!wxExViMacros::ExpandTemplate(ex, *this, value))
      {
        return false;
      }
      break;
      
    default: wxFAIL; break;
  }
  
  // If there is a prefix, make a comment out of it.
  if (!m_Prefix.empty())
  {
    value = ex->GetSTC()->GetLexer().MakeComment(m_Prefix, value);
  }
  
  return true;
}

bool wxExVariable::ExpandBuiltIn(wxExEx* ex, wxString& expanded) const
{
  if (m_Name == "CB")
  {
    expanded = ex->GetSTC()->GetLexer().GetCommentBegin();
  }
  else if (m_Name == "CC")
  {
    const int line = ex->GetSTC()->GetCurrentLine();
    const int startPos = ex->GetSTC()->PositionFromLine(line);
    const int endPos = ex->GetSTC()->GetLineEndPosition(line);
    expanded = ex->GetSTC()->GetLexer().CommentComplete(
      ex->GetSTC()->GetTextRange(startPos, endPos));
  }
  else if (m_Name == "CE")
  {
    expanded = ex->GetSTC()->GetLexer().GetCommentEnd();
  }
  else if (m_Name == "CL")
  {
    expanded = ex->GetSTC()->GetLexer().MakeComment(wxEmptyString, false);
  }
  else if (m_Name == "CREATED")
  {
    wxFileName file(ex->GetSTC()->GetFileName());
    wxDateTime dtCreate;
    
    if (file.GetTimes (NULL, NULL, &dtCreate))
    {
      expanded = dtCreate.FormatISODate();
    }
    else
    {
      expanded = wxDateTime::Now().FormatISODate();
    }
  }
  else if (m_Name == "DATE")
  {
    expanded = wxDateTime::Now().FormatISODate();
  }
  else if (m_Name == "DATETIME")
  {
    expanded = wxDateTime::Now().FormatISOCombined(' ');
  }
  else if (m_Name == "FILENAME")
  {
    expanded = ex->GetSTC()->GetFileName().GetName();
  }
  else if (m_Name == "FULLNAME")
  {
    expanded = ex->GetSTC()->GetFileName().GetFullName();
  }
  else if (m_Name == "FULLPATH")
  {
    expanded = ex->GetSTC()->GetFileName().GetFullPath();
  }
  else if (m_Name == "NL")
  {
    expanded = ex->GetSTC()->GetEOL();
  }
  else if (m_Name == "PATH")
  {
    expanded = ex->GetSTC()->GetFileName().GetPath();
  }
  else if (m_Name == "TIME")
  {
    expanded = wxDateTime::Now().FormatISOTime();
  }
  else if (m_Name == "YEAR")
  {
    expanded = wxDateTime::Now().Format("%Y");
  }
  else
  {
    return false;
  }
  
  return true;
}

bool wxExVariable::ExpandInput(wxString& expanded)
{
  if (m_AskForInput || m_Value.empty())
  {
    wxString value;
    
    if (m_Dialog == NULL)
    {
      m_Dialog = new wxExSTCEntryDialog(
        wxTheApp->GetTopWindow(),
        m_Name, 
        m_Value);
        
      m_Dialog->GetSTC()->GetVi().Use(false);
    }
    else
    {
      m_Dialog->SetTitle(m_Name);
      m_Dialog->GetSTC()->SetText(m_Value);
    }
        
    m_Dialog->GetSTC()->SetFocus();
    
    if (m_Dialog->ShowModal() == wxID_CANCEL)
    {
      return false;
    }
      
    value = m_Dialog->GetText();
    
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

void wxExVariable::Save(wxXmlNode* node) const
{
  node->AddAttribute("name", m_Name);
  
  switch (m_Type)
  {
    case VARIABLE_BUILTIN:
      node->AddAttribute("type", "BUILTIN");
      break;
      
    case VARIABLE_ENVIRONMENT:
      node->AddAttribute("type", "ENVIRONMENT");
      break;
      
    case VARIABLE_INPUT:
      node->AddAttribute("type", "INPUT");
      break;
      
    case VARIABLE_INPUT_ONCE:
      node->AddAttribute("type", "INPUT-ONCE");
      break;
    
    case VARIABLE_INPUT_SAVE:
      node->AddAttribute("type", "INPUT-SAVE");
      break;
    
    case VARIABLE_READ:
      break;
      
    case VARIABLE_TEMPLATE:
      node->AddAttribute("type", "TEMPLATE");
      break;
      
    default: wxFAIL; break;
  }
  
  if (!m_Prefix.empty())
  {
    node->AddAttribute("prefix", m_Prefix);
  }
    
  if (!m_Value.empty() && m_Type != VARIABLE_INPUT)
  {
    new wxXmlNode(node, wxXML_TEXT_NODE, "", m_Value);
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
