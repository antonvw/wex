////////////////////////////////////////////////////////////////////////////////
// Name:      lexer.cpp
// Purpose:   Implementation of wex::lexer class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <functional>
#include <numeric>
#include <pugixml.hpp>
#include <wex/lexer.h>
#include <wex/config.h>
#include <wex/frame.h>
#include <wex/lexers.h>
#include <wex/log.h>
#include <wex/stc.h>
#include <wex/tokenizer.h>
#include <wex/util.h>

namespace wex
{
  int convert_attrib(
    const std::vector<std::pair<std::string, int>> & v,
    const std::string& attrib)
  {
    for (const auto& e : v)
    {
      if (attrib == e.first)
      {
        return e.second;
      }
    }

    log("unsupported attrib") << attrib;
    
    return -1;
  }
}

wex::lexer::lexer(const std::string& lexer)
{
  set(lexer);
}
      
wex::lexer::lexer(stc* stc)
  : m_stc(stc) 
{
}

wex::lexer::lexer(const pugi::xml_node* node)
  : m_scintilla_lexer(node->attribute("name").value())
{
  m_is_ok = !m_scintilla_lexer.empty();

  if (!m_is_ok)
  {
    wex::log("missing lexer") << *node;
  }
  else
  {
    parse_attrib(node);
          
    auto_match((!node->attribute("macro").empty() ?
      node->attribute("macro").value():
      m_scintilla_lexer));

    if (m_scintilla_lexer == "hypertext")
    {
      // As our lexers.xml files cannot use xml comments,
      // add them here.
      m_comment_begin = "<!--";
      m_command_end = "-->";
    }

    for (const auto& child: node->children())
    {
      if (strcmp(child.name(), "styles") == 0)
      {
        node_styles(&child, m_scintilla_lexer, m_styles);
      }
      else if (strcmp(child.name(), "keywords") == 0)
      {
        // Add all direct keywords
        if (const std::string& direct(child.text().get());
          !direct.empty() && !add_keywords(direct))
        {
          wex::log("keywords could not be set") << child;
        }

        // Add all keywords that point to a keyword set.
        for (const auto& att: child.attributes())
        {
          std::string nm(att.name());
          const auto pos = nm.find("-");
          try
          {
            const auto setno = (pos == std::string::npos ? 
              0: std::stoi(nm.substr(pos + 1)));
            const auto keywords = lexers::get()->keywords(att.value());

            if (keywords.empty())
            {
              wex::log("empty keywords for") << att.value() << child;
            }

            if (!add_keywords(keywords, setno))
            {
              wex::log("keywords for") << att.value() << "could not be set" << child;
            }
          }
          catch (std::exception& e)
          {
            wex::log(e) << "keyword:" << att.name();
          }
        }
      }
      else if (strcmp(child.name(), "properties") == 0)
      {
        if (!m_properties.empty())
        {
          wex::log("properties already available") << child;
        }

        node_properties(&child, m_properties);
      }
      else if (strcmp(child.name(), "comments") == 0)
      {
        m_comment_begin = child.attribute("begin1").value();
        m_command_end = child.attribute("end1").value();
        m_comment_begin2 = child.attribute("begin2").value();
        m_command_end2 = child.attribute("end2").value();
      }
    }
  }
}
  
wex::lexer& wex::lexer::operator=(const wex::lexer& l)
{
  if (this != &l)
  {
    m_comment_begin = l.m_comment_begin;
    m_comment_begin2 = l.m_comment_begin2;
    m_command_end = l.m_command_end;
    m_command_end2 = l.m_command_end2;
    m_display_lexer = l.m_display_lexer;
    m_edge_columns = l.m_edge_columns;
    m_extensions = l.m_extensions;
    m_is_ok = l.m_is_ok;
    m_previewable = l.m_previewable;
    m_keywords = l.m_keywords;
    m_keywords_set = l.m_keywords_set;
    m_language = l.m_language;
    m_previewable = l.m_previewable;
    m_properties = l.m_properties;
    m_scintilla_lexer = l.m_scintilla_lexer;
    m_styles = l.m_styles;

    m_attribs = l.m_attribs;
    
    if (m_stc == nullptr && l.m_stc != nullptr)
    {
      m_stc = l.m_stc;
    }
  }

  return *this;
}
  
// Adds the specified keywords to the keywords map and the keywords set.
// The text might contain the keyword set after a ':'.
// Returns false if specified set is illegal or value is empty.
bool wex::lexer::add_keywords(const std::string& value, int setno)
{
  if (value.empty() || setno < 0 || setno >= wxSTC_KEYWORDSET_MAX)
  {
    return false;
  }
  
  std::set<std::string> keywords_set;

  for (tokenizer tkz(value, "\r\n "); tkz.has_more_tokens(); )
  {
    const auto line(tkz.get_next_token());
    std::string keyword;

    if (tokenizer fields(line, ":"); fields.count_tokens() > 1)
    {
      keyword = fields.get_next_token();

      try
      {
        if (const auto new_setno = std::stoi(fields.get_next_token());
          new_setno <= 0 || new_setno >= wxSTC_KEYWORDSET_MAX)
        {
          wex::log("invalid keyword set") << new_setno;
          return false;
        }
        else if (new_setno != setno)
        {
          if (!keywords_set.empty())
          {
            m_keywords_set.insert({setno, keywords_set});
            keywords_set.clear();
          }

          setno = new_setno;
        }
      }
      catch (std::exception& e)
      {
        wex::log(e) << "keyword:" << keyword;
        return false;
      }
    }
    else
    {
      keyword = line;
    }

    keywords_set.insert(keyword);
    m_keywords.insert(keyword);
  }

  if (const auto& it = m_keywords_set.find(setno); it == m_keywords_set.end())
  {
    m_keywords_set.insert({setno, keywords_set});
  }
  else
  {
    it->second.insert(keywords_set.begin(), keywords_set.end());
  }

  return true;
}

bool wex::lexer::apply() const
{
  if (m_stc == nullptr) return false;

  m_stc->ClearDocumentStyle();

  for (const auto& it : m_properties)
  {
    it.apply_reset(m_stc);
  }

  // Reset keywords, also if no lexer is available.
  for (size_t setno = 0; setno < wxSTC_KEYWORDSET_MAX; setno++)
  {
    m_stc->SetKeyWords(setno, std::string());
  }

  lexers::get()->apply_global_styles(m_stc);

  if (!lexers::get()->theme().empty())
  {
    for (const auto& k : m_keywords_set)
    {
      m_stc->SetKeyWords(k.first, get_string_set(k.second));
    }

    lexers::get()->apply(m_stc);

    for (const auto& p : m_properties) p.apply(m_stc);
    for (const auto& s : m_styles) s.apply(m_stc);
  }

  // And finally colour the entire document.
  if (const auto length = m_stc->GetLength(); length > 0)
  {
    m_stc->Colourise(0, length - 1);
  }
  
  switch (m_edge_columns.size())
  {
    case 0: break;

    case 1:
      m_stc->SetEdgeColumn(m_edge_columns.front());
      break;

    default:
      for (const auto& c : m_edge_columns)
      {
        m_stc->MultiEdgeAddLine(c, m_stc->GetEdgeColour());
      }
  }

  for (const auto& i : m_attribs)
  {
    std::get<2>(i)(m_stc, std::get<1>(i));
  }
  
  return true;
}

int wex::lexer::attrib(const std::string& name) const
{
  for (const auto& a : m_attribs)
  {
    if (std::get<0>(a) == name)
    {
      return std::get<1>(a);
    }
  }
  
  return -1;
}
  
void wex::lexer::auto_match(const std::string& lexer)
{
  if (const auto& l(lexers::get()->find(lexer));
    l.m_scintilla_lexer.empty())
  {
    if (lexers::get()->get_macros(lexer).empty())
    {
      wex::log::warning("no macros provided") << lexer;
    }

    for (const auto& it : lexers::get()->get_macros(lexer))
    {
      // First try exact match.
      if (const auto& macro = lexers::get()->theme_macros().find(it.first);
        macro != lexers::get()->theme_macros().end())
      {
        m_styles.emplace_back(it.second, macro->second);
      }
      else
      {
        // Then, a partial using find_if.
        if (const auto& style = 
          std::find_if(lexers::get()->theme_macros().begin(), 
            lexers::get()->theme_macros().end(), 
            [&](auto const& e) {
            return it.first.find(e.first) != std::string::npos;});
          style != lexers::get()->theme_macros().end())
        {
          m_styles.emplace_back(it.second, style->second);
        }
      }
    }
  }
  else
  {
    // Copy styles and properties, and not keywords,
    // so your derived display lexer can have it's own keywords.
    m_styles = l.m_styles;
    m_properties = l.m_properties;
    
    m_comment_begin = l.m_comment_begin;
    m_comment_begin2 = l.m_comment_begin2;
    m_command_end = l.m_command_end;
    m_command_end2 = l.m_command_end2;
  }
}

void wex::lexer::clear()
{
  m_comment_begin.clear();
  m_comment_begin2.clear();
  m_command_end.clear();
  m_command_end2.clear();
  m_display_lexer.clear();
  m_edge_columns.clear();
  m_extensions.clear();
  m_keywords.clear();
  m_keywords_set.clear();
  m_language.clear();
  m_properties.clear();
  m_scintilla_lexer.clear();
  m_styles.clear();
  m_attribs.clear();

  m_is_ok = false;
  m_previewable = false;

  if (m_stc != nullptr)
  {
    ((wxStyledTextCtrl *)m_stc)->SetLexer(wxSTC_LEX_NULL);
    apply();
    frame::statustext(std::string(), "PaneLexer");
    m_stc->reset_margins(stc::margin_t().set(stc::MARGIN_FOLDING));
  }
}

const std::string wex::lexer::comment_complete(const std::string& comment) const
{
  if (m_command_end.empty()) return std::string();
  
  // Fill out rest of comment with spaces, and comment end string.
  if (const int n = line_size() - comment.size() - m_command_end.size(); n <= 0) 
  {
    return std::string();
  }
  else
  {
    const auto blanks = std::string(n, ' ');
    return blanks + m_command_end;
  }
}

const std::string wex::lexer::formatted_text(
  const std::string& lines,
  const std::string& header,
  bool fill_out_with_space,
  bool fill_out) const
{
  std::string_view text = lines, header_to_use = header;
  std::string out;

  // Process text between the carriage return line feeds.
  for (size_t nCharIndex; (nCharIndex = text.find("\n")) != std::string::npos; )
  {
    out += align_text(
      text.substr(0, nCharIndex),
      header_to_use,
      fill_out_with_space,
      fill_out,
      *this);

    text = text.substr(nCharIndex + 1);
    header_to_use = std::string(header.size(), ' ');
  }

  if (!text.empty())
  {
    out += align_text(
      text,
      header_to_use,
      fill_out_with_space,
      fill_out,
      *this);
  }

  return out;
}

bool wex::lexer::is_keyword(const std::string& word) const
{
  return m_keywords.find(word) != m_keywords.end();
}

const std::string wex::lexer::keywords_string(
  int keyword_set, size_t min_size, const std::string& prefix) const
{
  if (keyword_set == -1)
  {
    return get_string_set(m_keywords, min_size, prefix);
  }
  else
  {
    if (const auto& it = m_keywords_set.find(keyword_set);
      it != m_keywords_set.end())
    {
      return get_string_set(it->second, min_size, prefix);
    }
  }

  return std::string();
}

size_t wex::lexer::line_size() const 
{
  return !m_edge_columns.empty() ?
    m_edge_columns.back():
    (size_t)config(_("stc.Edge column")).get(80l);
}

bool wex::lexer::keyword_starts_with(const std::string& word) const
{
  const auto& it = m_keywords.lower_bound(word);
  return 
    it != m_keywords.end() &&
    it->find(word) == 0;
}

std::stringstream wex::lexer::log() const
{
  std::stringstream ss;
  ss << 
    "display: " << m_display_lexer <<
    "extensions: " << m_extensions <<
    "language: " << m_language <<
    "lexer: " << m_scintilla_lexer;

  return ss;
}
  
const std::string wex::lexer::make_comment(
  const std::string& text,
  bool fill_out_with_space,
  bool fill_out) const
{
  std::string out;

  text.find("\n") != std::string::npos ?
    out += formatted_text(text, std::string(), fill_out_with_space, fill_out):
    out += align_text(text, std::string(), fill_out_with_space, fill_out, *this);

  return out;
}

const std::string wex::lexer::make_comment(
  const std::string& prefix,
  const std::string& text) const
{
  std::string out;

  text.find("\n") != std::string::npos ?
    out += formatted_text(text, prefix, true, true):
    out += align_text(text, prefix, true, true, *this);

  return out;
}

const std::string wex::lexer::make_single_line_comment(
  const std::string_view& text,
  bool fill_out_with_space,
  bool fill_out) const
{
  if (m_comment_begin.empty() && m_command_end.empty())
  {
    return std::string(text);
  }

  // First set the fill_out_character.
  char fill_out_character;

  if (fill_out_with_space || m_scintilla_lexer == "hypertext")
  {
    fill_out_character = ' ';
  }
  else
  {
    if (text.empty())
    {
      if (m_comment_begin == m_command_end)
           fill_out_character = '-';
      else fill_out_character = m_comment_begin[m_comment_begin.size() - 1];
    }
    else   fill_out_character = ' ';
  }

  std::string out = m_comment_begin + fill_out_character + std::string(text);

  // Fill out characters (prevent filling out spaces)
  if (fill_out && 
      (fill_out_character != ' ' || !m_command_end.empty()))
  {
    if (const auto fill_chars = 
      usable_chars_per_line() - text.size(); fill_chars > 0)
    {
      out += std::string(fill_chars, fill_out_character);
    }
  }

  if (!m_command_end.empty()) out += fill_out_character + m_command_end;

  return out;
}

void wex::lexer::parse_attrib(const pugi::xml_node* node)
{
  m_display_lexer = (!node->attribute("display").empty() ?
    node->attribute("display").value(): m_scintilla_lexer);
  const std::string exclude(node->attribute("exclude").value());
  m_extensions = node->attribute("extensions").value();
  m_language = node->attribute("language").value();
  m_previewable = !node->attribute("preview").empty();
  
  std::string platform;
  
#ifdef __WXGTK__
  platform = "linux";
#endif
#ifdef __WXMSW__
  platform = "msw";
#endif
#ifdef __WXOSX__
  platform = "osx";
#endif

  if (exclude.find(platform) != std::string::npos)
  {
    m_is_ok = false;
    return;
  }

  if (const std::string a(node->attribute("edgecolumns").value()); !a.empty())
  {
    try
    {
      m_edge_columns = tokenizer(a).tokenize();
    }
    catch (std::exception& e)
    {
      wex::log(e) << "edgecolumns:" << a;
    }
  }
    
  if (const std::string a(node->attribute("edgemode").value()); !a.empty())
  {
    m_attribs.push_back(
      {_("Edge line"), convert_attrib(
        {{"none", wxSTC_EDGE_NONE},
         {"line", wxSTC_EDGE_LINE},
         {"background", wxSTC_EDGE_BACKGROUND}}, a), [&](stc* stc, int attrib) {
             switch (attrib)
             {
               case -1: break;
                
               case wxSTC_EDGE_LINE:
                 stc->SetEdgeMode(m_edge_columns.size() <= 1 ? 
                   wxSTC_EDGE_LINE: wxSTC_EDGE_MULTILINE); 
                 break;
                
               default:
                 stc->SetEdgeMode(attrib); 
                 break;
             }}
      });
  }

  if (const std::string a(node->attribute("spacevisible").value()); !a.empty())
  {
    m_attribs.push_back(
      {_("Whitespace visible"), convert_attrib(
          {{"invisible", wxSTC_WS_INVISIBLE},
           {"always", wxSTC_WS_VISIBLEALWAYS},
           {"afterindent", wxSTC_WS_VISIBLEAFTERINDENT},
           {"onlyindent", wxSTC_WS_VISIBLEONLYININDENT}}, a), [&](stc* stc, int attrib) {
              if (attrib >= 0)
              {
                stc->SetViewWhiteSpace(attrib);
              }}
      });
  }

  if (const std::string a(node->attribute("tabdrawmode").value()); !a.empty())
  {
    m_attribs.push_back(
      {_("Tab draw mode"), convert_attrib(
          {{"arrow", wxSTC_TD_LONGARROW},
           {"strike", wxSTC_TD_STRIKEOUT}}, a), [&](stc* stc, int attrib) {
              if (attrib >= 0)
              {
                stc->SetTabDrawMode(attrib);
              }}
      });
  }
  
  if (const std::string a(node->attribute("tabmode").value()); !a.empty())
  {
    m_attribs.push_back(
      {_("Use tabs"), convert_attrib(
        {{"use", 1},
         {"off", 0}}, a), [&](stc* stc, int attrib) {
            if (attrib >= 0)
            {
              stc->SetUseTabs(true);
            }}
      });
  }

  if (const auto a(node->attribute("tabwidth").as_int(0)); a > 0)
  {
    m_attribs.push_back(
      {_("Tab width"), a, [&](stc* stc, int attrib) {
        if (attrib >= 0)
        {
          stc->SetIndent(attrib);
          stc->SetTabWidth(attrib);
        }}
      });
  }

  if (const std::string a(node->attribute("wrapline").value()); !a.empty())
  {
    m_attribs.push_back(
      {_("Wrap line"), convert_attrib(
          {{"none", wxSTC_WRAP_NONE},
           {"word", wxSTC_WRAP_WORD},
           {"char", wxSTC_WRAP_CHAR},
           {"whitespace", wxSTC_WRAP_WHITESPACE}}, a), [&](stc* stc, int attrib) {
              if (attrib >= 0)
              {
                stc->SetWrapMode(attrib);
              }}
      });
  }
}
    
bool wex::lexer::set(const std::string& lexer, bool fold)
{
  if (
    !lexer.empty() &&
    !lexers::get()->get_lexers().empty() &&
    !set(lexers::get()->find(lexer), fold))
  {
    log::verbose("lexer is not known") << lexer;
  }
  
  return m_is_ok;
}

bool wex::lexer::set(const lexer& lexer, bool fold)
{
  (*this) = (lexer.m_scintilla_lexer.empty() && m_stc != nullptr ?
     lexers::get()->find_by_text(m_stc->GetLine(0)): lexer);

  if (m_stc == nullptr) return m_is_ok;

  m_stc->SetLexerLanguage(m_scintilla_lexer);

  apply();
  
  const bool ok =
    (((wxStyledTextCtrl *)m_stc)->GetLexer()) != wxSTC_LEX_NULL;
  
  if (!m_scintilla_lexer.empty() && !ok)
  {
    log::verbose("lexer is not set") << lexer.display_lexer();
  }

  frame::statustext(display_lexer(), "PaneLexer");

  if (fold)
  {
    m_stc->fold();
  }

  return m_scintilla_lexer.empty() || ok;
}

void wex::lexer::set_property(
  const std::string& name, const std::string& value)
{
  if (const auto& it = std::find_if(
    m_properties.begin(), m_properties.end(), 
    [name](auto const& e) {return e.name() == name;});
    it != m_properties.end()) it->set(value);
  else m_properties.emplace_back(name, value);
}

size_t wex::lexer::usable_chars_per_line() const
{
  // We adjust this here for
  // the space the beginning and end of the comment characters occupy.
  return line_size()
    - ((m_comment_begin.size() != 0) ? m_comment_begin.size() + 1 : 0)
    - ((m_command_end.size() != 0) ? m_command_end.size() + 1 : 0);
}
