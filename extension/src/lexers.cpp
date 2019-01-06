////////////////////////////////////////////////////////////////////////////////
// Name:      lexers.cpp
// Purpose:   Implementation of wex::lexers class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <algorithm>
#include <functional>
#include <numeric>
#include <regex>
#include <wex/lexers.h>
#include <wex/config.h>
#include <wex/log.h>
#include <wex/stc.h>
#include <wex/tokenizer.h>
#include <wex/util.h>
#include <easylogging++.h>

// Constructor for lexers from specified filename.
// This must be an existing xml file containing all lexers.
// It does not do load_document, however if you use the global Get,
// it both constructs and loads the lexers.
wex::lexers::lexers(const path& filename)
  : m_Path(filename)
  , m_Theme(config("theme").get())
{
}

void wex::lexers::apply(stc* stc) const
{
  m_DefaultStyle.apply(stc);

  for (const auto& i : m_Indicators) i.apply(stc);
  for (const auto& p : m_globalProperties) p.apply(stc);
  for (const auto& m : m_Markers) m.apply(stc);
  
  if (stc->get_hexmode().is_active())
  {
    for (const auto& s : m_StylesHex) s.apply(stc);
  }
}
  
// No longer const, as it updates m_DefaultColours.
void wex::lexers::apply_global_styles(stc* stc)
{
  if (m_DefaultColours.empty())
  {
    m_DefaultColours["caretforeground"] = stc->GetCaretForeground().GetAsString();
    m_DefaultColours["caretlinebackground"] = stc->GetCaretLineBackground().GetAsString();
    m_DefaultColours["edge"] = stc->GetEdgeColour().GetAsString();
  }
    
  m_DefaultStyle.apply(stc);

  stc->StyleClearAll();

  for (const auto& s : m_Styles) s.apply(stc);

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

const std::string wex::lexers::apply_macro(
  const std::string& text, const std::string& lexer)
{
  if (const auto& it = get_macros(lexer).find(text);
    it != get_macros(lexer).end())
    return it->second;
  else if (const auto& it = theme_macros().find(text);
    it != theme_macros().end()) 
    return it->second;
  else 
    return text;
}

void wex::lexers::apply_margin_text_style(
  stc* stc, int line, margin_style_t style, const std::string& text) const
{
  switch (style)
  {
    case MARGIN_STYLE_DAY:
      stc->MarginSetStyle(line, m_StyleNoTextMarginDay);
      break;
    
    case MARGIN_STYLE_MONTH:
      stc->MarginSetStyle(line, m_StyleNoTextMarginMonth);
      break;
    
    case MARGIN_STYLE_OTHER:
      stc->MarginSetStyle(line, m_StyleNoTextMargin);
      break;
    
    case MARGIN_STYLE_WEEK:
      stc->MarginSetStyle(line, m_StyleNoTextMarginWeek);
      break;
    
    case MARGIN_STYLE_YEAR:
      stc->MarginSetStyle(line, m_StyleNoTextMarginYear);
      break;
  }
  
  if (!text.empty())
  {
    stc->MarginSetText(line, wxString(text.c_str()));
  }
}

const wex::lexer wex::lexers::find_by_filename(const std::string& fullname) const
{
  const auto& it = std::find_if(m_Lexers.begin(), m_Lexers.end(), 
    [fullname](auto const& e) {return !e.extensions().empty() && 
       matches_one_of(fullname, e.extensions());});
  return it != m_Lexers.end() ? *it: lexer();
}

const wex::lexer wex::lexers::find_by_name(const std::string& name) const
{
  const auto& it = std::find_if(m_Lexers.begin(), m_Lexers.end(), 
    [name](auto const& e) {return e.display_lexer() == name;});
  return it != m_Lexers.end() ? *it: lexer();
}

const wex::lexer wex::lexers::find_by_text(const std::string& text) const
{
  try
  {
    const std::string filtered(std::regex_replace(text, 
      std::regex("[ \t\n\v\f\r]+$"), "", std::regex_constants::format_sed));

    for (const auto& t : m_Texts) 
    {
      if (std::regex_search(filtered, std::regex(t.second)))
        return find_by_name(t.first);
    }
  }
  catch (std::exception& e)
  {
    log(e) << "findbytext";
  }

  return lexer();
}

wex::lexers* wex::lexers::get(bool createOnDemand)
{
  if (m_Self == nullptr && createOnDemand)
  {
    m_Self = new lexers(path(config().dir(), "wex-lexers.xml"));
    m_Self->load_document();
  }

  return m_Self;
}

const wex::indicator wex::lexers::get_indicator(const indicator& indicator) const
{
  const auto& it = m_Indicators.find(indicator);
  return (it != m_Indicators.end() ? *it: wex::indicator());
}

const std::string wex::lexers::keywords(const std::string& set) const
{
  const auto& it = m_Keywords.find(set);
  return (it != m_Keywords.end() ? it->second: std::string());
}

const wex::marker wex::lexers::get_marker(const marker& marker) const
{
  const auto& it = m_Markers.find(marker);
  return (it != m_Markers.end() ? *it: wex::marker());
}
  
bool wex::lexers::load_document()
{
  // This test is to prevent showing an error if the lexers file does not exist,
  // as this is not required.
  if (!m_Path.file_exists()) return false;
  
  pugi::xml_document doc;

  if (const auto result = doc.load_file(m_Path.data().string().c_str(), 
    pugi::parse_default | pugi::parse_trim_pcdata);
    !result)
  {
    xml_error(m_Path, &result);
    return false;
  }
  
  m_DefaultStyle = style();
  m_FoldingBackgroundColour.clear();
  m_FoldingForegroundColour.clear();
  m_globalProperties.clear();
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
    else if (strcmp(node.name(), "global") == 0) ParseNodeglobal(node);
    else if (strcmp(node.name(), "keyword")== 0) ParseNodeKeyword(node);
    else if (strcmp(node.name(), "lexer") ==  0) 
    {
      if (const wex::lexer lexer(&node); lexer.is_ok()) 
        m_Lexers.emplace_back(lexer);
    }
  }

  const std::string extensions(std::accumulate(
    m_Lexers.begin(), m_Lexers.end(), std::string(wxFileSelectorDefaultWildcardStr), 
    [&](const std::string& a, const wex::lexer& b) {
      if (!b.extensions().empty())
        return a.empty() ? b.extensions() : a + get_field_separator() + b.extensions();
      else return a;}));
  
  config conf(config::DATA_NO_STORE);
  if (!conf.item(_("Add what")).exists()) conf.set(extensions);
  if (!conf.item(_("In files")).exists()) conf.set(extensions);
  if (!conf.item(_("In folder")).exists()) conf.set(wxGetHomeDir().ToStdString());
  
  // Do some checking.
  if (!m_Lexers.empty() && !m_Theme.empty())
  {
    if (!m_DefaultStyle.is_ok()) log() << "default style not ok";
    if (!m_DefaultStyle.contains_default_style()) log() << "default style does not contain default style";
  }  

  if (m_ThemeMacros.empty())
  {
    log() << "themes are missing";
  }
  else 
  {
    for (const auto& it : m_ThemeMacros)
    {
      if (!it.first.empty() && it.second.empty())
      {
        log("theme") << it.first << " is unknown";
      }
    } 
  }

  VLOG(9) << 
    "default colors: " << m_DefaultColours.size() <<
    " global properties: " << m_globalProperties.size() <<
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

void wex::lexers::ParseNodeFolding(const pugi::xml_node& node)
{
  tokenizer fields(node.text().get(), ",");

  m_FoldingBackgroundColour = apply_macro(fields.get_next_token());
  m_FoldingForegroundColour = apply_macro(fields.get_next_token());
}

void wex::lexers::ParseNodeglobal(const pugi::xml_node& node)
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
      if (const wex::indicator indicator(child); indicator.is_ok())
      {
        m_Indicators.insert(indicator);
      }
    }
    else if (strcmp(child.name(), "marker") == 0)
    {
      if (const wex::marker marker(child); marker.is_ok()) m_Markers.insert(marker);
    }
    else if (strcmp(child.name(), "properties") == 0)
    {
      node_properties(&child, m_globalProperties);
    }
    else if (strcmp(child.name(), "style") == 0)
    {
      if (const wex::style style(child, "global"); style.contains_default_style())
      {
        if (m_DefaultStyle.is_ok())
        {
          log("duplicate default style") << child.name() << child;
        }
        else
        {
          m_DefaultStyle = style;
        }
      }
      else
      {
        if (style.define() == "style_textmargin")
        {
          m_StyleNoTextMargin = std::stoi(style.number());
        }
        else if (style.define() == "style_textmargin_day")
        {
          m_StyleNoTextMarginDay = std::stoi(style.number());
        }
        else if (style.define() == "style_textmargin_week")
        {
          m_StyleNoTextMarginWeek = std::stoi(style.number());
        }
        else if (style.define() == "style_textmargin_month")
        {
          m_StyleNoTextMarginMonth = std::stoi(style.number());
        }
        else if (style.define() == "style_textmargin_year")
        {
          m_StyleNoTextMarginYear = std::stoi(style.number());
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

void wex::lexers::ParseNodeKeyword(const pugi::xml_node& node)
{
  for (const auto& child: node.children())
  {
    m_Keywords[child.attribute("name").value()] = child.text().get();
  }
}

void wex::lexers::ParseNodeMacro(const pugi::xml_node& node)
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
            log("macro") << no << macro << " already exists";
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
                log(e) << "macro";
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

void wex::lexers::ParseNodeTheme(const pugi::xml_node& node)
{
  if (m_Theme.empty() && !config().item("theme").exists())
  {
    m_Theme = std::string(node.attribute("name").value());
  }
  
  std::map<std::string, std::string> tmpColours, tmpMacros;
  
  for (const auto& child: node.children())
  {
    if (const std::string content = child.text().get();
      strcmp(child.name(), "def") == 0)
    {
      if (const std::string style = child.attribute("style").value(); !style.empty())
      {
        if (const auto& it = tmpMacros.find(style); it != tmpMacros.end())
        {
          log("macro style") <<  style << child << " already exists";
        }
        else
        {
          tmpMacros[style] = content;
        }
      }
    }
    else if (strcmp(child.name(), "colour") == 0)
    {
      tmpColours[child.attribute("name").value()] = apply_macro(content);
    }
  }
  
  m_ThemeColours[node.attribute("name").value()] = tmpColours;
  m_ThemeMacros[node.attribute("name").value()] = tmpMacros;
}

void wex::lexers::ParseNodeThemes(const pugi::xml_node& node)
{
  for (const auto& child: node.children()) ParseNodeTheme(child);
}
      
wex::lexers* wex::lexers::set(wex::lexers* lexers)
{
  wex::lexers* old = m_Self;
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
  
bool wex::lexers::show_dialog(stc* stc) const
{
  wxArrayString s;
  s.Add(std::string());
  for (const auto& it : m_Lexers) s.Add(it.display_lexer());

  auto lexer = stc->get_lexer().display_lexer();
  if (!SingleChoice(stc, _("Enter Lexer"), s, lexer)) return false;

  lexer.empty() ? stc->get_lexer().reset(): (void)stc->get_lexer().set(lexer, true);
  
  return true;
}

bool wex::lexers::show_theme_dialog(wxWindow* parent)
{ 
  wxArrayString s;
  for (const auto& it : m_ThemeMacros) s.Add(it.first);

  if (!SingleChoice(parent, _("Enter Theme"), s, m_Theme)) return false;

  config("theme").set(m_Theme);

  return load_document();
}
