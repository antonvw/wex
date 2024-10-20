////////////////////////////////////////////////////////////////////////////////
// Name:      lexer.cpp
// Purpose:   Implementation of wex::lexer class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2008-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/tokenizer.hpp>
#include <wex/core/config.h>
#include <wex/core/core.h>
#include <wex/core/log.h>
#include <wex/syntax/lexer.h>
#include <wex/syntax/lexers.h>
#include <wex/syntax/stc.h>
#include <wex/syntax/util.h>
#include <wx/platinfo.h>

#include <algorithm>
#include <charconv>

#include "lexer-attribute-data.h"

namespace wex
{
int convert_int_attrib(
  const std::vector<std::pair<std::string, int>>& v,
  const std::string&                              text)
{
  if (const auto& it = std::find_if(
        v.begin(),
        v.end(),
        [text](const auto& p)
        {
          return text == p.first;
        });
      it != v.end())
  {
    return it->second;
  }

  log("unsupported attrib") << text;
  return -1;
}

/// Tokenizes the complete string into a vector of integers (size_t).
/// Returns the filled in vector.
auto tokenize_int(const std::string& text, const char* sep = " \t\r\n")
{
  std::vector<size_t> tokens;

  for (const auto& it : boost::tokenizer<boost::char_separator<char>>(
         text,
         boost::char_separator<char>(sep)))
  {
    if (int value = 0;
        std::from_chars(it.data(), it.data() + it.size(), value).ec ==
        std::errc())
    {
      tokens.emplace_back(value);
    }
  }

  return tokens;
};
} // namespace wex

wex::lexer::lexer(const std::string& lex)
  : lexer(nullptr, nullptr)
{
  if (!lex.empty())
  {
    set(lex);
  }
}

wex::lexer::lexer(syntax::stc* stc)
  : lexer(nullptr, stc)
{
}

wex::lexer::lexer(const pugi::xml_node* node)
  : lexer(node, nullptr)
{
  m_is_ok = !m_scintilla_lexer.empty();

  if (!m_is_ok)
  {
    wex::log("missing lexer") << *node;
  }
  else
  {
    parse_attrib(node);

    if (m_is_ok)
    {
      auto_match(
        (!node->attribute("macro").empty() ? node->attribute("macro").value() :
                                             m_scintilla_lexer));

      if (m_scintilla_lexer == "hypertext")
      {
        // As our lexers.xml files cannot use xml comments,
        // add them here.
        m_comment_begin = "<!--";
        m_command_end   = "-->";
      }

      parse_childen(node);
    }
  }
}

wex::lexer::lexer(const pugi::xml_node* node, syntax::stc* s)
  : m_scintilla_lexer(node != nullptr ? node->attribute("name").value() : "")
  , m_stc(s)
  , m_reflect(
      {REFLECT_ADD("display", m_display_lexer),
       REFLECT_ADD("extensions", m_extensions),
       REFLECT_ADD("language", m_language),
       REFLECT_ADD("lexer", m_scintilla_lexer)})
{
}

// Adds the specified keywords to the keywords map and the keywords set.
// The text might contain the keyword set after a ':'.
// Returns false if specified set is invalid or value is empty.
bool wex::lexer::add_keywords(const std::string& value, int setno)
{
  if (value.empty() || setno < 0 || setno >= wxSTC_KEYWORDSET_MAX)
  {
    return false;
  }

  std::set<std::string> keywords_set;

  for (const auto& it : boost::tokenizer<boost::char_separator<char>>(
         value,
         boost::char_separator<char>("\r\n ")))
  {
    const auto& line(it);
    std::string keyword;

    if (line.contains(":"))
    {
      keyword = find_before(line, ":");
      const auto& a_l(find_after(line, ":"));

      try
      {
        int new_setno = 0;
        if (
          std::from_chars(a_l.data(), a_l.data() + a_l.size(), new_setno).ec !=
            std::errc() ||
          new_setno <= 0 || new_setno >= wxSTC_KEYWORDSET_MAX)
        {
          wex::log("invalid keyword set") << new_setno << a_l;
          return false;
        }
        if (new_setno != setno)
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

const std::string wex::lexer::align_text(
  const std::string_view& lines,
  const std::string_view& header,
  bool                    fill_out_with_space,
  bool                    fill_out) const
{
  const auto line_length = usable_chars_per_line();

  // Use the header, with one space extra to separate, or no header at all.
  const auto& header_with_spaces =
    (header.empty()) ? std::string() : std::string(header.size(), ' ');

  bool        at_begin = true;
  std::string in(lines), line(header), out;

  while (!in.empty())
  {
    if (const auto& word = get_word(in);
        line.size() + 1 + word.size() > line_length)
    {
      out +=
        make_single_line_comment(line, fill_out_with_space, fill_out) + "\n";
      line = header_with_spaces + word;
    }
    else
    {
      line +=
        (!line.empty() && !at_begin ? std::string(" ") : std::string()) + word;
      at_begin = false;
    }
  }

  out += make_single_line_comment(line, fill_out_with_space, fill_out);

  return out;
}

bool wex::lexer::apply() const
{
  if (m_stc == nullptr)
  {
    return false;
  }

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

    for (const auto& p : m_properties)
    {
      p.apply(m_stc);
    }
    for (const auto& s : m_styles)
    {
      s.apply(m_stc);
    }
  }

  // And finally colour the entire document.
  if (const auto length = m_stc->GetLength(); length > 0)
  {
    m_stc->Colourise(0, length - 1);
  }

  switch (m_edge_columns.size())
  {
    case 0:
      break;

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
  const auto& a = std::find_if(
    m_attribs.begin(),
    m_attribs.end(),
    [&](auto const& i)
    {
      return std::get<0>(i) == name;
    });

  return a != m_attribs.end() ? std::get<1>(*a) : -1;
}

void wex::lexer::auto_match(const std::string& lexer)
{
  if (const auto& l(lexers::get()->find(lexer)); l.m_scintilla_lexer.empty())
  {
    if (const auto& macros(lexers::get()->get_macros(lexer)); macros.empty())
    {
      wex::log("no macros provided") << lexer;
      m_is_ok = false;
    }
    else
    {
      for (const auto& it : macros)
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
          if (const auto& style = std::find_if(
                lexers::get()->theme_macros().begin(),
                lexers::get()->theme_macros().end(),
                [&](auto const& e)
                {
                  return it.first.contains(e.first);
                });
              style != lexers::get()->theme_macros().end())
          {
            m_styles.emplace_back(it.second, style->second);
          }
        }
      }
    }
  }
  else
  {
    // Copy styles and properties, and not keywords,
    // so your derived display lexer can have its own keywords.
    m_styles     = l.m_styles;
    m_properties = l.m_properties;

    m_comment_begin  = l.m_comment_begin;
    m_comment_begin2 = l.m_comment_begin2;
    m_command_end    = l.m_command_end;
    m_command_end2   = l.m_command_end2;
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

  m_is_ok       = false;
  m_previewable = false;

  if (m_stc != nullptr)
  {
    m_stc->SetLexer(wxSTC_LEX_NULL);
    apply();
    m_stc->reset_margins(
      factory::stc::margin_t().set(factory::stc::MARGIN_FOLDING));
  }
}

const std::string wex::lexer::comment_complete(const std::string& comment) const
{
  if (m_command_end.empty())
  {
    return std::string();
  }

  // Fill out rest of comment with spaces, and comment end string.
  const int n = line_size() - comment.size() - m_command_end.size();
  if (n <= 0)
  {
    return std::string();
  }

  const auto& blanks = std::string(n, ' ');
  return blanks + m_command_end;
}

const std::string wex::lexer::formatted_text(
  const std::string& lines,
  const std::string& header,
  bool               fill_out_with_space,
  bool               fill_out) const
{
  std::string_view text = lines, header_to_use = header;
  std::string      out;

  // Process text between the carriage return line feeds.
  for (size_t pos = 0; (pos = text.find('\n')) != std::string::npos;)
  {
    out += align_text(
             text.substr(0, pos),
             header_to_use,
             fill_out_with_space,
             fill_out) +
           "\n";

    text          = text.substr(pos + 1);
    header_to_use = std::string(header.size(), ' ');
  }

  if (!text.empty())
  {
    out +=
      align_text(text, header_to_use, fill_out_with_space, fill_out) + "\n";
  }

  return out;
}

bool wex::lexer::is_keyword(const std::string& word) const
{
  return m_keywords.find(word) != m_keywords.end();
}

const std::string wex::lexer::keywords_string(
  int                keyword_set,
  size_t             min_size,
  const std::string& prefix) const
{
  if (keyword_set == -1)
  {
    return get_string_set(m_keywords, min_size, prefix);
  }

  if (const auto& it = m_keywords_set.find(keyword_set);
      it != m_keywords_set.end())
  {
    return get_string_set(it->second, min_size, prefix);
  }

  return std::string();
}

size_t wex::lexer::line_size() const
{
  return !m_edge_columns.empty() ?
           m_edge_columns.back() :
           (size_t)config(_("stc.Edge column")).get(80L);
}

bool wex::lexer::keyword_starts_with(const std::string& word) const
{
  const auto& it = m_keywords.lower_bound(word);
  return it != m_keywords.end() && it->starts_with(word);
}

const std::string wex::lexer::make_comment(
  const std::string& text,
  bool               fill_out_with_space,
  bool               fill_out) const
{
  std::string out;

  text.contains("\n") ?
    out += formatted_text(text, std::string(), fill_out_with_space, fill_out) :
    out += align_text(text, std::string(), fill_out_with_space, fill_out);

  return out;
}

const std::string wex::lexer::make_comment(
  const std::string& prefix,
  const std::string& text) const
{
  std::string out;

  text.contains("\n") ? out += formatted_text(text, prefix, true, true) :
                        out += align_text(text, prefix, true, true);

  return out;
}

const std::string wex::lexer::make_single_line_comment(
  const std::string_view& text,
  bool                    fill_out_with_space,
  bool                    fill_out) const
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
      {
        fill_out_character = '-';
      }
      else
      {
        fill_out_character = m_comment_begin[m_comment_begin.size() - 1];
      }
    }
    else
    {
      fill_out_character = ' ';
    }
  }

  std::string out = m_comment_begin + fill_out_character + std::string(text);

  // Fill out characters (prevent filling out spaces)
  if (fill_out && (fill_out_character != ' ' || !m_command_end.empty()))
  {
    if (const auto fill_chars = usable_chars_per_line() - text.size();
        fill_chars > 0)
    {
      out += std::string(fill_chars, fill_out_character);
    }
  }

  if (!m_command_end.empty())
  {
    out += fill_out_character + m_command_end;
  }

  return out;
}

void wex::lexer::parse_attrib(const pugi::xml_node* node)
{
  m_display_lexer =
    (!node->attribute("display").empty() ? node->attribute("display").value() :
                                           m_scintilla_lexer);
  m_extensions  = node->attribute("extensions").value();
  m_language    = node->attribute("language").value();
  m_previewable = !node->attribute("preview").empty();

  if (const std::string exclude(node->attribute("exclude").value());
      exclude.contains(
        wxPlatformInfo().GetOperatingSystemFamilyName().ToStdString()))
  {
    m_is_ok = false;
    return;
  }

  if (const std::string v(node->attribute("edgecolumns").value()); !v.empty())
  {
    try
    {
      m_edge_columns = tokenize_int(v);
    }
    catch (std::exception& e)
    {
      wex::log(e) << "edgecolumns:" << v;
    }
  }

  if (const std::string v(node->attribute("edgemode").value()); !v.empty())
  {
    m_attribs.emplace_back(
      _("Edge line"),
      convert_int_attrib(
        {{"none", wxSTC_EDGE_NONE},
         {"line", wxSTC_EDGE_LINE},
         {"background", wxSTC_EDGE_BACKGROUND}},
        v),
      [&](syntax::stc* stc, int attrib)
      {
        switch (attrib)
        {
          case -1:
            break;

          case wxSTC_EDGE_LINE:
            stc->SetEdgeMode(
              m_edge_columns.size() <= 1 ? wxSTC_EDGE_LINE :
                                           wxSTC_EDGE_MULTILINE);
            break;

          default:
            stc->SetEdgeMode(attrib);
            break;
        }
      });
  }

  if (const std::string v(node->attribute("spacevisible").value()); !v.empty())
  {
    m_attribs.emplace_back(
      _("Whitespace visible"),
      convert_int_attrib(
        {{"invisible", wxSTC_WS_INVISIBLE},
         {"always", wxSTC_WS_VISIBLEALWAYS},
         {"afterindent", wxSTC_WS_VISIBLEAFTERINDENT},
         {"onlyindent", wxSTC_WS_VISIBLEONLYININDENT}},
        v),
      [&](syntax::stc* stc, int attrib)
      {
        if (attrib >= 0)
        {
          stc->SetViewWhiteSpace(attrib);
        }
      });
  }

  if (const std::string v(node->attribute("tabdrawmode").value()); !v.empty())
  {
    m_attribs.emplace_back(
      _("Tab draw mode"),
      convert_int_attrib(
        {{"arrow", wxSTC_TD_LONGARROW}, {"strike", wxSTC_TD_STRIKEOUT}},
        v),
      [&](syntax::stc* stc, int attrib)
      {
        if (attrib >= 0)
        {
          stc->SetTabDrawMode(attrib);
        }
      });
  }

  if (const std::string v(node->attribute("tabmode").value()); !v.empty())
  {
    m_attribs.emplace_back(
      _("Expand tabs"),
      convert_int_attrib({{"use", 1}, {"off", 0}}, v),
      [&](syntax::stc* stc, int attrib)
      {
        if (attrib >= 0)
        {
          stc->SetUseTabs(attrib);
        }
      });
  }

  if (const auto v(node->attribute("tabwidth").as_int(0)); v > 0)
  {
    m_attribs.emplace_back(
      _("Tab width"),
      v,
      [&](syntax::stc* stc, int attrib)
      {
        if (attrib >= 0)
        {
          stc->SetIndent(attrib);
          stc->SetTabWidth(attrib);
        }
      });
  }

  if (const std::string v(node->attribute("wrapline").value()); !v.empty())
  {
    m_attribs.emplace_back(
      _("Wrap line"),
      convert_int_attrib(
        {{"none", wxSTC_WRAP_NONE},
         {"word", wxSTC_WRAP_WORD},
         {"char", wxSTC_WRAP_CHAR},
         {"whitespace", wxSTC_WRAP_WHITESPACE}},
        v),
      [&](syntax::stc* stc, int attrib)
      {
        if (attrib >= 0)
        {
          stc->SetWrapMode(attrib);
        }
      });
  }
}

void wex::lexer::parse_childen(const pugi::xml_node* node)
{
  for (const auto& child : node->children())
  {
    if (strcmp(child.name(), "styles") == 0)
    {
      node_styles(&child, m_scintilla_lexer, m_styles);
    }
    else if (strcmp(child.name(), "keywords") == 0)
    {
      parse_keyword(&child);
    }
    else if (strcmp(child.name(), "properties") == 0)
    {
      if (!m_properties.empty())
      {
        wex::log("properties already available") << m_scintilla_lexer << child;
      }

      node_properties(&child, m_properties);
    }
    else if (strcmp(child.name(), "comments") == 0)
    {
      m_comment_begin  = child.attribute("begin1").value();
      m_command_end    = child.attribute("end1").value();
      m_comment_begin2 = child.attribute("begin2").value();
      m_command_end2   = child.attribute("end2").value();
    }
  }
}

void wex::lexer::parse_keyword(const pugi::xml_node* node)
{
  // Add all direct keywords
  if (const std::string direct(node->text().get());
      !direct.empty() && !add_keywords(direct))
  {
    wex::log("keywords") << direct << " could not be set" << *node
                         << m_scintilla_lexer;
  }

  // Add all keywords that point to a keyword set.
  for (const auto& att : node->attributes())
  {
    try
    {
      lexer_attribute_data data(node, att);
      data.add_keywords(*this);
    }
    catch (std::exception& e)
    {
      wex::log(e) << "keyword:" << att.name();
    }
  }
}

bool wex::lexer::set(const std::string& lexer, bool fold)
{
  if (lexer.empty())
  {
    clear();
  }
  else if (
    !lexers::get()->get_lexers().empty() &&
    !set(lexers::get()->find(lexer), fold))
  {
    log::debug("lexer is not known") << lexer;
  }

  return m_is_ok;
}

bool wex::lexer::set(const lexer& lexer, bool fold)
{
  syntax::stc* keep = m_stc;

  (*this) =
    (lexer.m_scintilla_lexer.empty() && m_stc != nullptr &&
         !lexers::get()->get_lexers().empty() ?
       lexers::get()->find_by_text(m_stc->GetLine(0)) :
       lexer);

  if (keep)
  {
    m_stc = keep;
  }
  else
  {
    return m_is_ok;
  }

  m_is_ok = !m_scintilla_lexer.empty();
  m_stc->SetLexerLanguage(m_scintilla_lexer);
  m_stc->generic_settings();

  apply();

  const bool ok = m_stc->GetLexer() != wxSTC_LEX_NULL;

  if (!m_scintilla_lexer.empty() && !ok)
  {
    log::debug("lexer is not set") << lexer.display_lexer();
  }

  if (m_stc->GetProperty("fold") == "1" && !m_scintilla_lexer.empty())
  {
    m_stc->SetMarginWidth(
      m_stc->margin_folding_number(),
      config(_("stc.margin.Folding")).get(16));
  }

  if (fold)
  {
    m_stc->fold();
  }

  return m_scintilla_lexer.empty() || ok;
}

void wex::lexer::set_property(const std::string& name, const std::string& value)
{
  if (const auto& it = std::find_if(
        m_properties.begin(),
        m_properties.end(),
        [name](auto const& e)
        {
          return e.name() == name;
        });
      it != m_properties.end())
  {
    it->set(value);
  }
  else
  {
    m_properties.emplace_back(name, value);
  }
}

size_t wex::lexer::usable_chars_per_line() const
{
  // We adjust this here for
  // the space the beginning and end of the comment characters occupy.
  return line_size() -
         ((m_comment_begin.size() != 0) ? m_comment_begin.size() + 1 : 0) -
         ((m_command_end.size() != 0) ? m_command_end.size() + 1 : 0);
}
