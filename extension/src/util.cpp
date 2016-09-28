////////////////////////////////////////////////////////////////////////////////
// Name:      util.cpp
// Purpose:   Implementation of wxExtension utility methods
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <numeric>
#include <regex>
#include <wx/app.h>
#include <wx/clipbrd.h>
#include <wx/config.h>
#include <wx/filename.h>
#include <wx/generic/dirctrlg.h> // for wxTheFileIconsTable
#include <wx/stdpaths.h>
#include <wx/textctrl.h>
#include <wx/tokenzr.h>
#include <wx/wupdlock.h>
#include <wx/xml/xml.h>
#include <wx/extension/util.h>
#include <wx/extension/dir.h>
#include <wx/extension/ex.h>
#include <wx/extension/filedlg.h>
#include <wx/extension/filename.h>
#include <wx/extension/frame.h>
#include <wx/extension/frd.h>
#include <wx/extension/lexer.h>
#include <wx/extension/process.h>
#include <wx/extension/stc.h>
#include <wx/extension/tostring.h>
#include <wx/extension/vcs.h>
#include <wx/extension/vimacros.h>

const wxString wxExAlignText(const wxString& lines, const wxString& header,
  bool fill_out_with_space, bool fill_out, const wxExLexer& lexer)
{
  const size_t line_length = lexer.UsableCharactersPerLine();

  // Use the header, with one space extra to separate, or no header at all.
  const wxString header_with_spaces =
    (header.empty()) ? wxString() : wxString(' ', header.size());

  wxString in = lines, line = header;

  bool at_begin = true;
  wxString out;

  while (!in.empty())
  {
    const wxString word = wxExGetWord(in, false, false);

    if (line.size() + 1 + word.size() > line_length)
    {
      out << lexer.MakeSingleLineComment(line, fill_out_with_space, fill_out) << "\n";

      line = header_with_spaces + word;
    }
    else
    {
      line += (!line.empty() && !at_begin ? wxString(" "): wxString()) + word;
      at_begin = false;
    }
  }

  out << lexer.MakeSingleLineComment(line, fill_out_with_space, fill_out);

  return out;
}

bool wxExAutoComplete(const std::string& text, 
  const std::vector<std::string> & v, std::string& s)
{
  int matches = 0;
  
  for (const auto& it : v)
  {
    if (wxString(it).StartsWith(text))
    {
      s = it;
      matches++;
    }
  }

  return (matches == 1);
}
  
const wxString Encode(const wxString& text)
{
  wxString output(text);
  
  if (output.Contains(" "))
  {
    output.Replace(" ", "\\ ");
  }
  
  return output;
}
  
bool wxExAutoCompleteFileName(const wxString& text, std::vector<wxString> & v)
{
  // E.g.:
  // 1) text: src/vi
  // -> should build list with files in ./src starting with vi
  // path:   src
  // word:   vi
  // 2) text: /usr/include/s
  // ->should build list with files in /usr/include starting with s
  // path:   /usr/include
  // word:   s
  // And text might be prefixed by a command, e.g.: e src/vi
  wxFileName path(text.AfterLast(' '));
  
  if (path.IsRelative())
  {
    if (!path.MakeAbsolute())
    {
      return false;
    }
  }
  
  if (!path.DirExists())
  {
    return false;
  }
  
  const wxString word(
    text.AfterLast(' ').AfterLast(wxFileName::GetPathSeparator()));
  wxDir dir(path.GetPath());
  wxString filename;

  if (!dir.IsOpened() || !dir.GetFirst(&filename, word + "*"))
  {
    return false;
  }
  
  wxString expansion = filename.Mid(word.length());
  
  if (wxDirExists(dir.GetNameWithSep() + filename))
  {
    expansion += wxFileName::GetPathSeparator();
  }

  v.clear();
  v.emplace_back(Encode(expansion));
  v.emplace_back(filename);
    
  while (dir.GetNext(&filename))
  {
    v.emplace_back(Encode(filename));
  }

  if (v.size() > 2)
  {
    int rest_equal_size = 0;
    bool all_ok = true;
      
    for (size_t i = word.length(); i < v[1].size() && all_ok; i++)
    {
      for (size_t j = 2; j < v.size() && all_ok; j++)
      {
        if (i < v[j].size() && 
            v[1].GetChar(i) != v[j].GetChar(i))
        {
          all_ok = false;
        }
      }
    
      if (all_ok)
      {
        rest_equal_size++;
      }
    }

    v[0] = v[1].Mid(word.length(), rest_equal_size);
  }

  return true;
}

bool wxExClipboardAdd(const wxString& text)
{
  wxClipboardLocker locker;
  if (!locker) return false;
  if (!wxTheClipboard->AddData(new wxTextDataObject(text))) return false;

  // Take care that clipboard data remain after exiting
  // This is a boolean method as well, we don't check it, as
  // clipboard data is copied.
  // At least on Ubuntu 8.10 FLush returns false.
  wxTheClipboard->Flush();

  return true;
}

const std::string wxExClipboardGet()
{
  wxClipboardLocker locker;

  if (!locker)
  {
    wxLogStatus("Cannot open clipboard");
    return std::string();
  }

  if (!wxTheClipboard->IsSupported(wxDF_TEXT))
  {
    return std::string();
  }

  wxTextDataObject data;
  
  if (!wxTheClipboard->GetData(data))
  {
    wxLogStatus("Cannot get clipboard data");
    return std::string();
  }

  return data.GetText().ToStdString();
}

#if wxUSE_GUI
void wxExComboBoxFromList(wxComboBox* cb, const std::list < wxString > & text)
{
  wxExComboBoxAs<const std::list < wxString >>(cb, text);
}
#endif

bool wxExCompareFile(const wxFileName& file1, const wxFileName& file2)
{
  if (wxConfigBase::Get()->Read(_("Comparator")).empty())
  {
    return false;
  }

  const wxString arguments =
     (file1.GetModificationTime() < file2.GetModificationTime()) ?
       "\"" + file1.GetFullPath() + "\" \"" + file2.GetFullPath() + "\"":
       "\"" + file2.GetFullPath() + "\" \"" + file1.GetFullPath() + "\"";

  if (wxExecute(wxConfigBase::Get()->Read(_("Comparator")) + " " + arguments) == 0)
  {
    return false;
  }

  wxLogStatus(_("Compared") + ": " + arguments);

  return true;
}

const wxString wxExConfigDir()
{
#ifdef __WXMSW__
  return wxPathOnly(wxStandardPaths::Get().GetExecutablePath());
#else
  return wxFileName(
    wxGetHomeDir() + wxFileName::GetPathSeparator() + ".config",
    wxTheApp->GetAppName().Lower()).GetFullPath();
#endif
}
  
const wxString wxExConfigFirstOf(const wxString& key)
{
  return 
    wxConfigBase::Get()->Read(key).BeforeFirst(wxExGetFieldSeparator());
}

const wxString wxExConfigFirstOfWrite(const wxString& key, const wxString& value)
{
  wxStringTokenizer tkz(wxConfigBase::Get()->Read(key),
    wxExGetFieldSeparator());
  
  std::vector<wxString> v{value};

  while (tkz.HasMoreTokens())
  {
    const wxString val = tkz.GetNextToken();
    
    if (val != value)
    {
      v.emplace_back(val);
    }
  }

  wxConfigBase::Get()->Write(key, std::accumulate(v.begin(), v.end(), wxString{}, 
    [&](const wxString& a, const wxString& b) {
      return a + b + wxExGetFieldSeparator();}));
  
  return value;
}
  
const wxString wxExEllipsed(const wxString& text, const wxString& control)
{
  return text + "..." + 
    (!control.empty() ? "\t" + control: wxString());
}

const wxString wxExGetEndOfText(const wxString& text, size_t max_chars)
{
  wxString text_out(text);

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

const wxString wxExGetFindResult(const wxString& find_text, 
  bool find_next, bool recursive)
{
  wxString text;
  
  if (!recursive)
  {
    const wxString where = (find_next) ? _("bottom"): _("top");
    text <<
      _("Searching for") << " " << 
      wxExQuoted(wxExSkipWhiteSpace(find_text.ToStdString())) << " " <<
      _("hit") << " " << where;
  }
  else
  {
    wxBell();
      
    text << 
      wxExQuoted(wxExSkipWhiteSpace(find_text.ToStdString())) << " " << _("not found");
  }
  
  return text;
}

const wxUniChar wxExGetFieldSeparator()
{
  return '\x0B';
}

int wxExGetIconID(const wxFileName& filename)
{
  if (filename.FileExists() || 
      filename.DirExists(filename.GetFullPath()))
  {
    if (filename.DirExists(filename.GetFullPath()))
    {
      return wxFileIconsTable::folder;
    }
    else if (!filename.GetExt().empty())
    {
      return wxTheFileIconsTable->GetIconID(filename.GetExt());
    }
    else
    {
      return wxFileIconsTable::file;
    }
  }
  else
  {
    return wxFileIconsTable::computer;
  }
}

int wxExGetNumberOfLines(const wxString& text, bool trim)
{
  if (text.empty())
  {
    return 0;
  }
  
  wxString trimmed(text);
  
  if (trim)
  {
    trimmed.Trim();
  }
  
  const int c = std::count(trimmed.begin(), trimmed.end(), '\n') + 1;
  
  if (c != 1)
  {
    return c;
  }
  
  return std::count(trimmed.begin(), trimmed.end(), '\r') + 1;
}

const wxString wxExGetWord(wxString& text,
  bool use_other_field_separators,
  bool use_path_separator)
{
  wxString field_separators = " \t";
  if (use_other_field_separators) field_separators += ":";
  if (use_path_separator) field_separators = wxFILE_SEP_PATH;
  wxString token;
  wxStringTokenizer tkz(text, field_separators);
  if (tkz.HasMoreTokens()) token = tkz.GetNextToken();
  text = tkz.GetString();
  text.Trim(false);
  return token;
}

bool wxExIsBrace(int c) 
{
  return c == '[' || c == ']' ||
         c == '(' || c == ')' ||
         c == '{' || c == '}' ||
         c == '<' || c == '>';
}
         
bool wxExIsCodewordSeparator(int c) 
{
  return isspace(c) || wxExIsBrace(c) || 
         c == ',' || c == ';' || c == ':' || c == '@';
}

const std::list < wxString > wxExListFromConfig(const wxString& config)
{
  wxStringTokenizer tkz(
    wxConfigBase::Get()->Read(config), 
    wxExGetFieldSeparator());

  std::list < wxString > l;

  while (tkz.HasMoreTokens())
  {
    l.emplace_back(tkz.GetNextToken());
  }

  return l;
}

/// Saves entries from a list with strings to the config.
void wxExListToConfig(const std::list < wxString > & l, const wxString& config)
{
  if (l.empty()) return;

  wxString text;
  const int commandsSaveInConfig = 75;
  int items = 0;

  for (const auto& it : l)
  {
    if (items++ > commandsSaveInConfig) break;
    text += it + wxString(wxExGetFieldSeparator());
  }
  
  wxConfigBase::Get()->Write(config, text);
}

void wxExLogStatus(const wxExFileName& fn, long flags)
{
  if (!fn.IsOk())
  {
    return;
  }
  
  wxString text = ((flags & STAT_FULLPATH) ? 
    fn.GetFullPath(): 
    fn.GetFullName());

  if (fn.GetStat().IsOk())
  {
    const wxString what = ((flags & STAT_SYNC) ? 
      _("Synchronized"): 
      _("Modified"));
        
    text += " " + what + " " + fn.GetStat().GetModificationTime();
  }

  wxLogStatus(text);
}

long wxExMake(const wxFileName& makefile)
{
  wxExProcess* process = new wxExProcess;

  return process->Execute(
    wxConfigBase::Get()->Read("Make", "make") + " " +
      wxConfigBase::Get()->Read("MakeSwitch", "-f") + " " +
      makefile.GetFullPath(),
    wxEXEC_ASYNC,
    makefile.GetPath());
}

bool wxExMarkerAndRegisterExpansion(wxExEx* ex, wxString& text)
{
  if (ex == nullptr) return false;

  wxStringTokenizer tkz(text, "'" + wxString(wxUniChar(WXK_CONTROL_R)));

  while (tkz.HasMoreTokens())
  {
    tkz.GetNextToken();
    
    const wxString rest(tkz.GetString());
    
    if (!rest.empty())
    {
      const char name(rest.GetChar(0));
      
      // Replace marker.
      if (tkz.GetLastDelimiter() == '\'')
      {
        const int line = ex->MarkerLine(name);
        
        if (line >= 0)
        {
          text.Replace(
            tkz.GetLastDelimiter() + wxString(name), 
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
        text.Replace(
          tkz.GetLastDelimiter() + wxString(name), 
          name == '%' ? ex->GetSTC()->GetFileName().GetFullName(): ex->GetMacros().GetRegister(name));
      }
    }
  }
  
  return true;
}
  
int wxExMatch(const std::string& reg, const std::string& text, 
  std::vector < wxString > & v)
{
  v.clear();
  
  try 
  {
    std::match_results<std::string::const_iterator> res;
    std::regex rx(reg);
    std::regex_search(text, res, rx);
    
    if (res.size() > 1)
    {
      for (size_t i = 1; i < res.size(); i++)
      {
        v.emplace_back(wxString(res[i]));
      }
    }
  }
  catch (std::regex_error& e) 
  {
    wxLogError(wxString::Format("%s: in: %s code: %d",
      e.what(), reg.c_str(), e.code()));
  }

  return v.size();
}

bool wxExMatchesOneOf(const wxFileName& filename, const wxString& pattern)
{
  if (pattern == "*") return true; // asterix matches always.

  const wxString fullname_uppercase = filename.GetFullName().Upper();

  wxStringTokenizer tkz(pattern.Upper(), ";");
  
  while (tkz.HasMoreTokens())
  {
    const wxString token = tkz.GetNextToken();
    
    if (fullname_uppercase.Matches(token)) return true;
  }
  
  return false;
}

void wxExNodeProperties(const wxXmlNode* node, std::vector<wxExProperty>& properties)
{
  wxXmlNode *child = node->GetChildren();

  while (child)
  {
    if (child->GetName() == "property")
    {
      properties.emplace_back(wxExProperty(child));
    }
    
    child = child->GetNext();
  }
}

void wxExNodeStyles(const wxXmlNode* node, const wxString& lexer,
  std::vector<wxExStyle>& styles)
{
  wxXmlNode* child = node->GetChildren();

  while (child)
  {
    if (child->GetName() == "style")
    {
      styles.emplace_back(wxExStyle(child, lexer));
    }
    
    child = child->GetNext();
  }
}

#if wxUSE_GUI
int wxExOpenFiles(wxExFrame* frame, const std::vector< wxString > & files,
  long file_flags, int dir_flags, const wxString& command)
{
  wxWindowUpdateLocker locker(frame);
  
  int count = 0;
  
  for (const auto& it : files)
  {
    if (it.Contains("*") || it.Contains("?"))
    {
      count += wxExDirOpenFile(frame, 
        wxGetCwd(), it, file_flags, dir_flags).FindFiles();
    }
    else
    {
      wxString file(it);
      int line_no = 0;
      int col_no = 0;
      
      wxFileName fn(file);

      if (!fn.FileExists() && file.Contains(":"))
      {
        const wxString val = wxExLink().GetPath(file.ToStdString(), line_no, col_no);
        
        if (!val.empty())
        {
          fn.Assign(val);
        }
      }

      if (!fn.FileExists())
      {
        fn.MakeAbsolute();
      }
      
      fn.FileExists() ?
        frame->OpenFile(fn, line_no, wxEmptyString, col_no, file_flags, command):
        frame->OpenFile(fn, wxEmptyString, file_flags);
      
      count++;
    }
  }
  
  return count;
}

void wxExOpenFilesDialog(wxExFrame* frame,
  long style, const wxString& wildcards, bool ask_for_continue,
  long file_flags, int dir_flags)
{
  wxExSTC* stc = frame->GetSTC();
  wxArrayString paths;
  const wxString caption(_("Select Files"));
      
  if (stc != nullptr)
  {
    wxExFileDialog dlg(frame,
      &stc->GetFile(),
      caption,
      wildcards,
      style);

    if (ask_for_continue)
    {
      if (dlg.ShowModalIfChanged(true) == wxID_CANCEL) return;
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

  wxExOpenFiles(frame, wxExToVectorString(paths).Get(), file_flags, dir_flags);
}
#endif // wxUSE_GUI

const wxString wxExPrintCaption(const wxFileName& filename)
{
  return filename.GetFullPath();
}

const wxString wxExPrintFooter()
{
  return _("Page @PAGENUM@ of @PAGESCNT@");
}

const wxString wxExPrintHeader(const wxFileName& filename)
{
  if (filename.FileExists())
  {
    return
      wxExGetEndOfText(
        filename.GetFullPath() + " " +
        filename.GetModificationTime().Format(), 
        80);
  }
  else
  {
    return _("Printed") + ": " + wxDateTime::Now().Format();
  }
}

const wxString wxExQuoted(const wxString& text)
{
  return "'" + text + "'";
}

template <typename InputIterator>
const wxString GetColumn(InputIterator first, InputIterator last)
{
  wxString text;
  
  for (InputIterator it = first; it != last; ++it) 
  {
    text += it->second;
  }

  return text;
}
    
template <typename InputIterator>
const wxString GetLines(std::vector<wxString> & lines,
  size_t pos, size_t len, InputIterator ii)
{
  wxString text;
  
  for (auto it : lines)
  {
    text += it.replace(pos, len, *ii);
    ++ii;
  }

  return text;
}
    
bool wxExShellExpansion(wxString& command)
{
  std::vector <wxString> v;
  const std::string re_str("`(.*?)`"); // non-greedy
  const std::regex re(re_str);
  
  while (wxExMatch(re_str, command.ToStdString(), v) > 0)
  {
    wxExProcess process;
    if (!process.Execute(v[0], wxEXEC_SYNC)) return false;
    
    command = std::regex_replace(
      command.ToStdString(), 
      re, 
      process.GetOutput().ToStdString(), 
      std::regex_constants::format_sed);
  }
  
  return true;
}

const std::string wxExSkipWhiteSpace(const std::string& text, const std::string& replace_with)
{
  std::string output1 = std::regex_replace(text, 
    std::regex("[ \t\n\v\f\r]+"), replace_with, std::regex_constants::format_sed);
  
  std::string output2 = std::regex_replace(output1, 
    std::regex("^ +"), "", std::regex_constants::format_sed);

  std::string output3 = std::regex_replace(output2, 
    std::regex(" +$"), "", std::regex_constants::format_sed);
  
  return output3;
}

const wxString wxExSort(const wxString& input, 
  size_t sort_type, size_t pos, const wxString& eol, size_t len)
{
  wxBusyCursor wait;

  // Empty lines are not kept after sorting, as they are used as separator.
  wxStringTokenizer tkz(input, eol);
  std::map<wxString, wxString> m;
  std::multimap<wxString, wxString> mm;
  std::multiset<wxString> ms;
  std::vector<wxString> lines;
  
  while (tkz.HasMoreTokens())
  {
    const wxString line = tkz.GetNextToken() + eol;
    
    // Use an empty key if line is to short.
    wxString key;
    
    if (pos < line.length())
    {
      key = line.substr(pos, len);
    }
    
    if (len == std::string::npos)
    {
      if (sort_type & STRING_SORT_UNIQUE)
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

  wxString text;

  if (len == std::string::npos)
  {
    if (sort_type & STRING_SORT_DESCENDING)
    {
      text = ((sort_type & STRING_SORT_UNIQUE) ?
        GetColumn(m.rbegin(), m.rend()):
        GetColumn(mm.rbegin(), mm.rend()));
    }
    else
    {
      text = ((sort_type & STRING_SORT_UNIQUE) ?
        GetColumn(m.begin(), m.end()):
        GetColumn(mm.begin(), mm.end()));
    }
  }
  else
  {
    text = ((sort_type & STRING_SORT_DESCENDING) ? 
      GetLines(lines, pos, len, ms.rbegin()):
      GetLines(lines, pos, len, ms.begin()));
  }
  
  return text;
}

bool wxExSortSelection(wxExSTC* stc,
  size_t sort_type, size_t pos, size_t len)
{
  const int start_pos = stc->GetSelectionStart();
  
  if (start_pos == -1 || pos == std::string::npos)
  {
    return false;
  }
  
  bool error = false;
  stc->BeginUndoAction();
  
  try
  {
    if (stc->SelectionIsRectangle())
    {
      const int start_pos_line = stc->PositionFromLine(stc->LineFromPosition(start_pos));
      const int end_pos_line = stc->PositionFromLine(stc->LineFromPosition(stc->GetSelectionEnd()) + 1);
      const int nr_lines = 
        stc->LineFromPosition(stc->GetSelectionEnd()) - 
        stc->LineFromPosition(start_pos);
        
      const wxString sel = stc->GetTextRange(start_pos_line, end_pos_line); 
      stc->DeleteRange(start_pos_line, end_pos_line - start_pos_line);
      const wxString text(wxExSort(sel, sort_type, pos, stc->GetEOL(), len));
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
      const wxString text(wxExSort(stc->GetSelectedText(), sort_type, pos, stc->GetEOL(), len));
      stc->ReplaceSelection(text);
      stc->SetSelection(start_pos, start_pos + text.size());
    }
  }
  catch (std::exception& e)
  {
    wxLogError(e.what());
    error = true;
  }
  
  stc->EndUndoAction();
  
  return !error;
}
  
const wxString wxExTranslate(const wxString& text, int pageNum, int numPages)
{
  wxString translation = text;
  wxString num;

  num.Printf("%i", pageNum);
  translation.Replace("@PAGENUM@", num);

  num.Printf("%i", numPages);
  translation.Replace("@PAGESCNT@", num);

  return translation;
}

void wxExVCSCommandOnSTC(const wxExVCSCommand& command, 
  const wxExLexer& lexer, wxExSTC* stc)
{
  if (command.IsBlame())
  {
    // Do not show an edge for blamed documents, they are too wide.
    stc->SetEdgeMode(wxSTC_EDGE_NONE);
  }
  
  if (command.IsDiff())
  {
    stc->GetLexer().Set("diff");
  }
  else if (command.IsHistory())
  {
    stc->GetLexer().Reset();
  }
  else if (command.IsOpen())
  {
    stc->GetLexer().Set(lexer, true); // fold
  }
  else
  {
    stc->GetLexer().Reset();
  }
}

void wxExVCSExecute(wxExFrame* frame, int id, const std::vector< wxString > & files)
{
  wxASSERT(!files.empty());
  
  wxExVCS vcs(files, id);
  
  if (vcs.GetEntry().GetCommand().IsOpen())
  {
    if (vcs.ShowDialog(frame) == wxID_OK)
    {
      for (const auto& it : files)
      {
        wxExVCS vcs({it}, id);
        
        if (vcs.Execute())
        {
          frame->OpenFile(it, vcs.GetEntry());
        }
      }
    }
  }
  else
  {
    vcs.Request(frame);
  }
}
