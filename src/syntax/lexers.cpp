////////////////////////////////////////////////////////////////////////////////
// Name:      lexers.cpp
// Purpose:   Implementation of wex::lexers class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2008-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/common/util.h>
#include <wex/core/config.h>
#include <wex/core/core.h>
#include <wex/core/log.h>
#include <wex/core/regex.h>
#include <wex/factory/stc.h>
#include <wex/syntax/blame.h>
#include <wex/syntax/lexers.h>
#include <wex/syntax/util.h>

#include <algorithm>
#include <charconv>
#include <functional>
#include <numeric>

wex::lexers::lexers()
  : m_path(wex::path(config::dir(), "wex-lexers.xml"))
  , m_path_macro(wex::path(config::dir(), "wex-lexers-macro.xml"))
  , m_theme(config("theme").get())
  , m_reflect(
      {REFLECT_ADD("default colours", m_default_colours.size()),
       REFLECT_ADD("global properties", m_global_properties.size()),
       REFLECT_ADD("indicators", m_indicators.size()),
       REFLECT_ADD("keywords", m_keywords.size()),
       REFLECT_ADD("lexers", m_lexers.size()),
       REFLECT_ADD("macros", m_macros.size()),
       REFLECT_ADD("markers", m_markers.size()),
       REFLECT_ADD("styles", m_styles.size()),
       REFLECT_ADD("styles hex", m_styles_hex.size()),
       REFLECT_ADD("texts", m_texts.size()),
       REFLECT_ADD("theme colours", m_theme_colours.size()),
       REFLECT_ADD("theme macros", m_theme_macros.size())})
{
}

void wex::lexers::apply(factory::stc* stc) const
{
  m_default_style.apply(stc);

  for (const auto& i : m_indicators)
  {
    i.apply(stc);
  }
  for (const auto& p : m_global_properties)
  {
    p.apply(stc);
  }
  for (const auto& m : m_markers)
  {
    m.apply(stc);
  }

  if (stc->is_hexmode())
  {
    for (const auto& s : m_styles_hex)
    {
      s.apply(stc);
    }
  }
}

void wex::lexers::apply_default_style(
  const std::function<void(const std::string&)>& back,
  const std::function<void(const std::string&)>& fore) const
{
  if (regex r(",back:(.*),");
      back != nullptr && r.match(m_default_style.value()) > 0)
  {
    back(r[0]);
  }

  if (regex r(",fore:(.*)");
      fore != nullptr && r.match(m_default_style.value()) > 0)
  {
    fore(r[0]);
  }
}

// No longer const, as it updates m_default_colours.
void wex::lexers::apply_global_styles(factory::stc* stc)
{
  if (m_default_colours.empty())
  {
    m_default_colours["caretforeground"] =
      stc->GetCaretForeground().GetAsString();
    m_default_colours["caretlinebackground"] =
      stc->GetCaretLineBackground().GetAsString();
    m_default_colours["edge"] = stc->GetEdgeColour().GetAsString();
  }

  m_default_style.apply(stc);

  stc->StyleClearAll();

  for (const auto& s : m_styles)
  {
    s.apply(stc);
  }

  stc->SetFoldMarginHiColour(
    true,
    !m_folding_background_colour.empty() ?
      wxColour(m_folding_background_colour.c_str()) :
      // See ViewStyle.cxx foldmarginColour
      wxColour(0xc0, 0xc0, 0xc0));

  stc->SetFoldMarginColour(
    true,
    !m_folding_foreground_colour.empty() ?
      wxColour(m_folding_foreground_colour.c_str()) :
      wxColour(0xff, 0, 0));

  if (const auto& colour_it = m_theme_colours.find(m_theme);
      colour_it != m_theme_colours.end())
  {
    for (const auto& it : colour_it->second)
    {
      if (it.first == "caretforeground")
      {
        stc->SetCaretForeground(it.second.c_str());
      }
      else if (it.first == "caretlinebackground")
      {
        stc->SetCaretLineBackground(it.second.c_str());
      }
      else if (it.first == "selbackground")
      {
        stc->SetSelBackground(true, it.second.c_str());
      }
      else if (it.first == "selforeground")
      {
        stc->SetSelForeground(true, it.second.c_str());
      }
      else if (it.first == "calltipbackground")
      {
        stc->CallTipSetBackground(it.second.c_str());
      }
      else if (it.first == "calltipforeground")
      {
        stc->CallTipSetForeground(it.second.c_str());
      }
      else if (it.first == "edge")
      {
        stc->SetEdgeColour(it.second.c_str());
      }
    }
  }
}

const std::string wex::lexers::apply_macro(
  const std::string& text,
  const std::string& lexer) const
{
  if (const auto& it = get_macros(lexer).find(text);
      it != get_macros(lexer).end())
  {
    return it->second;
  }

  if (const auto& it = theme_macros().find(text); it != theme_macros().end())
  {
    return it->second;
  }

  return text;
}

void wex::lexers::apply_margin_text_style(factory::stc* stc, const blame* blame)
  const
{
  if (blame->line_no() < 0)
  {
    log("apply_margin_text_style") << "invalid line:" << blame->line_no();
    return;
  }

  switch (blame->style())
  {
    case blame::margin_style_t::DAY:
      stc->MarginSetStyle(blame->line_no(), m_style_no_text_margin_day);
      break;

    case blame::margin_style_t::MONTH:
      stc->MarginSetStyle(blame->line_no(), m_style_no_text_margin_month);
      break;

    case blame::margin_style_t::OTHER:
      stc->MarginSetStyle(blame->line_no(), m_style_no_text_margin);
      break;

    case blame::margin_style_t::WEEK:
      stc->MarginSetStyle(blame->line_no(), m_style_no_text_margin_week);
      break;

    case blame::margin_style_t::YEAR:
      stc->MarginSetStyle(blame->line_no(), m_style_no_text_margin_year);
      break;

    case blame::margin_style_t::UNKNOWN:
      break;
  }

  if (!blame->info().empty() && !blame->skip_info())
  {
    stc->MarginSetText(blame->line_no(), blame->info());
  }
}

void wex::lexers::clear_theme()
{
  if (!m_theme.empty())
  {
    m_theme_previous = m_theme;
    m_theme.clear();
  }
}

const wex::lexer& wex::lexers::find(const std::string& name) const
{
  assert(!m_lexers.empty());

  const auto& it = std::find_if(
    m_lexers.begin(),
    m_lexers.end(),
    [name](auto const& e)
    {
      return e.display_lexer() == name;
    });

  return it != m_lexers.end() ? *it : m_lexers.front();
}

const wex::lexer&
wex::lexers::find_by_filename(const std::string& filename) const
{
  assert(!m_lexers.empty());

  const auto& it = std::find_if(
    m_lexers.begin(),
    m_lexers.end(),
    [filename](auto const& e)
    {
      return !e.extensions().empty() &&
             matches_one_of(filename, e.extensions());
    });

  return it != m_lexers.end() ? *it : m_lexers.front();
}

const wex::lexer& wex::lexers::find_by_text(const std::string& text) const
{
  assert(!m_lexers.empty());

  if (text.empty())
  {
    return m_lexers.front();
  }

  try
  {
    const auto& filtered(std::regex_replace(
      text,
      std::regex("[ \t\n\v\f\r]+$"),
      "",
      std::regex_constants::format_sed));

    for (const auto& t : m_texts)
    {
      if (std::regex_search(filtered, std::regex(t.second)))
      {
        return find(t.first);
      }
    }
  }
  catch (std::exception& e)
  {
    log(e) << "find_by_text";
  }

  return m_lexers.front();
}

wex::lexers* wex::lexers::get(bool createOnDemand)
{
  if (m_self == nullptr && createOnDemand)
  {
    m_self = new lexers();
    m_self->load_document();
  }

  return m_self;
}

const wex::indicator&
wex::lexers::get_indicator(const indicator& indicator) const
{
  const auto& it = m_indicators.find(indicator);
  return (
    it != m_indicators.end() ? *it : *m_indicators.find(wex::indicator()));
}

const wex::lexers::name_values_t&
wex::lexers::get_macros(const std::string& lexer) const
{
  const auto& it = m_macros.find(lexer);
  return (
    it != m_macros.end() ? it->second : m_macros.find(std::string())->second);
}

const wex::marker& wex::lexers::get_marker(const marker& marker) const
{
  const auto& it = m_markers.find(marker);
  return (it != m_markers.end() ? *it : *m_markers.find(wex::marker()));
}

const std::string& wex::lexers::keywords(const std::string& set) const
{
  const auto& it = m_keywords.find(set);
  return (
    it != m_keywords.end() ? it->second :
                             m_keywords.find(std::string())->second);
}

bool wex::lexers::load_document()
{
  if (!load_document_init())
  {
    return false;
  }

  pugi::xml_document doc;

  load_document(doc, m_path);

  for (const auto& node : doc.document_element().children())
  {
    if (strcmp(node.name(), "global") == 0)
    {
      parse_node_global(node);
    }
    else if (strcmp(node.name(), "keyword") == 0)
    {
      parse_node_keyword(node);
    }
    else if (strcmp(node.name(), "lexer") == 0)
    {
      if (const wex::lexer lexer(&node); lexer.is_ok())
      {
        m_lexers.emplace_back(lexer);
      }
    }
  }

  load_document_check();

  m_is_loaded = true;

  return true;
}

void wex::lexers::load_document(pugi::xml_document& doc, const wex::path& file)
{
  if (const auto result = doc.load_file(
        file.string().c_str(),
        pugi::parse_default | pugi::parse_trim_pcdata);
      !result)
  {
    log(result) << file;
  }
}

void wex::lexers::load_document_check()
{
  if (!m_lexers.empty() && !m_theme.empty())
  {
    if (!m_default_style.is_ok())
    {
      log() << "default style not ok";
    }
    if (!m_default_style.contains_default_style())
    {
      log() << "default style does not contain default style";
    }
  }

  if (m_theme_macros.size() <= 1)
  {
    log() << "themes are missing";
  }
  else
  {
    for (const auto& it : m_theme_macros)
    {
      if (!it.first.empty() && it.second.empty())
      {
        log("theme") << it.first << " is unknown";
      }
    }
  }

  log::info("lexers") << m_path.string() << m_reflect.log();
}

bool wex::lexers::load_document_init()
{
  // This test is to prevent showing an error if the lexers file does not exist,
  // as this is not required.
  if (!m_path.file_exists() || !m_path_macro.file_exists())
  {
    return false;
  }

  if (m_is_loaded)
  {
    m_default_style.clear();
    m_folding_background_colour.clear();
    m_folding_foreground_colour.clear();
    m_global_properties.clear();
    m_indicators.clear();
    m_keywords.clear();
    m_lexers.clear();
    m_markers.clear();
    m_styles.clear();
    m_styles_hex.clear();
    m_texts.clear();
    m_theme_colours.clear();
    m_theme_macros.clear();

    m_lexers.emplace_back();
  }
  else
  {
    pugi::xml_document doc;

    load_document(doc, m_path_macro);

    for (const auto& node : doc.document_element().children())
    {
      if (strcmp(node.name(), "macro") == 0)
      {
        parse_node_macro(node);
      }
      else if (strcmp(node.name(), "colour") == 0)
      {
        wxTheColourDatabase->AddColour(
          node.attribute("no").value(),
          node.text().get());
      }
    }
  }

  m_indicators.insert(indicator());
  m_keywords[std::string()] = std::string();
  m_macros[std::string()]   = name_values_t{};
  m_markers.insert(marker());
  m_theme_colours[std::string()] = m_default_colours;
  m_theme_macros[std::string()]  = name_values_t{};

  return true;
}

void wex::lexers::parse_node_folding(const pugi::xml_node& node)
{
  m_folding_background_colour =
    apply_macro(find_before(node.text().get(), ","));
  m_folding_foreground_colour = apply_macro(find_after(node.text().get(), ","));
}

void wex::lexers::parse_node_global(const pugi::xml_node& node)
{
  for (const auto& child : node.children())
  {
    // this should be the first, to fill themes
    if (strcmp(child.name(), "themes") == 0)
    {
      parse_node_themes(child);
    }
    else if (m_theme.empty())
    {
      // Do nothing.
    }
    else if (strcmp(child.name(), "foldmargin") == 0)
    {
      parse_node_folding(child);
    }
    else if (strcmp(child.name(), "hex") == 0)
    {
      m_styles_hex.emplace_back(child, "global");
    }
    else if (strcmp(child.name(), "indicator") == 0)
    {
      if (const wex::indicator indicator(child); indicator.is_ok())
      {
        m_indicators.insert(indicator);
      }
    }
    else if (strcmp(child.name(), "marker") == 0)
    {
      if (const wex::marker marker(child); marker.is_ok())
      {
        m_markers.insert(marker);
      }
    }
    else if (strcmp(child.name(), "properties") == 0)
    {
      node_properties(&child, m_global_properties);
    }
    else if (strcmp(child.name(), "style") == 0)
    {
      if (const wex::style style(child, "global");
          style.contains_default_style())
      {
        if (m_default_style.is_ok())
        {
          log("duplicate default style") << child.name() << child;
        }
        else
        {
          m_default_style = style;
        }
      }
      else
      {
        if (style.define() == "style_textmargin")
        {
          m_style_no_text_margin = style.number();
        }
        else if (style.define() == "style_textmargin_day")
        {
          m_style_no_text_margin_day = style.number();
        }
        else if (style.define() == "style_textmargin_week")
        {
          m_style_no_text_margin_week = style.number();
        }
        else if (style.define() == "style_textmargin_month")
        {
          m_style_no_text_margin_month = style.number();
        }
        else if (style.define() == "style_textmargin_year")
        {
          m_style_no_text_margin_year = style.number();
        }

        m_styles.emplace_back(style);
      }
    }
    else if (strcmp(child.name(), "text") == 0)
    {
      m_texts.emplace_back(child.attribute("no").value(), child.text().get());
    }
  }
}

void wex::lexers::parse_node_keyword(const pugi::xml_node& node)
{
  for (const auto& child : node.children())
  {
    m_keywords[child.attribute("name").value()] = child.text().get();
  }
}

void wex::lexers::parse_node_macro(const pugi::xml_node& node)
{
  for (const auto& child : node.children())
  {
    if (const std::string name = child.attribute("name").value();
        strcmp(child.name(), "def") == 0)
    {
      parse_node_macro_def(child, name);
    }
    else
    {
      log("unsupported macro node") << name;
    }
  }
}

void wex::lexers::parse_node_macro_def(
  const pugi::xml_node& child,
  const std::string&    name)
{
  name_values_t macro_map;
  int           val = 0;

  for (const auto& macro : child.children())
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
          if (
            std::from_chars(
              content.data(),
              content.data() + content.size(),
              val)
              .ec == std::errc())
          {
            val++;
            macro_map[no] = content;
          }
          else
          {
            log("macro") << content << "not a number" << child;
          }
        }
        else
        {
          const auto [ptr, ec] = std::to_chars(
            m_buffer.data(),
            m_buffer.data() + m_buffer.size(),
            val++);
          macro_map[no] =
            std::string_view(m_buffer.data(), ptr - m_buffer.data());
        }
      }
    }
    else
    {
      log("macro") << name << "attribute no is missing" << macro;
    }
  }

  m_macros[name] = macro_map;
}

void wex::lexers::parse_node_theme(const pugi::xml_node& node)
{
  if (m_theme.empty() && !config().item("theme").exists())
  {
    m_theme = std::string(node.attribute("name").value());
  }

  name_values_t tmpColours, tmpMacros;

  for (const auto& child : node.children())
  {
    if (const std::string content = child.text().get();
        strcmp(child.name(), "def") == 0)
    {
      if (const std::string style = child.attribute("style").value();
          !style.empty())
      {
        if (const auto& it = tmpMacros.find(style); it != tmpMacros.end())
        {
          log("macro style") << style << child << " already exists";
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

  m_theme_colours[node.attribute("name").value()] = tmpColours;
  m_theme_macros[node.attribute("name").value()]  = tmpMacros;
}

void wex::lexers::parse_node_themes(const pugi::xml_node& node)
{
  for (const auto& child : node.children())
  {
    parse_node_theme(child);
  }
}

wex::lexers* wex::lexers::set(wex::lexers* lexers)
{
  wex::lexers* old = m_self;
  m_self           = lexers;
  return old;
}

bool wex::lexers::show_theme_dialog(wxWindow* parent)
{
  std::vector<std::string> v;

  std::transform(
    m_theme_macros.begin(),
    m_theme_macros.end(),
    std::back_inserter(v),
    [](const auto& i)
    {
      return i.first;
    });

  if (!single_choice_dialog(
        data::window()
          .parent(parent)
          .title(_("Enter Theme"))
          .size(
#ifdef __WXGTK__
            wxSize(100, 250)
#else
            wxDefaultSize
#endif
              ),
        v,
        m_theme))
  {
    return false;
  }

  config("theme").set(m_theme);

  return load_document();
}

const wex::lexers::name_values_t& wex::lexers::theme_macros() const
{
  const auto& it = m_theme_macros.find(m_theme);
  return (
    it != m_theme_macros.end() ? it->second :
                                 m_theme_macros.find(std::string())->second);
}
