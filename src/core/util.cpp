////////////////////////////////////////////////////////////////////////////////
// Name:      core/util.cpp
// Purpose:   Implementation of wex core utility methods
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <iomanip> // for get_time
#include <numeric>
#include <pugixml.hpp>
#include <regex>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/config.h>
#include <wex/core.h>
#include <wex/dir.h>
#include <wex/lexer.h>
#include <wex/listview-core.h>
#include <wex/log.h>
#include <wex/path.h>
#include <wex/stc-core.h>
#include <wex/tokenizer.h>
#include <wx/clipbrd.h>
#include <wx/generic/dirctrlg.h> // for wxTheFileIconsTable

wex::column::column()
{
  SetColumn(-1);
}

wex::column::column(const std::string& name, type_t type, int width)
  : m_type(type)
{
  wxListColumnFormat align;

  switch (m_type)
  {
    case column::FLOAT:
      align = wxLIST_FORMAT_RIGHT;
      if (width == 0)
        width = 80;
      break;

    case column::INT:
      align = wxLIST_FORMAT_RIGHT;
      if (width == 0)
        width = 60;
      break;

    case column::STRING:
      align = wxLIST_FORMAT_LEFT;
      if (width == 0)
        width = 100;
      break;

    case column::DATE:
      align = wxLIST_FORMAT_LEFT;
      if (width == 0)
        width = 150;
      break;

    default:
      assert(0);
  }

  SetColumn(-1); // default value, is set when inserting the col
  SetText(name);
  SetAlign(align);
  SetWidth(width);
}

void wex::column::set_is_sorted_ascending(sort_t type)
{
  switch (type)
  {
    case SORT_ASCENDING:
      m_is_sorted_ascending = true;
      break;

    case SORT_DESCENDING:
      m_is_sorted_ascending = false;
      break;

    case SORT_KEEP:
      break;

    case SORT_TOGGLE:
      m_is_sorted_ascending = !m_is_sorted_ascending;
      break;

    default:
      assert(0);
      break;
  }
}

const std::string wex::after(const std::string& text, char c, bool first)
{
  const auto pos = (first ? text.find(c) : text.rfind(c));
  return (pos == std::string::npos ? text : text.substr(pos + 1));
}

std::tuple<bool, const std::string, const std::vector<std::string>>
wex::auto_complete_filename(const std::string& text)
{
  // E.g.:
  // 1) text: src/vi
  // -> should build vector with files in ./src starting with vi
  // path:   src
  // prefix: vi
  // 2) text: /usr/include/s
  // ->should build vector with files in /usr/include starting with s
  // path:   /usr/include
  // prefix: s
  // And text might be prefixed by a command, e.g.: e src/vi
  path path(after(text, ' ', false));

  if (path.is_relative())
  {
    path.make_absolute();
  }

  const auto                     prefix(path.fullname());
  const std::vector<std::string> v(get_all_files(
    path.get_path(),
    data::dir()
      .file_spec(prefix + "*")
      .dir_spec(prefix + "*")
      .type(data::dir::type_t().set(data::dir::FILES).set(data::dir::DIRS))));

  if (v.empty())
  {
    return {false, std::string(), v};
  }

  if (v.size() > 1)
  {
    auto rest_equal_size = 0;
    bool all_ok          = true;

    for (auto i = prefix.length(); i < v[0].size() && all_ok; i++)
    {
      for (size_t j = 1; j < v.size() && all_ok; j++)
      {
        if (i < v[j].size() && v[0][i] != v[j][i])
        {
          all_ok = false;
        }
      }

      if (all_ok)
      {
        rest_equal_size++;
      }
    }

    return {true, v[0].substr(prefix.size(), rest_equal_size), v};
  }
  else
  {
    return {true, v[0].substr(prefix.size()), v};
  }
}

bool wex::auto_complete_text(
  const std::string&              text,
  const std::vector<std::string>& v,
  std::string&                    s)
{
  int matches = 0;

  for (const auto& it : v)
  {
    if (it.starts_with(text))
    {
      s = it;
      matches++;
    }
  }

  return (matches == 1);
}

const std::string wex::before(const std::string& text, char c, bool first)
{
  if (const auto pos = (first ? text.find(c) : text.rfind(c));
      pos != std::string::npos)
  {
    return text.substr(0, pos);
  }
  else
  {
    return text;
  }
}

bool wex::browser(const std::string& url)
{
  if (!wxLaunchDefaultBrowser(url))
  {
    return false;
  }

  log::info("browse") << url;

  return true;
}

bool wex::browser_search(const std::string& text)
{
  if (const auto& search_engine(config(_("stc.Search engine")).get_firstof());
      !search_engine.empty())
  {
    return browser(search_engine + "?q=" + text);
  }
  else
  {
    return false;
  }
}

bool wex::clipboard_add(const std::string& text)
{
  wxTheClipboard->UsePrimarySelection();

  if (text.empty())
  {
    log("clipboard text empty");
    return false;
  }

  if (wxClipboardLocker locker; !locker)
  {
    log("clipboard lock");
    return false;
  }
  else
  {
    if (wxTheClipboard->SetData(new wxTextDataObject(text)))
    {
      // Take care that clipboard data remain after exiting
      // This is a boolean method as well, we don't check it, as
      // clipboard data is copied.
      wxTheClipboard->Flush();
    }
    else
    {
      log("clipboard add");
      return false;
    }
  }

  return true;
}

const std::string wex::clipboard_get()
{
  if (wxClipboardLocker locker; !locker)
  {
    log("clipboard lock");
    return std::string();
  }
  else if (wxTheClipboard->IsSupported(wxDF_TEXT))
  {
    if (wxTextDataObject data; wxTheClipboard->GetData(data))
    {
      return data.GetText().ToStdString();
    }
  }

  log("clipboard empty");
  return std::string();
}

const std::string
wex::ellipsed(const std::string& text, const std::string& control, bool ellipse)
{
  return text + (ellipse ? "..." : std::string()) +
         (!control.empty() ? "\tCtrl+" + control : std::string());
}

const std::string wex::firstof(
  const std::string& text,
  const std::string& chars,
  size_t             start_pos,
  firstof_t          flags)
{
  const auto pos = !flags[FIRST_OF_FROM_END] ?
                     text.find_first_of(chars, start_pos) :
                     text.find_last_of(chars, start_pos);

  if (!flags[FIRST_OF_BEFORE])
  {
    return pos == std::string::npos ? std::string() : text.substr(pos + 1);
  }
  else
  {
    return pos == std::string::npos ? text : text.substr(0, pos);
  }
}

const std::string wex::get_endoftext(const std::string& text, size_t max_chars)
{
  auto text_out(text);

  if (text_out.length() > max_chars)
  {
    if (4 + text_out.length() - max_chars < text_out.length())
    {
      text_out = "..." + text_out.substr(4 + text_out.length() - max_chars);
    }
    else
    {
      text_out = text.substr(text.length() - max_chars);
    }
  }

  return text_out;
}

const std::string wex::get_find_result(
  const std::string& find_text,
  bool               find_next,
  bool               recursive)
{
  if (!recursive)
  {
    const auto where =
      (find_next) ? _("bottom").ToStdString() : _("top").ToStdString();

    return _("Searching for").ToStdString() + " " + quoted(trim(find_text)) +
           " " + _("hit").ToStdString() + " " + where;
  }
  else
  {
    if (config(_("Error bells")).get(true))
    {
      wxBell();
    }

    return quoted(trim(find_text)) + " " + _("not found").ToStdString();
  }
}

int wex::get_iconid(const path& filename)
{
  return filename.file_exists() ?
           wxFileIconsTable::file :
           (filename.dir_exists() ? wxFileIconsTable::folder :
                                    wxFileIconsTable::computer);
}

int wex::get_number_of_lines(const std::string& text, bool trim)
{
  if (text.empty())
  {
    return 0;
  }

  const auto trimmed = (trim ? wex::trim(text) : text);

  if (const int c = std::count(trimmed.begin(), trimmed.end(), '\n') + 1;
      c != 1)
  {
    return c;
  }

  return std::count(trimmed.begin(), trimmed.end(), '\r') + 1;
}

const std::string wex::get_string_set(
  const std::set<std::string>& kset,
  size_t                       min_size,
  const std::string&           prefix)
{
  return std::accumulate(
    kset.begin(),
    kset.end(),
    std::string{},
    [&](const std::string& a, const std::string& b) {
      return (b.size() >= min_size && b.starts_with(prefix)) ? a + b + ' ' : a;
    });
}

// See also get_modification_time in stat.cpp
std::tuple<bool, time_t>
wex::get_time(const std::string& text, const std::string& format)
{
  std::tm           tm = {0};
  std::stringstream ss(text);
  ss >> std::get_time(&tm, format.c_str());

  if (ss.fail())
  {
    wex::log("get_time") << ss << "format:" << format;
    return {false, 0};
  }

  const time_t t(mktime(&tm));

  return {t != -1, t};
}

const std::string wex::get_word(std::string& text)
{
  std::string field_separators = " \t";
  std::string token;
  tokenizer   tkz(text, field_separators);
  if (tkz.has_more_tokens())
    token = tkz.get_next_token();
  text = tkz.get_string();
  text = trim(text, skip_t().set(TRIM_LEFT));

  return token;
}

bool wex::is_brace(int c)
{
  return c == '[' || c == ']' || c == '(' || c == ')' || c == '{' || c == '}' ||
         c == '<' || c == '>';
}

bool wex::is_codeword_separator(int c)
{
  return isspace(c) || is_brace(c) || c == ',' || c == ';' || c == ':' ||
         c == '@';
}

int wex::match(
  const std::string&        reg,
  const std::string&        text,
  std::vector<std::string>& v)
{
  if (reg.empty())
    return -1;

  try
  {
    if (std::match_results<std::string::const_iterator> m;
        !std::regex_search(text, m, std::regex(reg)))
    {
      return -1;
    }
    else if (m.size() > 1)
    {
      v.clear();
      std::copy(++m.begin(), m.end(), std::back_inserter(v));
    }

    return v.size();
  }
  catch (std::regex_error& e)
  {
    log(e) << reg << "code:" << (int)e.code();
    return -1;
  }
}

bool wex::matches_one_of(
  const std::string& fullname,
  const std::string& pattern)
{
  if (pattern == "*")
    return true; // asterix matches always
  if (fullname.empty())
    return false; // empty string never matches

  // Make a regex of pattern matching chars.
  auto re(pattern);
  replace_all(re, ".", "\\.");
  replace_all(re, "*", ".*");
  replace_all(re, "?", ".?");

  for (tokenizer tkz(re, ";"); tkz.has_more_tokens();)
  {
    if (std::regex_match(fullname, std::regex(tkz.get_next_token())))
      return true;
  }

  return false;
}

void wex::node_properties(
  const pugi::xml_node*  node,
  std::vector<property>& properties)
{
  for (const auto& child : node->children())
  {
    if (strcmp(child.name(), "property") == 0)
    {
      properties.emplace_back(child);
    }
  }
}

const std::string wex::now(const std::string& format)
{
  std::time_t       tm = std::time(nullptr);
  std::stringstream ss;
  ss << std::put_time(std::localtime(&tm), format.c_str());
  return ss.str();
}

bool wex::one_letter_after(const std::string& text, const std::string& letter)
{
  return std::regex_match(letter, std::regex("^" + text + "[a-zA-Z]$"));
}

void wex::node_styles(
  const pugi::xml_node* node,
  const std::string&    lexer,
  std::vector<style>&   styles)
{
  for (const auto& child : node->children())
  {
    if (strcmp(child.name(), "style") == 0)
    {
      styles.emplace_back(child, lexer);
    }
  }
}

const std::string wex::print_caption(const path& filename)
{
  return filename.string();
}

const std::string wex::print_footer()
{
  return _("Page @PAGENUM@ of @PAGESCNT@").ToStdString();
}

const std::string wex::print_header(const path& filename)
{
  if (filename.file_exists())
  {
    return get_endoftext(
      filename.string() + " " + filename.stat().get_modification_time(),
      filename.lexer().line_size());
  }
  else
  {
    return _("Printed").ToStdString() + ": " + now();
  }
}

const std::string wex::quoted(const std::string& text)
{
  return "'" + text + "'";
}

bool wex::regafter(const std::string& text, const std::string& letter)
{
  return std::regex_match(
    letter,
    std::regex("^" + text + "[0-9=\"a-z%._\\*]$"));
}

int wex::replace_all(
  std::string&       text,
  const std::string& search,
  const std::string& replace,
  int*               match_pos)
{
  int  count  = 0;
  bool update = false;

  for (size_t pos = 0; (pos = text.find(search, pos)) != std::string::npos;)
  {
    if (match_pos != nullptr && !update)
    {
      *match_pos = (int)pos;
      update     = true;
    }

    text.replace(pos, search.length(), replace);
    pos += replace.length();

    count++;
  }

  return count;
}

template <typename InputIterator>
const std::string GetColumn(InputIterator first, InputIterator last)
{
  std::string text;

  for (InputIterator it = first; it != last; ++it)
  {
    text += it->second;
  }

  return text;
}

template <typename InputIterator>
const std::string get_lines(
  std::vector<std::string>& lines,
  size_t                    pos,
  size_t                    len,
  InputIterator             ii)
{
  std::string text;

  for (auto& it : lines)
  {
    text += it.replace(pos, len, *ii);
    ++ii;
  }

  return text;
}

bool wex::single_choice_dialog(
  wxWindow*            parent,
  const std::string&   title,
  const wxArrayString& s,
  std::string&         selection)
{
  wxSingleChoiceDialog dlg(parent, _("Input") + ":", title, s);

  if (const auto index = s.Index(selection); index != wxNOT_FOUND)
    dlg.SetSelection(index);
  if (dlg.ShowModal() == wxID_CANCEL)
    return false;

  selection = dlg.GetStringSelection();

  return true;
}

const std::string wex::sort(
  const std::string& input,
  string_sort_t      sort_t,
  size_t             pos,
  const std::string& eol,
  size_t             len)
{
  // Empty lines are not kept after sorting, as they are used as separator.
  std::map<std::string, std::string>      m;
  std::multimap<std::string, std::string> mm;
  std::multiset<std::string>              ms;
  std::vector<std::string>                lines;

  for (tokenizer tkz(input, eol); tkz.has_more_tokens();)
  {
    const std::string line = tkz.get_next_token() + eol;

    // Use an empty key if line is to short.
    std::string key;

    if (pos < line.length())
    {
      key = line.substr(pos, len);
    }

    if (len == std::string::npos)
    {
      if (sort_t[STRING_SORT_UNIQUE])
        m.insert({key, line});
      else
        mm.insert({key, line});
    }
    else
    {
      lines.emplace_back(line);
      ms.insert(key);
    }
  }

  std::string text;

  if (len == std::string::npos)
  {
    if (sort_t[STRING_SORT_DESCENDING])
    {
      text =
        (sort_t[STRING_SORT_UNIQUE] ? GetColumn(m.rbegin(), m.rend()) :
                                      GetColumn(mm.rbegin(), mm.rend()));
    }
    else
    {
      text =
        (sort_t[STRING_SORT_UNIQUE] ? GetColumn(m.begin(), m.end()) :
                                      GetColumn(mm.begin(), mm.end()));
    }
  }
  else
  {
    text =
      (sort_t[STRING_SORT_DESCENDING] ?
         get_lines(lines, pos, len, ms.rbegin()) :
         get_lines(lines, pos, len, ms.begin()));
  }

  return text;
}

bool wex::sort_selection(
  core::stc*    stc,
  string_sort_t sort_t,
  size_t        pos,
  size_t        len)
{
  bool error = false;

  try
  {
    if (stc->SelectionIsRectangle())
    {
      const auto start_pos = stc->GetSelectionNStart(0);

      if (start_pos == -1)
      {
        log("sort_selection rectangle start_pos") << start_pos;
        return false;
      }

      std::string selection;

      for (int i = 0; i < stc->GetSelections(); i++)
      {
        auto start = stc->GetSelectionNStart(i);
        auto end   = stc->GetSelectionNEnd(i);

        if (start == end)
        {
          log("sort_selection rectangle start equals end") << start;
          return false;
        }

        selection += stc->GetTextRange(start, end) + "\n";
      }

      stc->BeginUndoAction();

      const auto nr_cols =
        stc->GetColumn(stc->GetSelectionEnd()) - stc->GetColumn(start_pos);
      const auto nr_lines = stc->LineFromPosition(stc->GetSelectionEnd()) -
                            stc->LineFromPosition(start_pos);

      const auto& text(sort(selection, sort_t, 0, "\n"));

      tokenizer tkz(text, "\n");

      for (int i = 0; i < stc->GetSelections(); i++)
      {
        auto start = stc->GetSelectionNStart(i);
        auto end   = stc->GetSelectionNEnd(i);
        stc->Replace(start, end, tkz.get_next_token());
      }

      stc->SelectNone();
      stc->SetCurrentPos(start_pos);

      for (int j = 0; j < nr_cols; j++)
      {
        stc->CharRightRectExtend();
      }
      for (int i = 0; i < nr_lines; i++)
      {
        stc->LineDownRectExtend();
      }
    }
    else
    {
      const auto start_pos = stc->GetSelectionStart();

      if (
        start_pos == -1 || pos > (size_t)stc->GetSelectionEnd() ||
        pos == std::string::npos || stc->GetSelectionEmpty())
      {
        log("sort_selection") << start_pos << pos << stc->GetSelectionEnd();
        return false;
      }

      const auto text(
        sort(stc->get_selected_text(), sort_t, pos, stc->eol(), len));

      stc->BeginUndoAction();
      stc->ReplaceSelection(text);
      stc->SetSelection(start_pos, start_pos + text.size());
    }
  }
  catch (std::exception& e)
  {
    log(e) << "during sort";
    error = true;
  }

  stc->EndUndoAction();

  return !error;
}

const std::string
wex::translate(const std::string& text, int pageNum, int numPages)
{
  auto translation(text);

  replace_all(translation, "@PAGENUM@", std::to_string(pageNum));
  replace_all(translation, "@PAGESCNT@", std::to_string(numPages));

  return translation;
}

const std::string
wex::trim(const std::string& text, skip_t type, const std::string& replace_with)
{
  auto output(text);

  if (type[TRIM_MID])
  {
    output = std::regex_replace(
      output,
      std::regex("[ \t\n\v\f\r]+"),
      replace_with,
      std::regex_constants::format_sed);
  }

  if (type[TRIM_LEFT])
  {
    output = std::regex_replace(
      output,
      std::regex("^[ \t\n\v\f\r]+"),
      "",
      std::regex_constants::format_sed);
  }

  if (type[TRIM_RIGHT])
  {
    output = std::regex_replace(
      output,
      std::regex("[ \t\n\v\f\r]+$"),
      "",
      std::regex_constants::format_sed);
  }

  return output;
}
