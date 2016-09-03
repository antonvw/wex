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
#include <numeric>
#include <regex>
#include <wx/config.h>
#include <wx/stdpaths.h>
#include <wx/xml/xml.h>
#include <wx/extension/lexers.h>
#include <wx/extension/stc.h>
#include <wx/extension/util.h> // for wxExMatchesOneOf

wxExLexers* wxExLexers::m_Self = nullptr;

// Constructor for lexers from specified filename.
// This must be an existing xml file containing all lexers.
// It does not do LoadDocument, however if you use the global Get,
// it both constructs and loads the lexers.
wxExLexers::wxExLexers(const wxFileName& filename)
  : m_FileName(filename)
  // Here the default theme is set, and used if the application
  // is run for the first time.
  , m_Theme(wxConfigBase::Get()->Read("theme", "studio"))
{
}

void wxExLexers::Apply(wxExSTC* stc) const
{
  m_DefaultStyle.Apply(stc);

  for_each(m_Indicators.begin(), m_Indicators.end(), std::bind2nd(std::mem_fun_ref(&wxExIndicator::Apply), stc));
  for_each(m_GlobalProperties.begin(), m_GlobalProperties.end(), std::bind2nd(std::mem_fun_ref(&wxExProperty::Apply), stc));
  for_each(m_Markers.begin(), m_Markers.end(), std::bind2nd(std::mem_fun_ref(&wxExMarker::Apply), stc));
  
  if (stc->GetHexMode().Active())
  {
    for_each(m_StylesHex.begin(), m_StylesHex.end(), std::bind2nd(std::mem_fun_ref(&wxExStyle::Apply), stc));
  }
}
  
// No longer const, as it updates m_DefaultColours.
void wxExLexers::ApplyGlobalStyles(wxExSTC* stc)
{
  if (m_DefaultColours.empty())
  {
    m_DefaultColours["caretforeground"] = stc->GetCaretForeground().GetAsString();
    m_DefaultColours["caretlinebackground"] = stc->GetCaretLineBackground().GetAsString();
    m_DefaultColours["edge"] = stc->GetEdgeColour().GetAsString();
  }
    
  m_DefaultStyle.Apply(stc);

  stc->StyleClearAll();

  for_each(m_Styles.begin(), m_Styles.end(), std::bind2nd(std::mem_fun_ref(&wxExStyle::Apply), stc));

  const auto& colour_it = m_ThemeColours.find(m_Theme);
  
  if (colour_it != m_ThemeColours.end())
  {
    for (const auto& it : colour_it->second)
    {
           if (it.first == "caretforeground") stc->SetCaretForeground(it.second);
      else if (it.first == "caretlinebackground") stc->SetCaretLineBackground(it.second);
      else if (it.first == "selbackground") stc->SetSelBackground(true, it.second);
      else if (it.first == "selforeground") stc->SetSelForeground(true, it.second);
      else if (it.first == "calltipbackground") stc->CallTipSetBackground(wxColour(it.second));
      else if (it.first == "calltipforeground") stc->CallTipSetForeground(wxColour(it.second));
      else if (it.first == "edge") stc->SetEdgeColour(wxColour(it.second));
    }
  }
}

const wxString wxExLexers::ApplyMacro(const wxString& text, const wxString& lexer)
{
  const auto& it = GetMacros(lexer).find(text);
  
  if (it != GetMacros(lexer).end())
  {
    return it->second;
  }
  else
  {
    const auto& it = GetThemeMacros().find(text);
    if (it != GetThemeMacros().end()) return it->second;
  }
  
  return text;
}

const wxExLexer wxExLexers::FindByFileName(const wxFileName& filename) const
{
  const auto& it = std::find_if(m_Lexers.begin(), m_Lexers.end(), 
    [filename](auto const& e) {return !e.GetExtensions().empty() && wxExMatchesOneOf(filename, e.GetExtensions());});
  return it != m_Lexers.end() ? *it: wxExLexer();
}

const wxExLexer wxExLexers::FindByName(const wxString& name) const
{
  const auto& it = std::find_if(m_Lexers.begin(), m_Lexers.end(), 
    [name](auto const& e) {return e.GetDisplayLexer() == name;});
  return it != m_Lexers.end() ? *it: wxExLexer();
}

const wxExLexer wxExLexers::FindByText(const wxString& text) const
{
  // Add automatic lexers if text starts with some special tokens.
  const wxString text_lowercase = text.Lower().Trim();

       if (text_lowercase.StartsWith("<html>")) return FindByName("hypertext");
  else if (text_lowercase.StartsWith("<?xml")) return FindByName("xml");
  // cpp files like #include <map> really do not have a .h extension 
  // (e.g. /usr/include/c++/3.3.5/map) so add here.
  else if (text_lowercase.StartsWith("//")) return FindByName("cpp");
  else if (text_lowercase.StartsWith("<?php")) return FindByName("phpscript");
  else if (text.StartsWith("#!/bin/csh")) return FindByName("csh");
  else if (text.StartsWith("#!/bin/tcsh")) return FindByName("tcsh");
  else if (text.StartsWith("#!/bin/sh")) return FindByName("sh");
  else
  {
    // If there is another Shell Language Indicator,
    // match with bash.
    std::regex re("#!.*/bin/.*");
    if (std::regex_match(text_lowercase.ToStdString(), re)) return FindByName("bash");
  }

  return wxExLexer();
}

wxExLexers* wxExLexers::Get(bool createOnDemand)
{
  if (m_Self == nullptr && createOnDemand)
  {
    m_Self = new wxExLexers(wxFileName(wxExConfigDir(), "lexers.xml"));
    m_Self->LoadDocument();
  }

  return m_Self;
}

const wxExIndicator wxExLexers::GetIndicator(const wxExIndicator& indicator) const
{
  const auto it = m_Indicators.find(indicator);
  return (it != m_Indicators.end() ? *it: wxExIndicator());
}

const wxString wxExLexers::GetKeywords(const wxString& set) const
{
  const auto& it = m_Keywords.find(set);
  return (it != m_Keywords.end() ? it->second: wxString());
}

const wxExMarker wxExLexers::GetMarker(const wxExMarker& marker) const
{
  const auto it = m_Markers.find(marker);
  return (it != m_Markers.end() ? *it: wxExMarker());
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
  if (!m_FileName.FileExists()) return false;

  wxXmlDocument doc;

  if (!doc.Load(m_FileName.GetFullPath())) return false;
  
  Initialize();
  
  // Even if no theme is chosen,
  // read all lexers, to be able to select other themes again.

  wxXmlNode* child = doc.GetRoot()->GetChildren();

  while (child)
  {
         if (child->GetName() == "macro") ParseNodeMacro(child);
    else if (child->GetName() == "global") ParseNodeGlobal(child);
    else if (child->GetName() == "keyword") ParseNodeKeyword(child);
    else if (child->GetName() == "lexer")
    {
      const wxExLexer lexer(child);
      if (lexer.IsOk()) m_Lexers.emplace_back(lexer);
    }

    child = child->GetNext();
  }

  // Check config, but do not create one.
  wxConfigBase* config = wxConfigBase::Get(false);

  if (config != nullptr)
  {
    const std::string extensions(std::accumulate(
      m_Lexers.begin(), m_Lexers.end(), std::string(wxFileSelectorDefaultWildcardStr), 
      [&](const wxString& a, const wxExLexer& b) {
        if (!b.GetExtensions().empty())
          return a.empty() ? b.GetExtensions() : a + wxExGetFieldSeparator() + b.GetExtensions();
        else return a;}));
    if (!config->Exists(_("Add what"))) config->Write(_("Add what"), wxString(extensions));
    if (!config->Exists(_("In files"))) config->Write(_("In files"), wxString(extensions));
    if (!config->Exists(_("In folder"))) config->Write(_("In folder"), wxGetHomeDir());
  }
  
  if (!m_Lexers.empty())
  {
    if (!m_DefaultStyle.IsOk()) wxLogError("Default style not ok");
    if (!m_DefaultStyle.ContainsDefaultStyle()) wxLogError("Default style does not contain default style");
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
          const auto& it = macro_map.find(attrib);

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
        const auto& it = tmpMacros.find(style);

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
    if (child->GetName() == "theme") ParseNodeTheme(child);
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

bool SingleChoice(wxWindow* parent, const wxString& caption, 
  bool show_modal, const wxArrayString& s, wxString& selection)
{
  wxSingleChoiceDialog dlg(parent, _("Input") + ":", caption, s);

  const int index = s.Index(selection);
  if (index != wxNOT_FOUND) dlg.SetSelection(index);
  if (show_modal && dlg.ShowModal() == wxID_CANCEL) return false;

  selection = dlg.GetStringSelection();
  
  return true;
}
  
bool wxExLexers::ShowDialog(wxExSTC* stc, const wxString& caption, bool show_modal) const
{
  wxArrayString s;
  for (const auto& it : m_Lexers) s.Add(it.GetDisplayLexer());
  s.Add(wxEmptyString);

  wxString lexer = stc->GetLexer().GetDisplayLexer();
  if (!SingleChoice(stc, caption, show_modal, s, lexer)) return false;

  lexer.empty() ? 
    stc->GetLexer().Reset():
    stc->GetLexer().Set(lexer, true); // allow fold
  
  return true;
}

bool wxExLexers::ShowThemeDialog(wxWindow* parent, const wxString& caption, bool show_modal)
{ 
  if (!show_modal) return false;
  
  wxArrayString s;
  for (const auto& it : m_ThemeMacros) s.Add(it.first);

  if (!SingleChoice(parent, caption, show_modal, s, m_Theme)) return false;

  wxConfigBase::Get()->Write("theme", m_Theme);  

  return LoadDocument();
}
