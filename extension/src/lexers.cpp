////////////////////////////////////////////////////////////////////////////////
// Name:      lexers.cpp
// Purpose:   Implementation of wxExLexers class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <algorithm>
#include <functional>
#include <regex>
#include <wx/config.h>
#include <wx/stdpaths.h>
#include <wx/stc/stc.h>
#include <wx/xml/xml.h>
#include <wx/extension/lexers.h>
#include <wx/extension/util.h> // for wxExMatchesOneOf

wxExLexers* wxExLexers::m_Self = nullptr;

// Constructor for lexers from specified filename.
// This must be an existing xml file containing all lexers.
// It does not do LoadDocument, however if you use the global Get,
// it both constructs and loads the lexers.
wxExLexers::wxExLexers(const wxFileName& filename)
  : m_FileName(filename)
  , m_NoTheme(wxEmptyString)
  // Here the default theme is set, and used if the application
  // is run for the first time.
  , m_Theme(wxConfigBase::Get()->Read("theme", "studio"))
{
  m_DefaultColours.clear();
}

// No longer const, as it updates m_DefaultColours.
void wxExLexers::ApplyGlobalStyles(wxStyledTextCtrl* stc)
{
  if (m_DefaultColours.empty())
  {
    m_DefaultColours["caretforeground"] = 
      stc->GetCaretForeground().GetAsString();
      
    m_DefaultColours["caretlinebackground"] = 
      stc->GetCaretLineBackground().GetAsString();
      
    m_DefaultColours["edge"] = 
      stc->GetEdgeColour().GetAsString();
  }
    
  m_DefaultStyle.Apply(stc);

  stc->StyleClearAll();

  for_each (m_Styles.begin(), m_Styles.end(), 
    std::bind2nd(std::mem_fun_ref(&wxExStyle::Apply), stc));

  const auto colour_it = m_ThemeColours.find(m_Theme);
  
  if (colour_it != m_ThemeColours.end())
  {
    for (const auto& it : colour_it->second)
    {
      if (it.first == "caretforeground")
      {
        stc->SetCaretForeground(it.second);
      }
      else if (it.first == "caretlinebackground")
      {
        stc->SetCaretLineBackground(it.second);
      }
      else if (it.first == "selbackground")
      {
        stc->SetSelBackground(true, it.second);
      }
      else if (it.first == "selforeground")
      {
        stc->SetSelForeground(true, it.second);
      }
      else if (it.first == "calltipbackground")
      {
        stc->CallTipSetBackground(wxColour(it.second));
      }
      else if (it.first == "calltipforeground")
      {
        stc->CallTipSetForeground(wxColour(it.second));
      }
      else if (it.first == "edge")
      {
        stc->SetEdgeColour(wxColour(it.second));
      }
    }
  }
  
  ApplyIndicators(stc);
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

const wxString wxExLexers::ApplyMacro(
  const wxString& text, const wxString& lexer)
{
  const auto it = GetMacros(lexer).find(text);
  
  if (it != GetMacros(lexer).end())
  {
    return it->second;
  }
  else
  {
    const auto it = GetThemeMacros().find(text);

    if (it != GetThemeMacros().end())
    {
      return it->second;
    }
  }
  
  return text;
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
  wxString wildcards = 
    _("All Files") + wxString::Format(" (%s)|%s",
      wxFileSelectorDefaultWildcardStr,
      wxFileSelectorDefaultWildcardStr);

  for (const auto& it : m_Lexers)
  {
    if (!it.GetExtensions().empty())
    {
      const wxString wildcard =
        it.GetDisplayLexer() +
        " (" + it.GetExtensions() + ") |" +
        it.GetExtensions();

      if (wxExMatchesOneOf(filename, it.GetExtensions()))
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
  for (const auto& it : m_Lexers)
  {
    if (
      !it.GetExtensions().empty() && 
       wxExMatchesOneOf(filename, it.GetExtensions()))
    {
      return it;
    }
  }

  return wxExLexer();
}

const wxExLexer wxExLexers::FindByName(const wxString& name) const
{
  for (const auto& it : m_Lexers)
  {
    if (it.GetDisplayLexer() == name)
    {
      return it;
    }
  }
  
  return wxExLexer();
}

const wxExLexer wxExLexers::FindByText(const wxString& text) const
{
  // Add automatic lexers if text starts with some special tokens.
  const wxString text_lowercase = text.Lower().Trim();

  if (text_lowercase.StartsWith("<html>"))
  {
    return FindByName("hypertext");
  }
  else if (text_lowercase.StartsWith("<?xml"))
  {
    return FindByName("xml");
  }
  // cpp files like #include <map> really do not have a .h extension 
  // (e.g. /usr/include/c++/3.3.5/map) so add here.
  else if (text_lowercase.StartsWith("//"))
  {
    return FindByName("cpp");
  }
  else if (text_lowercase.StartsWith("<?php"))
  {
    return FindByName("phpscript");
  }
  else if (text.StartsWith("#!/bin/csh"))
  {
    return FindByName("csh");
  }
  else if (text.StartsWith("#!/bin/tcsh"))
  {
    return FindByName("tcsh");
  }
  else if (text.StartsWith("#!/bin/sh"))
  {
    return FindByName("sh");
  }
  else
  {
    // If there is another Shell Language Indicator,
    // match with bash.
    std::regex re("#!.*/bin/.*");
    
    if (std::regex_match(text_lowercase.ToStdString(), re))
    {
      return FindByName("bash");
    }
  }

  return wxExLexer();
}

wxExLexers* wxExLexers::Get(bool createOnDemand)
{
  if (m_Self == nullptr && createOnDemand)
  {
    m_Self = new wxExLexers(wxFileName(
#ifdef wxExUSE_PORTABLE
      wxPathOnly(wxStandardPaths::Get().GetExecutablePath())
#else
      wxStandardPaths::Get().GetUserDataDir()
#endif
      + wxFileName::GetPathSeparator() + "lexers.xml")
    );

    m_Self->LoadDocument();
  }

  return m_Self;
}

const wxString wxExLexers::GetKeywords(const wxString& set) const
{
  const auto it = m_Keywords.find(set);
  
  if (it != m_Keywords.end())
  {
    return it->second;
  }
  else
  {
    return wxEmptyString;
  }
}

const wxString wxExLexers::GetLexerExtensions() const
{
  wxString text = wxFileSelectorDefaultWildcardStr;

  for (const auto& it : m_Lexers)
  {
    if (!it.GetExtensions().empty())
    {
      if (!text.empty())
      {
        text += wxExGetFieldSeparator();
      }

      text += it.GetExtensions();
    }
  }

  return text;
}

bool wxExLexers::IndicatorIsLoaded(const wxExIndicator& indic) const
{
  return m_Indicators.find(indic) != m_Indicators.end();
}

void wxExLexers::Initialize()
{
  m_DefaultStyle = wxExStyle();
  m_ThemeColours.clear();
  m_GlobalProperties.clear();
  m_Indicators.clear();
  m_Keywords.clear();
  m_Lexers.clear();
  m_Macros.clear();
  m_ThemeMacros.clear();
  m_Markers.clear();
  m_Styles.clear();
  m_StylesHex.clear();
  m_ThemeColours[m_NoTheme] = m_DefaultColours;
  const std::map<wxString, wxString> empty_map;
  m_ThemeMacros[m_NoTheme] = empty_map;  
}

bool wxExLexers::LoadDocument()
{
  // This test is to prevent showing an error if the lexers file does not exist,
  // as this is not required.
  if (!m_FileName.FileExists())
  {
    return false;
  } 

  wxXmlDocument doc;

  if (!doc.Load(m_FileName.GetFullPath()))
  {
    return false;
  }
  
  Initialize();
  
  // Even if no theme is chosen,
  // read all lexers, to be able to select other themes again.

  wxXmlNode* child = doc.GetRoot()->GetChildren();

  while (child)
  {
    if (child->GetName() == "macro")
    {
      ParseNodeMacro(child);
    }
    else if (child->GetName() == "global")
    {
      ParseNodeGlobal(child);
    }
    else if (child->GetName() == "keyword")
    {
      ParseNodeKeyword(child);
    }
    else if (child->GetName() == "lexer")
    {
      const wxExLexer lexer(child);
      
      if (lexer.IsOk())
      {
        m_Lexers.emplace_back(lexer);
      }
    }

    child = child->GetNext();
  }

  // Check config, but do not create one.
  wxConfigBase* config = wxConfigBase::Get(false);

  if (config != nullptr)
  {
    if (!config->Exists(_("Add what")))
    {
      config->Write(_("Add what"), GetLexerExtensions());
    }
  
    if (!config->Exists(_("In files")))
    {
      config->Write(_("In files"), GetLexerExtensions());
    }

    if (!config->Exists(_("In folder")))
    {
      config->Write(_("In folder"), wxGetHomeDir());
    }
  }
  
  // Do some theme checking.
  if (m_ThemeMacros.size() <= 1) // NoTheme is always present
  {
    wxLogError("Themes are missing");
    return false;
  }

  for (const auto& it : m_ThemeMacros)
  {
    if (!it.first.empty() && it.second.empty())
    {
      wxLogError("Theme %s is unknown", it.first);
      return false;
    }
  } 

  return true;
}

bool wxExLexers::MarkerIsLoaded(const wxExMarker& marker) const
{
  return m_Markers.find(marker) != m_Markers.end();
}

void wxExLexers::ParseNodeGlobal(const wxXmlNode* node)
{
  wxXmlNode* child = node->GetChildren();

  while (child)
  {
    if (m_Theme == m_NoTheme)
    {
      // Do nothing.
    }
    else if (child->GetName() == "hex")
    {
      m_StylesHex.emplace_back(wxExStyle(child, "global"));
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
      wxExNodeProperties(child, m_GlobalProperties);
    }
    else if (child->GetName() == "style")
    {
      const wxExStyle style(child, "global");

      if (style.ContainsDefaultStyle())
      {
        if (m_DefaultStyle.IsOk())
        {
          wxLogError("Duplicate default style: %s on line: %d",
            child->GetName().c_str(), 
            child->GetLineNumber());
        }
        else
        {
          m_DefaultStyle = style;
        }
      }
      else
      {
        m_Styles.emplace_back(style);
      }
    }
    
    child = child->GetNext();
  }
}

void wxExLexers::ParseNodeKeyword(const wxXmlNode* node)
{
  wxXmlNode* child = node->GetChildren();

  while (child)
  {
    if (child->GetName() == "set")
    {
      const wxString name(child->GetAttribute("name"));
      const wxString content(child->GetNodeContent().Strip(wxString::both));
      m_Keywords[name] = content;
    }
    
    child = child->GetNext();
  }
}

void wxExLexers::ParseNodeMacro(const wxXmlNode* node)
{
  wxXmlNode* child = node->GetChildren();

  while (child)
  {
    if (child->GetName() == "def")
    {
      const wxString name = child->GetAttribute("name");
    
      wxXmlNode* macro = child->GetChildren();
      
      std::map <wxString, wxString> macro_map;

      long val = 0;
  
      while (macro)
      {
        const wxString attrib = macro->GetAttribute("no");
        const wxString content = macro->GetNodeContent().Strip(wxString::both);

        if (!attrib.empty())
        {
          const auto it = macro_map.find(attrib);

          if (it != macro_map.end())
          {
            wxLogError("Macro: %s on line: %d already exists",
              attrib.c_str(), 
              macro->GetLineNumber());
          }
          else
          {
            if (!content.empty())
            {
              if (!content.ToLong(&val))
              {
                wxLogError("Macro: %s on line: %d is not a number",
                  attrib.c_str(), 
                  macro->GetLineNumber());
              }
              else
              {
                macro_map[attrib] = content;
                val++;
              }
            }
            else
            {
              macro_map[attrib] = wxString::Format("%ld", val);
              val++;
            }
          }
        }
        
        macro = macro->GetNext();
      }
      
      m_Macros[name] = macro_map;      
    }
    else if (child->GetName() == "themes")
    {
      ParseNodeThemes(child);
    }
    
    child = child->GetNext();
  }
}

void wxExLexers::ParseNodeTheme(const wxXmlNode* node)
{
  std::map<wxString, wxString> tmpColours;
  std::map<wxString, wxString> tmpMacros;
  
  wxXmlNode *child = node->GetChildren();
  
  while (child)
  {
    const wxString content = child->GetNodeContent().Strip(wxString::both);
      
    if (child->GetName() == "def")
    {
      const wxString style = child->GetAttribute("style");
      
      if (!style.empty())
      {
        auto it = tmpMacros.find(style);

        if (it != tmpMacros.end())
        {
          wxLogError("Macro style: %s on line: %d already exists",
            style.c_str(), 
            child->GetLineNumber());
        }
        else
        {
          tmpMacros[style] = content;
        }
      }
    }
    else if (child->GetName() == "colour")
    {
      tmpColours[child->GetAttribute("name", "0")] = ApplyMacro(content);
    }
    
    child = child->GetNext();
  }
  
  m_ThemeColours[node->GetAttribute("name")] = tmpColours;
  m_ThemeMacros[node->GetAttribute("name")] = tmpMacros;
}

void wxExLexers::ParseNodeThemes(const wxXmlNode* node)
{
  wxXmlNode *child = node->GetChildren();

  while (child)
  {
    if (child->GetName() == "theme")
    {
      ParseNodeTheme(child);
    }
    
    child = child->GetNext();
  }
}
      
void wxExLexers::RestoreTheme()
{
  m_Theme = wxConfigBase::Get()->Read("theme", "studio");
}
  
wxExLexers* wxExLexers::Set(wxExLexers* lexers)
{
  wxExLexers* old = m_Self;
  m_Self = lexers;
  return old;
}

bool wxExLexers::SetTheme(const wxString& theme)
{
  const auto it = m_ThemeColours.find(theme);
  
  if (it != m_ThemeColours.end())
  {
    m_Theme = theme;
    wxConfigBase::Get()->Write("theme", m_Theme);  
    return true;
  }

  return false;  
}

void wxExLexers::SetThemeNone()
{
  m_Theme = m_NoTheme;
}

bool wxExLexers::ShowDialog(
  wxWindow* parent,
  wxString& lexer,
  const wxString& caption,
  bool show_modal) const
{
  wxArrayString s;

  for (const auto& it : m_Lexers)
  {
    s.Add(it.GetDisplayLexer());
  } 

  s.Add(wxEmptyString);

  wxSingleChoiceDialog dlg(
    parent, 
    _("Input") + ":", 
    caption, 
    s);

  const int index = s.Index(lexer);
  
  if (index != wxNOT_FOUND)
  {
    dlg.SetSelection(index);
  }

  if (show_modal && dlg.ShowModal() == wxID_CANCEL)
  {
    return false;
  }

  lexer = dlg.GetStringSelection();

  return true;
}

bool wxExLexers::ShowThemeDialog(
  wxWindow* parent, 
  const wxString& caption,
  bool show_modal)
{
  if (m_ThemeMacros.size() <= 1) // NoTheme is always present
  {
    return false;
  }

  wxArrayString choices;

  for (const auto& it : m_ThemeMacros)
  {
    choices.Add(it.first);
  }

  wxSingleChoiceDialog dlg(
    parent,
    _("Input") + ":", 
    caption,
    choices);

  const int index = choices.Index(m_Theme);

  if (index != wxNOT_FOUND)
  {
    dlg.SetSelection(index);
  }

  if (show_modal)
  {
    if (dlg.ShowModal() == wxID_CANCEL)
    {
      return false;
    }
  
    SetTheme(dlg.GetStringSelection());
  }

  return LoadDocument();
}
