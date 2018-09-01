////////////////////////////////////////////////////////////////////////////////
// Name:      lexers.cpp
// Purpose:   Implementation of wxExLexers class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
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
#include <wx/extension/lexers.h>
#include <wx/extension/log.h>
#include <wx/extension/stc.h>
#include <wx/extension/tokenizer.h>
#include <wx/extension/util.h> // for wxExMatchesOneOf
#include <easylogging++.h>

// Constructor for lexers from specified filename.
// This must be an existing xml file containing all lexers.
// It does not do LoadDocument, however if you use the global Get,
// it both constructs and loads the lexers.
wxExLexers::wxExLexers(const wxExPath& filename)
  : m_Path(filename)
  , m_Theme(wxConfigBase::Get()->Read("theme", std::string()))
{
}

void wxExLexers::Apply(wxExSTC* stc) const
{
  m_DefaultStyle.Apply(stc);

  for (const auto& i : m_Indicators) i.Apply(stc);
  for (const auto& p : m_GlobalProperties) p.Apply(stc);
  for (const auto& m : m_Markers) m.Apply(stc);
  
  if (stc->GetHexMode().Active())
  {
    for (const auto& s : m_StylesHex) s.Apply(stc);
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

  for (const auto& s : m_Styles) s.Apply(stc);

  if (!m_FoldingBackgroundColour.empty())
  {
    stc->SetFoldMarginHiColour(true, m_FoldingBackgroundColour.c_str());
  }
  else
  {
    // See ViewStyle.cxx foldmarginColour
    stc->SetFoldMarginHiColour(true, wxColour(0xc0, 0xc0, 0xc0));
  }

  if (!m_FoldingForegroundColour.empty())
  {
    stc->SetFoldMarginColour(true, m_FoldingForegroundColour.c_str());
  }
  else
  {
    stc->SetFoldMarginColour(true, wxColour(0xff, 0, 0));
  }

  if (const auto& colour_it = m_ThemeColours.find(m_Theme);
    colour_it != m_ThemeColours.end())
  {
    for (const auto& it : colour_it->second)
    {
           if (it.first == "caretforeground") stc->SetCaretForeground(it.second.c_str());
      else if (it.first == "caretlinebackground") stc->SetCaretLineBackground(it.second.c_str());
      else if (it.first == "selbackground") stc->SetSelBackground(true, it.second.c_str());
      else if (it.first == "selforeground") stc->SetSelForeground(true, it.second.c_str());
      else if (it.first == "calltipbackground") stc->CallTipSetBackground(it.second.c_str());
      else if (it.first == "calltipforeground") stc->CallTipSetForeground(it.second.c_str());
      else if (it.first == "edge") stc->SetEdgeColour(it.second.c_str());
    }
  }
}

const std::string wxExLexers::ApplyMacro(const std::string& text, const std::string& lexer)
{
  if (const auto& it = GetMacros(lexer).find(text);
    it != GetMacros(lexer).end())
    return it->second;
  else if (const auto& it = GetThemeMacros().find(text);
    it != GetThemeMacros().end()) 
    return it->second;
  else 
    return text;
}

void wxExLexers::ApplyMarginTextStyle(wxExSTC* stc, int line) const
{
  if (m_StyleNoTextMargin != -1)
  {
    stc->MarginSetStyle(line, m_StyleNoTextMargin);
  }
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
  try
  {
    const std::string filtered(std::regex_replace(text, 
      std::regex("[ \t\n\v\f\r]+$"), "", std::regex_constants::format_sed));

    for (const auto& t : m_Texts) 
    {
      if (std::regex_search(filtered, std::regex(t.second)))
        return FindByName(t.first);
    }
  }
  catch (std::exception& e)
  {
    wxExLog(e) << "findbytext";
  }

  return wxExLexer();
}

wxExLexers* wxExLexers::Get(bool createOnDemand)
{
  if (m_Self == nullptr && createOnDemand)
  {
    m_Self = new wxExLexers(wxExPath(wxExConfigDir(), "lexers.xml"));
    m_Self->LoadDocument();
  }

  return m_Self;
}

const wxExIndicator wxExLexers::GetIndicator(const wxExIndicator& indicator) const
{
  const auto& it = m_Indicators.find(indicator);
  return (it != m_Indicators.end() ? *it: wxExIndicator());
}

const std::string wxExLexers::GetKeywords(const std::string& set) const
{
  const auto& it = m_Keywords.find(set);
  return (it != m_Keywords.end() ? it->second: std::string());
}

const wxExMarker wxExLexers::GetMarker(const wxExMarker& marker) const
{
  const auto& it = m_Markers.find(marker);
  return (it != m_Markers.end() ? *it: wxExMarker());
}
  
bool wxExLexers::LoadDocument()
{
  // This test is to prevent showing an error if the lexers file does not exist,
  // as this is not required.
  if (!m_Path.FileExists()) return false;
  
  pugi::xml_document doc;

  if (const auto result = doc.load_file(m_Path.Path().string().c_str(), 
    pugi::parse_default | pugi::parse_trim_pcdata);
    !result)
  {
    wxExXmlError(m_Path, &result);
    return false;
  }
  
  m_DefaultStyle = wxExStyle();
  m_FoldingBackgroundColour.clear();
  m_FoldingForegroundColour.clear();
  m_GlobalProperties.clear();
  m_Indicators.clear();
  m_Keywords.clear();
  m_Lexers.clear();
  m_Macros.clear();
  m_Markers.clear();
  m_Styles.clear();
  m_StylesHex.clear();
  m_Texts.clear();
  m_ThemeColours.clear();
  m_ThemeMacros.clear();
  
  m_ThemeColours[std::string()] = m_DefaultColours;
  m_ThemeMacros[std::string()] = std::map<std::string, std::string>{};  
  
  for (const auto& node: doc.document_element().children())
  {
         if (strcmp(node.name(), "macro") == 0) ParseNodeMacro(node);
    else if (strcmp(node.name(), "global") == 0) ParseNodeGlobal(node);
    else if (strcmp(node.name(), "keyword")== 0) ParseNodeKeyword(node);
    else if (strcmp(node.name(), "lexer") ==  0) 
    {
      if (const wxExLexer lexer(&node); lexer.IsOk()) 
        m_Lexers.emplace_back(lexer);
    }
  }

  // Check config, but do not create one.
  if (auto* config = wxConfigBase::Get(false); config != nullptr)
  {
    const std::string extensions(std::accumulate(
      m_Lexers.begin(), m_Lexers.end(), std::string(wxFileSelectorDefaultWildcardStr), 
      [&](const std::string& a, const wxExLexer& b) {
        if (!b.GetExtensions().empty())
          return a.empty() ? b.GetExtensions() : a + wxExGetFieldSeparator() + b.GetExtensions();
        else return a;}));
    if (!config->Exists(_("Add what"))) config->Write(_("Add what"), extensions.c_str());
    if (!config->Exists(_("In files"))) config->Write(_("In files"), extensions.c_str());
    if (!config->Exists(_("In folder"))) config->Write(_("In folder"), wxGetHomeDir());
  }
  
  // Do some checking.
  if (!m_Lexers.empty() && !m_Theme.empty())
  {
    if (!m_DefaultStyle.IsOk()) wxExLog() << "default style not ok";
    if (!m_DefaultStyle.ContainsDefaultStyle()) wxExLog() << "default style does not contain default style";
  }  

  if (m_ThemeMacros.empty())
  {
    wxExLog() << "themes are missing";
  }
  else 
  {
    for (const auto& it : m_ThemeMacros)
    {
      if (!it.first.empty() && it.second.empty())
      {
        wxExLog("theme") << it.first << " is unknown";
      }
    } 
  }

  VLOG(9) << 
    "default colors: " << m_DefaultColours.size() <<
    " global properties: " << m_GlobalProperties.size() <<
    " indicators: " << m_Indicators.size() <<
    " keywords: " << m_Keywords.size() <<
    " lexers: " << m_Lexers.size() <<
    " macros: " << m_Macros.size() <<
    " markers: " << m_Markers.size() <<
    " styles: " << m_Styles.size() <<
    " styles hex: " << m_StylesHex.size() <<
    " texts: " << m_Texts.size() <<
    " theme colours: " << m_ThemeColours.size() <<
    " theme macros: " << m_ThemeMacros.size();
  
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
    if (m_Theme.empty())
    {
      // Do nothing.
    }
    else if (strcmp(child.name(), "foldmargin") == 0)
    {
      ParseNodeFolding(child);
    }
    else if (strcmp(child.name(), "hex") == 0)
    {
      m_StylesHex.emplace_back(child, "global");
    }
    else if (strcmp(child.name(), "indicator") == 0)
    {
      if (const wxExIndicator indicator(child); indicator.IsOk())
      {
        m_Indicators.insert(indicator);
      }
    }
    else if (strcmp(child.name(), "marker") == 0)
    {
      if (const wxExMarker marker(child); marker.IsOk()) m_Markers.insert(marker);
    }
    else if (strcmp(child.name(), "properties") == 0)
    {
      wxExNodeProperties(&child, m_GlobalProperties);
    }
    else if (strcmp(child.name(), "style") == 0)
    {
      if (const wxExStyle style(child, "global"); style.ContainsDefaultStyle())
      {
        if (m_DefaultStyle.IsOk())
        {
          wxExLog("duplicate default style") << child.name() << child;
        }
        else
        {
          m_DefaultStyle = style;
        }
      }
      else
      {
        if (style.GetDefine() == "style_textmargin")
        {
          m_StyleNoTextMargin = std::stoi(style.GetNo());
        }

        m_Styles.emplace_back(style);
      }
    }
    else if (strcmp(child.name(), "text") == 0)
    {
      m_Texts.push_back({child.attribute("no").value(), child.text().get()});
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
        if (const std::string no = macro.attribute("no").value(); !no.empty())
        {
          if (const auto& it = macro_map.find(no); it != macro_map.end())
          {
            wxExLog("macro") << no << macro << " already exists";
          }
          else
          {
            if (const std::string content = macro.text().get(); !content.empty())
            {
              try
              {
                val = std::stoi(content) + 1;
                macro_map[no] = content;
              }
              catch (std::exception& e)
              {
                wxExLog(e) << "macro";
              }
            }
            else
            {
              macro_map[no] = std::to_string(val);
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
  if (m_Theme.empty() && !wxConfigBase::Get(false)->Exists("theme"))
  {
    m_Theme = std::string(node.attribute("name").value());
  }
  
  std::map<std::string, std::string> tmpColours;
  std::map<std::string, std::string> tmpMacros;
  
  for (const auto& child: node.children())
  {
    if (const std::string content = child.text().get();
      strcmp(child.name(), "def") == 0)
    {
      if (const std::string style = child.attribute("style").value(); !style.empty())
      {
        if (const auto& it = tmpMacros.find(style); it != tmpMacros.end())
        {
          wxExLog("macro style") <<  style << child << " already exists";
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
      
wxExLexers* wxExLexers::Set(wxExLexers* lexers)
{
  wxExLexers* old = m_Self;
  m_Self = lexers;
  return old;
}

bool SingleChoice(wxWindow* parent, const std::string& title, 
  const wxArrayString& s, std::string& selection)
{
  wxSingleChoiceDialog dlg(parent, _("Input") + ":", title, s);

  if (const auto index = s.Index(selection); 
    index != wxNOT_FOUND) dlg.SetSelection(index);
  if (dlg.ShowModal() == wxID_CANCEL) return false;

  selection = dlg.GetStringSelection();
  
  return true;
}
  
bool wxExLexers::ShowDialog(wxExSTC* stc) const
{
  wxArrayString s;
  s.Add(std::string());
  for (const auto& it : m_Lexers) s.Add(it.GetDisplayLexer());

  auto lexer = stc->GetLexer().GetDisplayLexer();
  if (!SingleChoice(stc, _("Enter Lexer"), s, lexer)) return false;

  lexer.empty() ? stc->GetLexer().Reset(): (void)stc->GetLexer().Set(lexer, true);
  
  return true;
}

bool wxExLexers::ShowThemeDialog(wxWindow* parent)
{ 
  wxArrayString s;
  for (const auto& it : m_ThemeMacros) s.Add(it.first);

  if (!SingleChoice(parent, _("Enter Theme"), s, m_Theme)) return false;

  wxConfigBase::Get()->Write("theme", m_Theme.c_str());

  return LoadDocument();
}
