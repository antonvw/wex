////////////////////////////////////////////////////////////////////////////////
// Name:      util.cpp
// Purpose:   Implementation of wex utility methods
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <iomanip>
#include <numeric>
#include <pugixml.hpp>
#include <regex>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/app.h>
#include <wx/clipbrd.h>
#include <wx/generic/dirctrlg.h> // for wxTheFileIconsTable
#include <wx/wupdlock.h>
#include <wex/util.h>
#include <wex/config.h>
#include <wex/dir.h>
#include <wex/ex.h>
#include <wex/filedlg.h>
#include <wex/frame.h>
#include <wex/frd.h>
#include <wex/lexer.h>
#include <wex/lexers.h>
#include <wex/log.h>
#include <wex/managedframe.h>
#include <wex/path.h>
#include <wex/process.h>
#include <wex/stc.h>
#include <wex/tokenizer.h>
#include <wex/tostring.h>
#include <wex/vcs.h>
#include <wex/vi-macros.h>

const std::string wex::after(
  const std::string& text, char c, bool first)
{
  const auto pos = (first ? text.find(c): text.rfind(c));
  return
    (pos == std::string::npos ? text: text.substr(pos + 1));
}

const std::string wex::align_text(
  const std::string_view& lines, const std::string_view& header,
  bool fill_out_with_space, bool fill_out, const lexer& lexer)
{
  const auto line_length = lexer.usable_chars_per_line();

  // Use the header, with one space extra to separate, or no header at all.
  const auto header_with_spaces =
    (header.empty()) ? std::string() : std::string(header.size(), ' ');

  std::string in(lines);
  std::string line(header);

  bool at_begin = true;
  std::string out;

  while (!in.empty())
  {
    if (const auto word = get_word(in, false, false);
      line.size() + 1 + word.size() > line_length)
    {
      out += lexer.make_single_line_comment(line, fill_out_with_space, fill_out) + "\n";
      line = header_with_spaces + word;
    }
    else
    {
      line += (!line.empty() && !at_begin ? std::string(" "): std::string()) + word;
      at_begin = false;
    }
  }

  out += lexer.make_single_line_comment(line, fill_out_with_space, fill_out);

  return out;
}

std::tuple<bool, const std::string, const std::vector<std::string>> 
  wex::autocomplete_filename(const std::string& text)
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

  const auto prefix(path.fullname());
  const std::vector <std::string > v(get_all_files(
    path.get_path(), 
    prefix + "*",
    prefix + "*",
    dir::type_t().set(dir::FILES).set(dir::DIRS)));

  if (v.empty())
  {
    return {false, std::string(), v};
  }

  if (v.size() > 1)
  {
    auto rest_equal_size = 0;
    bool all_ok = true;
      
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

bool wex::autocomplete_text(const std::string& text, 
  const std::vector<std::string> & v, std::string& s)
{
  int matches = 0;
  
  for (const auto& it : v)
  {
    if (it.find(text) == 0)
    {
      s = it;
      matches++;
    }
  }

  return (matches == 1);
}

const std::string wex::before(
  const std::string& text, char c, bool first)
{
  if (const auto pos = (first ? text.find(c): text.rfind(c));
    pos != std::string::npos)
  {
    return text.substr(0, pos);
  }
  else
  {
    return text;
  }
}  

bool wex::browser_search(const std::string& text)
{
  if (const auto search_engine(config(_("Search engine")).firstof());
    search_engine.empty())
  {
    return false;
  }
  else 
  {
    wxLaunchDefaultBrowser(search_engine + "?q=" + text);
    return true;
  }
}

bool wex::clipboard_add(const std::string& text)
{
  if (text.empty())
  {
    log("clipboard text empty");
    return false;
  }
  
  if (wxClipboardLocker locker; !locker)
  {
    return false;
  }
  else
  {
    if (wxTheClipboard->AddData(new wxTextDataObject(text)))
    {
      // Take care that clipboard data remain after exiting
      // This is a boolean method as well, we don't check it, as
      // clipboard data is copied.
      // At least on Ubuntu 8.10 FLush returns false.
      wxTheClipboard->Flush();
    }
  }

  return true;
}

const std::string wex::clipboard_get()
{
  if (wxClipboardLocker locker; !locker)
  {
    return std::string();
  }
  else if (wxTheClipboard->IsSupported(wxDF_TEXT))
  {
    if (wxTextDataObject data; wxTheClipboard->GetData(data))
    {
      return data.GetText().ToStdString();
    }
  }

  return std::string();
}

void wex::combobox_from_list(wxComboBox* cb, const std::list < std::string > & text)
{
  combobox_as<const std::list < std::string >>(cb, text);
}

bool wex::comparefile(const path& file1, const path& file2)
{
  if (config(_("Comparator")).empty())
  {
    return false;
  }

  const auto arguments =
     (file1.stat().st_mtime < file2.stat().st_mtime) ?
       "\"" + file1.data().string() + "\" \"" + file2.data().string() + "\"":
       "\"" + file2.data().string() + "\" \"" + file1.data().string() + "\"";

  if (!process().execute(
    config(_("Comparator")).get() + " " + arguments, 
    process::EXEC_WAIT))
  {
    return false;
  }

  log::status(_("Compared")) << arguments;

  return true;
}

const std::string wex::ellipsed(
  const std::string& text, const std::string& control, bool ellipse)
{
  return text + 
    (ellipse ? "...": std::string()) + 
    (!control.empty() ? "\t" + control: std::string());
}

const std::string wex::firstof(
  const std::string& text, 
  const std::string& chars, 
  size_t start_pos,
  firstof_t flags)
{
  const auto pos = !flags[FIRST_OF_FROM_END] ? 
    text.find_first_of(chars, start_pos):
    text.find_last_of(chars, start_pos);

  if (!flags[FIRST_OF_BEFORE])
  {
    return pos == std::string::npos ?
      std::string():
      text.substr(pos + 1);
  }
  else
  {
    return pos == std::string::npos ?
      text:
      text.substr(0, pos);
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

const std::string wex::get_find_result(const std::string& find_text, 
  bool find_next, bool recursive)
{
  if (!recursive)
  {
    const auto where = (find_next) ? _("bottom").ToStdString(): _("top").ToStdString();
    return
      _("Searching for").ToStdString() + " " + 
      quoted(skip_white_space(find_text)) + " " +
      _("hit").ToStdString() + " " + where;
  }
  else
  {
    if (config(_("Error bells")).get(true))
    {
      wxBell();
    }
    return
      quoted(skip_white_space(find_text)) + " " + _("not found").ToStdString();
  }
}

const char wex::get_field_separator()
{
  return '\x0B';
}

int wex::get_iconid(const path& filename)
{
  return filename.file_exists() ? 
    wxFileIconsTable::file: (filename.dir_exists() ? 
    wxFileIconsTable::folder: 
    wxFileIconsTable::computer);
}

int wex::get_number_of_lines(const std::string& text, bool trim)
{
  if (text.empty())
  {
    return 0;
  }
  
  const auto trimmed = (trim ? skip_white_space(text): text);
  
  if (const int c = std::count(trimmed.begin(), trimmed.end(), '\n') + 1; c != 1)
  {
    return c;
  }
  
  return std::count(trimmed.begin(), trimmed.end(), '\r') + 1;
}

const std::string wex::get_string_set(
  const std::set<std::string>& kset, size_t min_size, const std::string& prefix)
{
  return std::accumulate(kset.begin(), kset.end(), std::string{}, 
    [&](const std::string& a, const std::string& b) {
      return (b.size() >= min_size && b.find(prefix) == 0) ? a + b + ' ': a;});
}

// See also get_modification_time in stat.cpp
std::tuple <bool, time_t> wex::get_time(
  const std::string& text, const std::string& format)
{
  time_t t;
#ifdef __WXMSW__
  wxDateTime dt; 
  if (!dt.ParseFormat(text, format)) return {false, 0}; 
  t = dt.GetTicks();
#else
  std::tm tm = { 0 };
  std::stringstream ss(text);
  ss >> std::get_time(&tm, format.c_str());
  
  if (ss.fail())
  {
    return {false, 0};
  }
  
  if ((t = mktime(&tm)) == -1) return {false, 0};
#endif

  return {true, t};
}

const std::string wex::get_word(std::string& text,
  bool use_other_field_separators,
  bool use_path_separator)
{
  std::string field_separators = " \t";
  if (use_other_field_separators) field_separators += ":";
  if (use_path_separator) field_separators = wxFILE_SEP_PATH;
  std::string token;
  tokenizer tkz(text, field_separators);
  if (tkz.has_more_tokens()) token = tkz.get_next_token();
  text = tkz.get_string();
  text = skip_white_space(text, skip_t().set(SKIP_LEFT));
  return token;
}

bool wex::is_brace(int c) 
{
  return c == '[' || c == ']' ||
         c == '(' || c == ')' ||
         c == '{' || c == '}' ||
         c == '<' || c == '>';
}
         
bool wex::is_codeword_separator(int c) 
{
  return isspace(c) || is_brace(c) || 
         c == ',' || c == ';' || c == ':' || c == '@';
}

long wex::make(const path& makefile)
{
  wex::process* process = new wex::process;

  return process->execute(
    config("Make").get("make") + " " +
      config("MakeSwitch").get("-f") + " " +
      makefile.data().string(),
    process::EXEC_NO_WAIT,
    makefile.get_path());
}

bool wex::marker_and_register_expansion(ex* ex, std::string& text)
{
  if (ex == nullptr) return false;

  for (tokenizer tkz(text, "'" + std::string(1, WXK_CONTROL_R), false); tkz.has_more_tokens(); )
  {
    tkz.get_next_token();
    
    if (const auto rest(tkz.get_string()); !rest.empty())
    {
      // Replace marker.
      if (const char name(rest[0]); tkz.last_delimiter() == '\'')
      {
        if (const auto line = ex->marker_line(name); line >= 0)
        {
          replace_all(text, 
            tkz.last_delimiter() + std::string(1, name), 
            std::to_string(line + 1));
        }
        else
        {
          return false;
        }
      }
      // Replace register.
      else
      {
        replace_all(text,
          tkz.last_delimiter() + std::string(1, name), 
          name == '%' ? ex->get_stc()->get_filename().fullname(): ex->get_macros().get_register(name));
      }
    }
  }
  
  return true;
}
  
int wex::match(const std::string& reg, const std::string& text, 
  std::vector < std::string > & v)
{
  try 
  {
    if (
       std::match_results<std::string::const_iterator> m;
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
    log(e) << reg << "code:" << e.code();
    return -1;
  }
}

bool wex::matches_one_of(const std::string& fullname, const std::string& pattern)
{
  if (pattern == "*") return true; // asterix matches always.

  // Make a regex of pattern matching chars.
  auto re(pattern); 
  replace_all(re, ".", "\\.");
  replace_all(re, "*", ".*");
  replace_all(re, "?", ".?");
  
  for (tokenizer tkz(re, ";"); tkz.has_more_tokens(); )
  {
    if (std::regex_match(fullname, std::regex(tkz.get_next_token()))) return true;
  }
  
  return false;
}

void wex::node_properties(const pugi::xml_node* node, std::vector<property>& properties)
{
  for (const auto& child: node->children())
  {
    if (strcmp(child.name(), "property") == 0)
    {
      properties.emplace_back(child);
    }
  }
}

void wex::node_styles(const pugi::xml_node* node, const std::string& lexer,
  std::vector<style>& styles)
{
  for (const auto& child: node->children())
  {
    if (strcmp(child.name(), "style") == 0)
    {
      styles.emplace_back(child, lexer);
    }
  }
}

bool wex::one_letter_after(const std::string& text, const std::string& letter)
{
  return std::regex_match(letter, std::regex("^" + text + "[a-zA-Z]$"));
}

int wex::open_files(
  frame* frame, 
  const std::vector< path > & files,
  const stc_data& stc_data, 
  dir::type_t type)
{
  wxWindowUpdateLocker locker(frame);
  
  int count = 0;
  
  for (const auto& it : files)
  {
    if (
      it.data().string().find("*") != std::string::npos || 
      it.data().string().find("?") != std::string::npos)
    {
      count += open_file_dir(frame, 
        path::current(),
        it.data().string(), stc_data.flags(), type).find_files();
    }
    else
    {
      path fn(it);
      wex::stc_data data(stc_data);

      if (!it.file_exists() && it.data().string().find(":") != std::string::npos)
      {
        if (const path& val(link().get_path(it.data().string(), data.control()));
          !val.data().empty())
        {
          fn = val;
        }
      }

      if (!fn.file_exists())
      {
        fn.make_absolute();
      }
       
      if (frame->open_file(fn, data) != nullptr)
      {
        count++;
      }
    }
  }
  
  return count;
}

void wex::open_files_dialog(frame* frame,
  long style, 
  const std::string& wildcards, 
  bool ask_for_continue,
  const stc_data& data, 
  dir::type_t type)
{
  wxArrayString paths;
  const std::string caption(_("Select Files"));
      
  if (auto* stc = frame->get_stc(); stc != nullptr)
  {
    file_dialog dlg(
      &stc->get_file(),
      window_data().style(style).title(caption),
      wildcards);

    if (ask_for_continue)
    {
      if (dlg.show_modal_if_changed(true) == wxID_CANCEL) return;
    }
    else
    {
      if (dlg.ShowModal() == wxID_CANCEL) return;
    }
      
    dlg.GetPaths(paths);
  }
  else
  {
    wxFileDialog dlg(frame,
      caption,
      wxEmptyString,
      wxEmptyString,
      wildcards,
      style);

    if (dlg.ShowModal() == wxID_CANCEL) return;

    dlg.GetPaths(paths);
  }

  open_files(frame, to_vector_path(paths).get(), data, type);
}

const std::string wex::print_caption(const path& filename)
{
  return filename.data().string();
}

const std::string wex::print_footer()
{
  return _("Page @PAGENUM@ of @PAGESCNT@").ToStdString();
}

const std::string wex::print_header(const path& filename)
{
  if (filename.file_exists())
  {
    return
      get_endoftext(
        filename.data().string() + " " +
        wxDateTime(filename.stat().st_mtime).Format().ToStdString(), 
        filename.lexer().line_size());
  }
  else
  {
    return _("Printed").ToStdString() + ": " + wxDateTime::Now().Format().ToStdString();
  }
}

const std::string wex::quoted(const std::string& text)
{
  return "'" + text + "'";
}

bool wex::regafter(const std::string& text, const std::string& letter)
{
  return std::regex_match(letter, std::regex("^" + text + "[0-9=\"a-z%._]$"));
}

int wex::replace_all(std::string& text, 
  const std::string& search,
  const std::string& replace,
  int* match_pos) 
{
  int count = 0;
  bool update = false;

  for (size_t pos = 0; (pos = text.find(search, pos)) != std::string::npos; ) 
  {
    if (match_pos != nullptr && !update)
    {
      *match_pos = (int)pos;
      update = true;
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
const std::string GetLines(std::vector<std::string> & lines,
  size_t pos, size_t len, InputIterator ii)
{
  std::string text;
  
  for (auto it : lines)
  {
    text += it.replace(pos, len, *ii);
    ++ii;
  }

  return text;
}
    
bool wex::shell_expansion(std::string& command)
{
  std::vector <std::string> v;
  const std::string re_str("`(.*?)`"); // non-greedy
  const std::regex re(re_str);
  
  while (match(re_str, command, v) > 0)
  {
    process process;
    if (!process.execute(v[0], process::EXEC_WAIT)) return false;
    
    command = std::regex_replace(
      command, 
      re, 
      process.get_stdout(), 
      std::regex_constants::format_sed);
  }
  
  return true;
}

bool wex::single_choice_dialog(wxWindow* parent, const std::string& title, 
  const wxArrayString& s, std::string& selection)
{
  wxSingleChoiceDialog dlg(parent, _("Input") + ":", title, s);

  if (const auto index = s.Index(selection); 
    index != wxNOT_FOUND) dlg.SetSelection(index);
  if (dlg.ShowModal() == wxID_CANCEL) return false;

  selection = dlg.GetStringSelection();
  
  return true;
}
  
const std::string wex::skip_white_space(
  const std::string& text, 
  skip_t type,
  const std::string& replace_with)
{
  auto output(text);
  
  if (type[SKIP_MID])
  {
    output = std::regex_replace(output, 
      std::regex("[ \t\n\v\f\r]+"), replace_with, std::regex_constants::format_sed);
  }
  
  if (type[SKIP_LEFT])
  {
    output = std::regex_replace(output, 
      std::regex("^[ \t\n\v\f\r]+"), "", std::regex_constants::format_sed);
  }

  if (type[SKIP_RIGHT])
  {
    output = std::regex_replace(output, 
      std::regex("[ \t\n\v\f\r]+$"), "", std::regex_constants::format_sed);
  }
  
  return output;
}

const std::string wex::sort(
  const std::string& input, 
  string_sort_t sort_t, 
  size_t pos, 
  const std::string& eol, 
  size_t len)
{
  wxBusyCursor wait;

  // Empty lines are not kept after sorting, as they are used as separator.
  std::map<std::string, std::string> m;
  std::multimap<std::string, std::string> mm;
  std::multiset<std::string> ms;
  std::vector<std::string> lines;
  
  for (tokenizer tkz(input, eol); tkz.has_more_tokens(); )
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
      text = (sort_t[STRING_SORT_UNIQUE] ?
        GetColumn(m.rbegin(), m.rend()):
        GetColumn(mm.rbegin(), mm.rend()));
    }
    else
    {
      text = (sort_t[STRING_SORT_UNIQUE] ?
        GetColumn(m.begin(), m.end()):
        GetColumn(mm.begin(), mm.end()));
    }
  }
  else
  {
    text = (sort_t[STRING_SORT_DESCENDING] ? 
      GetLines(lines, pos, len, ms.rbegin()):
      GetLines(lines, pos, len, ms.begin()));
  }
  
  return text;
}

bool wex::sort_selection(
  stc* stc,
  string_sort_t sort_t, 
  size_t pos, 
  size_t len)
{
  const auto start_pos = stc->GetSelectionStart();
  
  if (start_pos == -1 || 
    pos > (size_t)stc->GetSelectionEnd() || pos == std::string::npos)
  {
    return false;
  }
  
  bool error = false;
  stc->BeginUndoAction();
  
  try
  {
    if (stc->SelectionIsRectangle())
    {
      const auto start_pos_line = 
        stc->PositionFromLine(stc->LineFromPosition(start_pos));
      const auto end_pos_line = 
        stc->PositionFromLine(stc->LineFromPosition(stc->GetSelectionEnd()) + 1);
      const auto nr_lines = 
        stc->LineFromPosition(stc->GetSelectionEnd()) - 
        stc->LineFromPosition(start_pos);
        
      const auto sel = stc->GetTextRange(start_pos_line, end_pos_line); 
      stc->DeleteRange(start_pos_line, end_pos_line - start_pos_line);
      
      const auto text(sort(
        sel.ToStdString(), 
        sort_t, 
        pos, 
        stc->eol(), 
        len));
      
      stc->InsertText(start_pos_line, text);

      stc->SetCurrentPos(start_pos);
      stc->SelectNone();      
      for (size_t j = 0; j < len; j++)
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
      const auto text(sort(
        stc->GetSelectedText().ToStdString(), 
        sort_t, 
        pos, 
        stc->eol(), 
        len));
      
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
  
const std::string wex::translate(const std::string& text, 
  int pageNum, int numPages)
{
  auto translation(text);

  replace_all(translation, "@PAGENUM@", std::to_string(pageNum));
  replace_all(translation, "@PAGESCNT@", std::to_string(numPages));

  return translation;
}

void wex::vcs_command_stc(const vcs_command& command, 
  const lexer& lexer, stc* stc)
{
  if (command.is_blame())
  {
    // Do not show an edge for blamed documents, they are too wide.
    stc->SetEdgeMode(wxSTC_EDGE_NONE);
  }
  
  if (command.is_diff())
  {
    stc->get_lexer().set("diff");
  }
  else if (command.is_history())
  {
    stc->get_lexer().reset();
  }
  else if (command.is_open())
  {
    stc->get_lexer().set(lexer, true); // fold
  }
  else
  {
    stc->get_lexer().reset();
  }
}

void wex::vcs_execute(frame* frame, int id, const std::vector< path > & files)
{
  if (files.empty()) return;
  
  if (vcs vcs(files, id); vcs.entry().get_command().is_open())
  {
    if (vcs.show_dialog() == wxID_OK)
    {
      for (const auto& it : files)
      {
        if (wex::vcs vcs({it}, id); vcs.execute())
        {
          if (!vcs.entry().get_stdout().empty())
          {
            frame->open_file(it, vcs.entry());
          }
          else if (!vcs.entry().get_stderr().empty())
          {
            log() << vcs.entry().get_stderr();
          }
          else
          {
            log::status("No output");
            log::verbose("no output from") << vcs.entry().get_command_executed();
          }
        }
      }
    }
  }
  else
  {
    vcs.request();
  }
}

void wex::xml_error(
  const path& filename, 
  const pugi::xml_parse_result* result,
  stc* stc)
{
  log::status("Xml error") << result->description();
  log(*result) << filename.name();

  // prevent recursion
  if (stc == nullptr && filename != lexers::get()->get_filename())
  {
    if (auto* frame = wxDynamicCast(wxTheApp->GetTopWindow(), managed_frame);
      frame != nullptr)
    {
      stc = frame->open_file(filename);
    }
  }

  if (stc != nullptr && result->offset != 0)
  {
    stc->get_vi().command("gg");
    stc->get_vi().command(std::to_string(result->offset) + "|");
  }
}
