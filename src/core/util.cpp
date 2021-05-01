////////////////////////////////////////////////////////////////////////////////
// Name:      core/util.cpp
// Purpose:   Implementation of wex core utility methods
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>
#include <numeric>
#include <regex>
#include <wex/config.h>
#include <wex/core.h>
#include <wex/dir.h>
#include <wex/log.h>
#include <wex/path.h>
#include <wx/choicdlg.h>
#include <wx/clipbrd.h>
#include <wx/generic/dirctrlg.h> // for wxFileIconsTable

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
  if (const auto& search_engine(config(_("stc.Search engine")).get_first_of());
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

  log::info("clipboard empty");

  return std::string();
}

const std::string
wex::ellipsed(const std::string& text, const std::string& control, bool ellipse)
{
  return text + (ellipse ? "..." : std::string()) +
         (!control.empty() ? "\tCtrl+" + control : std::string());
}

const std::string wex::first_of(
  const std::string& text,
  const std::string& chars,
  size_t             start_pos,
  first_of_t         flags)
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

    return _("Searching for").ToStdString() + " " +
           quoted(boost::algorithm::trim_copy(find_text)) + " " +
           _("hit").ToStdString() + " " + where;
  }
  else
  {
    if (config(_("Error bells")).get(true))
    {
      wxBell();
    }

    return quoted(boost::algorithm::trim_copy(find_text)) + " " +
           _("not found").ToStdString();
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

  const auto trimmed = (trim ? boost::algorithm::trim_copy(text) : text);

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
    [&](const std::string& a, const std::string& b)
    {
      return (b.size() >= min_size && b.starts_with(prefix)) ? a + b + ' ' : a;
    });
}

const std::string wex::get_word(std::string& text)
{
  boost::tokenizer<boost::char_separator<char>> tok(
    text,
    boost::char_separator<char>(" \t"));

  std::string token;

  if (auto it = tok.begin(); it != tok.end())
  {
    token = *it;
    text  = text.substr(token.size());
  }

  boost::algorithm::trim_left(text);

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
         c == '+' || c == '-' || c == '@';
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
  boost::algorithm::replace_all(re, ".", "\\.");
  boost::algorithm::replace_all(re, "*", ".*");
  boost::algorithm::replace_all(re, "?", ".?");

  for (const auto& it : boost::tokenizer<boost::char_separator<char>>(
         re,
         boost::char_separator<char>(";")))
  {
    if (std::regex_match(fullname, std::regex(it)))
      return true;
  }

  return false;
}

bool wex::one_letter_after(const std::string& text, const std::string& letter)
{
  return std::regex_match(letter, std::regex("^" + text + "[a-zA-Z]$"));
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

const std::string
wex::translate(const std::string& text, int pageNum, int numPages)
{
  const auto& translation(boost::algorithm::replace_all_copy(
    text,
    "@PAGENUM@",
    std::to_string(pageNum)));

  return boost::algorithm::replace_all_copy(
    translation,
    "@PAGESCNT@",
    std::to_string(numPages));
}
