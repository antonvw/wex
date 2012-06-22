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
  VARIABLE_INPUT,          ///< input once from user
  VARIABLE_INPUT_SAVE,     ///< input once from user, save value in xml file
  VARIABLE_READ            ///< read value from xml file
};

wxExVariable::wxExVariable()
  : m_Type(VARIABLE_READ)
  , m_IsModified(false)
  , m_Dialog(NULL)
{
}
  
wxExVariable::wxExVariable(const wxXmlNode* node)
  : m_Type(VARIABLE_READ)
  , m_IsModified(false)
  , m_Dialog(NULL)
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
    else
    {
      wxLogError("Variable type is not supported: " + type);
    }
  }

  m_Name = node->GetAttribute("name");
  m_Prefix = node->GetAttribute("prefix");
  m_Value = node->GetNodeContent().Strip(wxString::both);
}

void wxExVariable::Clear()
{
  if (m_Type == VARIABLE_INPUT)
  {
    m_Value.clear();
  }
}

bool wxExVariable::Expand(bool playback, wxExEx* ex)
{
  if (ex->GetSTC()->GetReadOnly() || ex->GetSTC()->HexMode())
  {
    return false;
  }
  
  wxString text;
  
  switch (m_Type)
  {
    case VARIABLE_BUILTIN:
      if (!ExpandBuiltIn(ex, text))
      {
        return false;
      }
      break;
      
    case VARIABLE_ENVIRONMENT:
      if (!wxGetEnv(m_Name, &text))
      {
        return false;
      }
      break;
      
    case VARIABLE_INPUT:
    case VARIABLE_INPUT_SAVE:
      // First expand variable.
      if (!ExpandInput(playback, text))
      {
        return false;
      }
      break;
      
    case VARIABLE_READ:
      text = m_Value;
      break;
      
    default: wxFAIL; break;
  }
  
  // If there is a prefix, make a comment out of it.
  if (!m_Prefix.empty())
  {
    text = ex->GetSTC()->GetLexer().MakeComment(m_Prefix, text);
  }
  
  ex->GetSTC()->AddText(text);
    
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

bool wxExVariable::ExpandInput(bool playback, wxString& expanded)
{
  if (!playback || m_Value.empty())
  {
    wxString value;
    
    if (!m_Prefix.empty())
    {
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
        
      if (m_Dialog->ShowModal())
      {
        value = m_Dialog->GetText();
      }
    }
    else
    {
      value = wxGetTextFromUser(
        m_Name,
        wxGetTextFromUserPromptStr,
        m_Value);
    }
    
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
      
    case VARIABLE_INPUT_SAVE:
      node->AddAttribute("type", "INPUT-SAVE");
      break;
    
    case VARIABLE_READ:
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

#endif // wxUSE_GUI
