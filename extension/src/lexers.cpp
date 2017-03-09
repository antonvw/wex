////////////////////////////////////////////////////////////////////////////////
// Name:      lexers.cpp
// Purpose:   Implementation of wxExLexers class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
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
#include <wx/extension/lexers.h>
#include <wx/extension/stc.h>
#include <wx/extension/tokenizer.h>
#include <wx/extension/util.h> // for wxExMatchesOneOf

wxExLexers* wxExLexers::m_Self = nullptr;

// Constructor for lexers from specified filename.
// This must be an existing xml file containing all lexers.
// It does not do LoadDocument, however if you use the global Get,
// it both constructs and loads the lexers.
wxExLexers::wxExLexers(const wxExFileName& filename)
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

  if (!m_FoldingBackgroundColour.empty())
  {
    const wxString col(m_FoldingBackgroundColour);
    stc->SetFoldMarginHiColour(true, col);
  }

  if (!m_FoldingForegroundColour.empty())
  {
    const wxString col((m_FoldingForegroundColour));
    stc->SetFoldMarginColour(true, col);
  }

  const auto& colour_it = m_ThemeColours.find(m_Theme);
  
  if (colour_it != m_ThemeColours.end())
  {
    for (const auto& it : colour_it->second)
    {
           if (it.first == "caretforeground") stc->SetCaretForeground(wxString(it.second));
      else if (it.first == "caretlinebackground") stc->SetCaretLineBackground(wxString(it.second));
      else if (it.first == "selbackground") stc->SetSelBackground(true, wxString(it.second));
      else if (it.first == "selforeground") stc->SetSelForeground(true, wxString(it.second));
      else if (it.first == "calltipbackground") stc->CallTipSetBackground(wxString(it.second));
      else if (it.first == "calltipforeground") stc->CallTipSetForeground(wxString(it.second));
      else if (it.first == "edge") stc->SetEdgeColour(wxString(it.second));
    }
  }
}

const std::string wxExLexers::ApplyMacro(const std::string& text, const std::string& lexer)
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

const wxExLexer wxExLexers::FindByFileName(const std::string& fullname) const
{
  const auto& it = std::find_if(m_Lexers.begin(), m_Lexers.end(), 
    [fullname](auto const& e) {return !e.GetExtensions().empty() && 
       wxExMatchesOneOf(fullname, e.GetExtensions());});
  return it != m_Lexers.end() ? *it: wxExLexer();
}

const wxExLexer wxExLexers::FindByName(const std::string& name) const
{
  const auto& it = std::find_if(m_Lexers.begin(), m_Lexers.end(), 
    [name](auto const& e) {return e.GetDisplayLexer() == name;});
  return it != m_Lexers.end() ? *it: wxExLexer();
}

const wxExLexer wxExLexers::FindByText(const std::string& text) const
{
  // Add automatic lexers if text starts with some special tokens.
  std::string text_lowercase = std::regex_replace(text, 
    std::regex("[ \t\n\v\f\r]+$"), "", std::regex_constants::format_sed);
  for (auto & c : text_lowercase) c = ::tolower(c);

       if (text_lowercase.find("<html>") == 0) return FindByName("hypertext");
  else if (text_lowercase.find("<?xml") == 0) return FindByName("xml");
  // cpp files like #include <map> really do not have a .h extension 
  // (e.g. /usr/include/c++/3.3.5/map) so add here.
  else if (text_lowercase.find("//") == 0) return FindByName("cpp");
  else if (text_lowercase.find("<?php") == 0) return FindByName("phpscript");
  else if (text_lowercase.find("#!/bin/csh") == 0) return FindByName("csh");
  else if (text_lowercase.find("#!/bin/env python") == 0) return FindByName("python");
  else if (text_lowercase.find("#!/bin/tcsh") == 0) return FindByName("tcsh");
  else if (text_lowercase.find("#!/bin/sh") == 0) return FindByName("sh");
  else
  {
    // If there is another Shell Language Indicator,
    // match with bash.
    std::regex re("#!.*/bin/.*");
    if (std::regex_match(text_lowercase, re)) return FindByName("bash");
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

const std::string wxExLexers::GetKeywords(const std::string& set) const
{
  const auto& it = m_Keywords.find(set);
  return (it != m_Keywords.end() ? it->second: std::string());
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
  m_ThemeMacros[m_NoTheme] = std::map<std::string, std::string>{};  
}

bool wxExLexers::LoadDocument()
{
  // This test is to prevent showing an error if the lexers file does not exist,
  // as this is not required.
  if (!m_FileName.FileExists()) return false;
  
  pugi::xml_document doc;
  const pugi::xml_parse_result result = doc.load_file(
    m_FileName.GetFullPath().c_str(),
    pugi::parse_default | pugi::parse_trim_pcdata);

  if (!result)
  {
    wxExXmlError(m_FileName, &result);
    return false;
  }
  
  Initialize();
  
  for (const auto& node: doc.document_element().children())
  {
         if (strcmp(node.name(), "macro") == 0) ParseNodeMacro(node);
    else if (strcmp(node.name(), "global") == 0) ParseNodeGlobal(node);
    else if (strcmp(node.name(), "keyword")== 0) ParseNodeKeyword(node);
    else if (strcmp(node.name(), "lexer") ==  0) 
    {
      const wxExLexer lexer(&node);
      if (lexer.IsOk()) m_Lexers.emplace_back(lexer);
    }
  }

  // Check config, but do not create one.
  wxConfigBase* config = wxConfigBase::Get(false);

  if (config != nullptr)
  {
    const std::string extensions(std::accumulate(
      m_Lexers.begin(), m_Lexers.end(), std::string(wxFileSelectorDefaultWildcardStr), 
      [&](const std::string& a, const wxExLexer& b) {
        if (!b.GetExtensions().empty())
          return a.empty() ? b.GetExtensions() : a + wxExGetFieldSeparator() + b.GetExtensions();
        else return a;}));
    if (!config->Exists(_("Add what"))) config->Write(_("Add what"), wxString(extensions));
    if (!config->Exists(_("In files"))) config->Write(_("In files"), wxString(extensions));
    if (!config->Exists(_("In folder"))) config->Write(_("In folder"), wxGetHomeDir());
  }
  
  // Do some checking.
  if (!m_Lexers.empty())
  {
    if (!m_DefaultStyle.IsOk()) std::cerr << "Default style not ok" << "\n";
    if (!m_DefaultStyle.ContainsDefaultStyle()) std::cerr << "Default style does not contain default style" << "\n";
  }
  
  if (m_ThemeMacros.size() <= 1) // NoTheme is always present
  {
    std::cerr << "Themes are missing\n";
  }

  for (const auto& it : m_ThemeMacros)
  {
    if (!it.first.empty() && it.second.empty())
    {
      std::cerr << "Theme " << it.first << " is unknown\n";
    }
  } 

  return true;
}

void wxExLexers::ParseNodeFolding(const pugi::xml_node& node)
{
  wxExTokenizer fields(node.text().get(), ",");

  m_FoldingBackgroundColour = ApplyMacro(fields.GetNextToken());
  m_FoldingForegroundColour = ApplyMacro(fields.GetNextToken());
}

void wxExLexers::ParseNodeGlobal(const pugi::xml_node& node)
{
  for (const auto& child: node.children())
  {
    if (m_Theme == m_NoTheme)
    {
      // Do nothing.
    }
    else if (strcmp(child.name(), "foldmargin") == 0)
    {
      ParseNodeFolding(child);
    }
    else if (strcmp(child.name(), "hex") == 0)
    {
      m_StylesHex.emplace_back(wxExStyle(child, "global"));
    }
    else if (strcmp(child.name(), "indicator") == 0)
    {
      const wxExIndicator indicator(child);

      if (indicator.IsOk())
      {
        m_Indicators.insert(indicator);
      }
    }
    else if (strcmp(child.name(), "marker") == 0)
    {
      const wxExMarker marker(child);
      if (marker.IsOk()) m_Markers.insert(marker);
    }
    else if (strcmp(child.name(), "properties") == 0)
    {
      wxExNodeProperties(&child, m_GlobalProperties);
    }
    else if (strcmp(child.name(), "style") == 0)
    {
      const wxExStyle style(child, "global");

      if (style.ContainsDefaultStyle())
      {
        if (m_DefaultStyle.IsOk())
        {
          std::cerr << "Duplicate default style: " << child.name() << " with offset: "
            << child.offset_debug() << "\n";
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
  }
}

void wxExLexers::ParseNodeKeyword(const pugi::xml_node& node)
{
  for (const auto& child: node.children())
  {
    m_Keywords[child.attribute("name").value()] = child.text().get();
  }
}

void wxExLexers::ParseNodeMacro(const pugi::xml_node& node)
{
  for (const auto& child: node.children())
  {
    if (strcmp(child.name(), "def") == 0)
    {
      const std::string name = child.attribute("name").value();
    
      std::map <std::string, std::string> macro_map;

      int val = 0;

      for (const auto& macro: child.children())
      {
        const std::string attrib = macro.attribute("no").value();
        const std::string content = macro.text().get();

        if (!attrib.empty())
        {
          const auto& it = macro_map.find(attrib);

          if (it != macro_map.end())
          {
            std::cerr << "Macro: " << attrib << " with offset: " << 
              macro.offset_debug() << " already exists\n";
          }
          else
          {
            if (!content.empty())
            {
              try
              {
                val = std::stoi(content) + 1;
                macro_map[attrib] = content;
              }
              catch (std::exception& e)
              {
                std::cerr << "Macro excption: " << e.what() << "\n";
              }
            }
            else
            {
              macro_map[attrib] = std::to_string(val);
              val++;
            }
          }
        }
      }
      
      m_Macros[name] = macro_map;      
    }
    else if (strcmp(child.name(), "themes") == 0)
    {
      ParseNodeThemes(child);
    }
  }
}

void wxExLexers::ParseNodeTheme(const pugi::xml_node& node)
{
  std::map<std::string, std::string> tmpColours;
  std::map<std::string, std::string> tmpMacros;
  
  for (const auto& child: node.children())
  {
    const std::string content = child.text().get();
      
    if (strcmp(child.name(), "def") == 0)
    {
      const std::string style = child.attribute("style").value();
      
      if (!style.empty())
      {
        const auto& it = tmpMacros.find(style);

        if (it != tmpMacros.end())
        {
          std::cerr << "Macro style: " <<  style << " with offset: " << child.offset_debug() << " already exists";
        }
        else
        {
          tmpMacros[style] = content;
        }
      }
    }
    else if (strcmp(child.name(), "colour") == 0)
    {
      tmpColours[child.attribute("name").value()] = ApplyMacro(content);
    }
  }
  
  m_ThemeColours[node.attribute("name").value()] = tmpColours;
  m_ThemeMacros[node.attribute("name").value()] = tmpMacros;
}

void wxExLexers::ParseNodeThemes(const pugi::xml_node& node)
{
  for (const auto& child: node.children()) ParseNodeTheme(child);
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
  const wxArrayString& s, std::string& selection)
{
  wxSingleChoiceDialog dlg(parent, _("Input") + ":", caption, s);

  const int index = s.Index(selection);
  if (index != wxNOT_FOUND) dlg.SetSelection(index);
  if (dlg.ShowModal() == wxID_CANCEL) return false;

  selection = dlg.GetStringSelection();
  
  return true;
}
  
bool wxExLexers::ShowDialog(wxExSTC* stc, const wxString& caption) const
{
  wxArrayString s;
  for (const auto& it : m_Lexers) s.Add(it.GetDisplayLexer());
  s.Add(wxEmptyString);

  auto lexer = stc->GetLexer().GetDisplayLexer();
  if (!SingleChoice(stc, caption, s, lexer)) return false;

  lexer.empty() ? 
    stc->GetLexer().Reset():
    stc->GetLexer().Set(lexer, true); // allow fold
  
  return true;
}

bool wxExLexers::ShowThemeDialog(wxWindow* parent, const wxString& caption)
{ 
  wxArrayString s;
  for (const auto& it : m_ThemeMacros) s.Add(it.first);

  if (!SingleChoice(parent, caption, s, m_Theme)) return false;

  wxConfigBase::Get()->Write("theme", wxString(m_Theme));

  return LoadDocument();
}
