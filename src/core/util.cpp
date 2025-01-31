////////////////////////////////////////////////////////////////////////////////
// Name:      core/util.cpp
// Purpose:   Implementation of wex core utility methods
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>
#include <boost/url.hpp>

#include <wex/core/config.h>
#include <wex/core/core.h>
#include <wex/core/log.h>
#include <wex/core/path.h>
#include <wx/clipbrd.h>
#include <wx/generic/dirctrlg.h> // for wxFileIconsTable

#include <algorithm>
#include <numeric>
#include <regex>

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

bool wex::browser(const std::string& url)
{
  try
  {
    if (const boost::urls::url_view view(url);
        (view.has_scheme() || view.is_path_absolute()))
    {
      if (!wxLaunchDefaultBrowser(url))
      {
        log("browser launch") << url;
        return false;
      }

      log::info("browser") << url;
      return true;
    }
  }
  catch (const std::exception&)
  {
    // do nothing, see id::stc::open_www.. causes exception boost.url
  }

  return false;
}

bool wex::browser_search(const std::string& text)
{
  if (const auto& search_engine(
        config(_("stc.Search engine")).get_first_of("https://duckduckgo.com"));
      !search_engine.empty())
  {
    return browser(search_engine + "?q=" + text);
  }

  log("browser_search engine empty") << text;
  return false;
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
  /* NOLINTNEXTLINE */
  else if (wxTheClipboard->SetData(new wxTextDataObject(text)))
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

  return true;
}

const std::string wex::clipboard_get()
{
  if (wxClipboardLocker locker; !locker)
  {
    log("clipboard lock");
    return std::string();
  }
  /* NOLINTNEXTLINE */
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

const std::string
wex::find_after(const std::string& text, const std::string& seq)
{
  const auto pos = text.find(seq);
  return pos == std::string::npos ? text : text.substr(pos + seq.size());
}

const std::string
wex::find_before(const std::string& text, const std::string& seq)
{
  const auto pos = text.find(seq);
  return pos == std::string::npos ? text : text.substr(0, pos);
}

const std::string wex::find_tail(const std::string& text, size_t max_chars)
{
  if (text.size() > max_chars)
  {
    const size_t corr = (4 + text.size() - max_chars < text.size() ? 3 : 0);
    const auto   tail = boost::algorithm::find_tail(text, max_chars - corr);
    return (corr ? "..." : std::string()) +
           std::string(tail.begin(), tail.end());
  }

  return text;
}

int wex::get_number_of_lines(const std::string& text, bool trim)
{
  if (text.empty())
  {
    return 0;
  }

  const auto& trimmed = (trim ? boost::algorithm::trim_copy(text) : text);

  // If text contains \r\n, assume a DOS file, count only \n.
  // Otherwise, count all endings.
  return ((trimmed.contains("\r\n")) ?
            std::count(trimmed.begin(), trimmed.end(), '\n') :
            std::count_if(
              trimmed.begin(),
              trimmed.end(),
              [](auto i)
              {
                return i == '\n' || i == '\r';
              })) +
         1;
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

int wex::icompare(const std::string& text1, const std::string& text2)
{
  return boost::algorithm::to_upper_copy(text1).compare(
    boost::algorithm::to_upper_copy(text2));
}

bool wex::icontains(const std::string& text, const std::string& sequence)
{
  return boost::algorithm::icontains(text, sequence);
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
  const std::string& filename,
  const std::string& pattern,
  bool               is_regex)
{
  if (!is_regex && pattern == "*")
  {
    return true; // asterix matches always
  }

  if (filename.empty())
  {
    return false; // empty string never matches
  }

  // Make a regex of pattern matching chars.
  auto re(pattern);

  if (!is_regex)
  {
    boost::algorithm::replace_all(re, ".", "\\.");
    boost::algorithm::replace_all(re, "*", ".*");
    boost::algorithm::replace_all(re, "?", ".?");
  }

  for (const auto& it : boost::tokenizer<boost::char_separator<char>>(
         re,
         boost::char_separator<char>(";")))
  {
    try
    {
      if (
        !is_regex && std::regex_match(filename, std::regex(it)) ||
        is_regex && std::regex_search(filename, std::regex(it)))
      {
        return true;
      }
    }
    catch (std::regex_error& e)
    {
      log::status() << e.what();
      return false;
    }
  }

  return false;
}

const std::string wex::quoted(const std::string& text, char delim)
{
  std::ostringstream ss;
  ss << std::quoted(text, delim);
  return ss.str();
}

const std::string wex::quoted_find(const std::string& text, char c)
{
  return text.contains(c) ? quoted(text, '"') : text;
}

const std::string
wex::rfind_after(const std::string& text, const std::string& seq)
{
  const auto pos = text.rfind(seq);
  return pos == std::string::npos ? text : text.substr(pos + seq.size());
}

const std::string
wex::rfind_before(const std::string& text, const std::string& seq)
{
  const auto pos = text.rfind(seq);
  return pos == std::string::npos ? text : text.substr(0, pos);
}
