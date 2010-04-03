/******************************************************************************\
* File:          lexers.cpp
* Purpose:       Implementation of wxExLexers classes
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 2008-2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <algorithm>
#include <wx/config.h>
#include <wx/stdpaths.h>
#include <wx/stc/stc.h>
#include <wx/extension/lexers.h>
#include <wx/extension/util.h> // for wxExMatchesOneOf

wxExLexers* wxExLexers::m_Self = NULL;

wxExLexers::wxExLexers(const wxFileName& filename)
  : m_FileName(filename)
{
}

void wxExLexers::ApplyGlobalStyles(wxStyledTextCtrl* stc) const
{
  for_each (m_Styles.begin(), m_Styles.end(), 
    std::bind2nd(std::mem_fun_ref(&wxExStyle::Apply), stc));
}

void wxExLexers::ApplyHexStyles(wxStyledTextCtrl* stc) const
{
  for_each (m_StylesHex.begin(), m_StylesHex.end(), 
    std::bind2nd(std::mem_fun_ref(&wxExStyle::Apply), stc));
}

void wxExLexers::ApplyIndicators(wxStyledTextCtrl* stc) const
{
  for_each (m_Indicators.begin(), m_Indicators.end(), 
    std::bind2nd(std::mem_fun_ref(&wxExIndicator::Apply), stc));
}

const wxString wxExLexers::ApplyMacro(const wxString& text) const
{
  std::map<wxString, wxString>::const_iterator it = m_Macros.find(text);

  if (it != m_Macros.end())
  {
    return it->second;
  }
  else
  {
    return text;
  }
}

void wxExLexers::ApplyMarkers(wxStyledTextCtrl* stc) const
{
  for_each (m_Markers.begin(), m_Markers.end(), 
    std::bind2nd(std::mem_fun_ref(&wxExMarker::Apply), stc));
}

void wxExLexers::ApplyProperties(wxStyledTextCtrl* stc) const
{
  for_each (m_GlobalProperties.begin(), m_GlobalProperties.end(), 
    std::bind2nd(std::mem_fun_ref(&wxExProperty::Apply), stc));
}

const wxString wxExLexers::BuildWildCards(
  const wxFileName& filename) const
{
  const wxString allfiles_wildcard =
    _("All Files") + wxString::Format(" (%s)|%s",
      wxFileSelectorDefaultWildcardStr,
      wxFileSelectorDefaultWildcardStr);

  wxString wildcards = allfiles_wildcard;

  // Build the wildcard string using all available lexers.
  for (
    std::map<wxString, wxExLexer>::const_iterator it = m_Lexers.begin();
    it != m_Lexers.end();
    ++it)
  {
    if (!it->second.GetExtensions().empty())
    {
      const wxString wildcard =
        it->second.GetScintillaLexer() +
        " (" + it->second.GetExtensions() + ") |" +
        it->second.GetExtensions();

      if (wxExMatchesOneOf(filename, it->second.GetExtensions()))
      {
        wildcards = wildcard + "|" + wildcards;
      }
      else
      {
        wildcards = wildcards + "|" + wildcard;
      }
    }
  }

  return wildcards;
}

const wxExLexer wxExLexers::FindByFileName(
  const wxFileName& filename) const
{
  if (!filename.IsOk())
  {
    return wxExLexer();
  }

  for (
    std::map<wxString, wxExLexer>::const_iterator it = m_Lexers.begin();
    it != m_Lexers.end();
    ++it)
  {
    if (wxExMatchesOneOf(filename, it->second.GetExtensions()))
    {
      return it->second;
    }
  }

  return wxExLexer();
}

const wxExLexer wxExLexers::FindByName(const wxString& name) const
{
  std::map<wxString, wxExLexer>::const_iterator it = m_Lexers.find(name);

  if (it != m_Lexers.end())
  {
    return it->second;
  }

  return wxExLexer();
}

const wxExLexer wxExLexers::FindByText(const wxString& text) const
{
  // Add automatic lexers if text starts with some special tokens.
  const wxString text_lowercase = text.Lower();

  if (text_lowercase.StartsWith("<html>") ||
      text_lowercase.StartsWith("<?php"))
  {
    return FindByName("hypertext");
  }
  else if (text_lowercase.StartsWith("<?xml"))
  {
    return FindByName("xml");
  }
  // cpp files like #include <map> really do not have a .h extension (e.g. /usr/include/c++/3.3.5/map)
  // so add here.
  else if (text_lowercase.StartsWith("//"))
  {
    return FindByName("cpp");
  }

  return wxExLexer();
}

wxExLexers* wxExLexers::Get(bool createOnDemand)
{
  if (m_Self == NULL && createOnDemand)
  {
    m_Self = new wxExLexers(wxFileName(
#ifdef wxExUSE_PORTABLE
      wxPathOnly(wxStandardPaths::Get().GetExecutablePath())
#else
      wxStandardPaths::Get().GetUserDataDir()
#endif
      + wxFileName::GetPathSeparator() + "lexers.xml")
    );

    m_Self->Read();
  }

  return m_Self;
}

const wxString wxExLexers::GetLexerExtensions() const
{
  wxString text;

  for (
    std::map<wxString, wxExLexer>::const_iterator it = m_Lexers.begin();
    it != m_Lexers.end();
    ++it)
  {
    if (!it->second.GetExtensions().empty() &&
         it->second.GetExtensions() != "*." + it->second.GetScintillaLexer())
    {
      if (!text.empty())
      {
        text += wxExGetFieldSeparator();
      }

      text += it->second.GetExtensions();
    }
  }

  return text;
}

bool wxExLexers::IndicatorIsLoaded(const wxExIndicator& indic) const
{
  std::set<wxExIndicator>::const_iterator it = m_Indicators.find(indic);
  return (it != m_Indicators.end());
}

bool wxExLexers::MarkerIsLoaded(const wxExMarker& marker) const
{
  std::set<wxExMarker>::const_iterator it = m_Markers.find(marker);
  return (it != m_Markers.end());
}

void wxExLexers::ParseTagGlobal(const wxXmlNode* node)
{
  wxXmlNode* child = node->GetChildren();

  while (child)
  {
    if (child->GetName() == "comment")
    {
      // Ignore comments.
    }
    else if (child->GetName() == "hex")
    {
      m_StylesHex.push_back(wxExStyle(child));
    }
    else if (child->GetName() == "indicator")
    {
      const wxExIndicator indicator (child);

      if (indicator.IsOk())
      {
        m_Indicators.insert(indicator);
      }
    }
    else if (child->GetName() == "marker")
    {
      const wxExMarker marker(child);

      if (marker.IsOk())
      {
        m_Markers.insert(marker);
      }
    }
    else if (child->GetName() == "properties")
    {
      m_GlobalProperties = ParseTagProperties(child);
    }
    else if (child->GetName() == "style")
    {
      const wxExStyle style(child);

      if (style.IsDefault())
      {
        m_DefaultStyle = style;
      }
      else
      {
        m_Styles.push_back(style);
      }
    }
    else
    {
      wxLogError(_("Undefined global tag: %s on line: %d"),
        child->GetName().c_str(), 
        child->GetLineNumber());
    }

    child = child->GetNext();
  }
}

void wxExLexers::ParseTagMacro(const wxXmlNode* node)
{
  wxXmlNode* child = node->GetChildren();

  while (child)
  {
    if (child->GetName() == "comment")
    {
      // Ignore comments.
    }
    else if (child->GetName() == "def")
    {
      const wxString attrib = child->GetAttribute("no", "");
      const wxString content = child->GetNodeContent().Strip(wxString::both);
      const wxString style = child->GetAttribute("style", "");

      if (!attrib.empty())
      {
        std::map<wxString, wxString>::const_iterator it = m_Macros.find(attrib);

        if (it != m_Macros.end())
        {
          wxLogError(_("Macro: %s on line: %d already exists"),
            attrib.c_str(), 
            child->GetLineNumber());
        }
        else
        {
          m_Macros[attrib] = content;
        }
      }

      if (!style.empty())
      {
        std::map<wxString, wxString>::const_iterator it = 
          m_MacrosStyle.find(style);

        if (it != m_MacrosStyle.end())
        {
          wxLogError(_("Macro style: %s on line: %d already exists"),
            style.c_str(), 
            child->GetLineNumber());
        }
        else
        {
          m_MacrosStyle[style] = content;
        }
      }
    }
    else
    {
      wxLogError(_("Undefined macro tag: %s on line: %d"),
        child->GetName().c_str(), 
        child->GetLineNumber());
    }

    child = child->GetNext();
  }
}

const std::vector<wxExProperty> wxExLexers::ParseTagProperties(
  const wxXmlNode* node) const
{
  std::vector<wxExProperty> text;

  wxXmlNode *child = node->GetChildren();

  while (child)
  {
    if (child->GetName() == "property")
    {
      text.push_back(wxExProperty(child));
    }
    else if (child->GetName() == "comment")
    {
      // Ignore comments.
    }
    else
    {
      wxLogError(_("Undefined properties tag: %s on line: %d"),
        child->GetName().c_str(), child->GetLineNumber());
    }

    child = child->GetNext();
  }

  return text;
}

void wxExLexers::Read()
{
  // This test is to prevent showing an error if the lexers file does not exist,
  // as this is not required.
  if (!m_FileName.FileExists())
  {
    return;
  } 

  wxXmlDocument doc;

  if (!doc.Load(m_FileName.GetFullPath()))
  {
    return;
  }

  // Initialize members.
  m_Indicators.clear();
  m_Lexers.clear();
  m_Macros.clear();
  m_MacrosStyle.clear();
  m_Markers.clear();
  m_Styles.clear();
  m_StylesHex.clear();
  m_DefaultStyle = wxExStyle(NULL);
  m_GlobalProperties.clear();

  wxXmlNode* child = doc.GetRoot()->GetChildren();

  while (child)
  {
    if (child->GetName() == "macro")
    {
      ParseTagMacro(child);
    }
    else if (child->GetName() == "global")
    {
      ParseTagGlobal(child);
    }
    else if (child->GetName() == "lexer")
    {
      const wxExLexer lexer(child);
      m_Lexers.insert(std::make_pair(lexer.GetScintillaLexer(), lexer));
    }

    child = child->GetNext();
  }

  if (!wxConfigBase::Get()->Exists(_("In files")))
  {
    wxConfigBase::Get()->Write(_("In files"), GetLexerExtensions());
  }

  if (!wxConfigBase::Get()->Exists(_("Add what")))
  {
    wxConfigBase::Get()->Write(_("Add what"), GetLexerExtensions());
  }
}

wxExLexers* wxExLexers::Set(wxExLexers* lexers)
{
  wxExLexers* old = m_Self;
  m_Self = lexers;
  return old;
}

bool wxExLexers::ShowDialog(
  wxWindow* parent,
  wxString& lexer,
  const wxString& caption) const
{
  wxArrayString s;

  for (
    std::map<wxString, wxExLexer>::const_iterator it = m_Lexers.begin();
    it != m_Lexers.end();
    ++it)
  {
    s.Add(it->first);
  } 

  s.Add(wxEmptyString);

  wxSingleChoiceDialog dlg(
    parent, 
    _("Input") + ":", 
    caption, 
    s);
  
  dlg.SetSelection(s.Index(lexer));

  if (dlg.ShowModal() == wxID_CANCEL)
  {
    return false;
  }

  lexer = FindByName(dlg.GetStringSelection()).GetScintillaLexer();

  return true;
}
