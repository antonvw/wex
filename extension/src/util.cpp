////////////////////////////////////////////////////////////////////////////////
// Name:      util.cpp
// Purpose:   Implementation of wxExtension utility methods
// Author:    Anton van Wezenbeek
// Copyright: (c) 2014 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <shunting-yard/shunting-yard.h>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <regex>
#include <wx/clipbrd.h>
#include <wx/combobox.h>
#include <wx/config.h>
#include <wx/filename.h>
#include <wx/filehistory.h>
#include <wx/generic/dirctrlg.h> // for wxTheFileIconsTable
#include <wx/regex.h>
#include <wx/stdpaths.h>
#include <wx/textctrl.h>
#include <wx/tokenzr.h>
#include <wx/wupdlock.h>
#include <wx/xml/xml.h>
#include <wx/extension/util.h>
#include <wx/extension/configitem.h>
#include <wx/extension/configdlg.h>
#include <wx/extension/dir.h>
#include <wx/extension/ex.h>
#include <wx/extension/filedlg.h>
#include <wx/extension/filename.h>
#include <wx/extension/frame.h>
#include <wx/extension/frd.h>
#include <wx/extension/lexer.h>
#include <wx/extension/process.h>
#include <wx/extension/stc.h>
#include <wx/extension/vcs.h>
#include <wx/extension/vimacros.h>

wxExToVectorString::wxExToVectorString(const wxArrayString& in)
{
  FromArrayString(in);
}

wxExToVectorString::wxExToVectorString(const wxFileDialog& in)
{
  wxArrayString paths;
  in.GetPaths(paths);
  FromArrayString(paths);
}

wxExToVectorString::wxExToVectorString(const wxFileHistory& in, int count)
{
  for (int i = 0; i < count && i < in.GetCount(); i++)
  {
    m_VS.push_back(in.GetHistoryFile(i));
  }  
}

wxExToVectorString::wxExToVectorString(const wxGenericDirCtrl& in)
{
  wxArrayString paths;
  in.GetPaths(paths);
  FromArrayString(paths);
}

wxExToVectorString::wxExToVectorString(const wxString& in)
{
  wxStringTokenizer tkz(in);
      
  while (tkz.HasMoreTokens())
  {
    m_VS.push_back(tkz.GetNextToken());
  }
}

void wxExToVectorString::FromArrayString(const wxArrayString& in)
{
  for (const auto& it : in)
  {
    m_VS.push_back(it);
  }
}

const wxString wxExAlignText(
  const wxString& lines,
  const wxString& header,
  bool fill_out_with_space,
  bool fill_out,
  const wxExLexer& lexer)
{
  const size_t line_length = lexer.UsableCharactersPerLine();

  // Use the header, with one space extra to separate, or no header at all.
  const wxString header_with_spaces =
    (header.empty()) ? wxString(wxEmptyString) : wxString(' ', header.size());

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
      line += (!line.empty() && !at_begin ? wxString(" "): wxString(wxEmptyString)) + word;
      at_begin = false;
    }
  }

  out << lexer.MakeSingleLineComment(line, fill_out_with_space, fill_out);

  return out;
}

bool wxExAutoComplete(
  const wxString& text, 
  const std::vector<wxString> & v,
  wxString& s)
{
  int matches = 0;
  
  for (auto& it : v)
  {
    if (it.StartsWith(text))
    {
      s = it;
      matches++;
    }
  }

  return (matches == 1);
}
  
bool wxExAutoCompleteFileName(
  const wxString& text, 
  std::vector<wxString> & v)
{
  const wxString path(text.AfterLast(' '));
  const wxString word(path.AfterLast(wxFileName::GetPathSeparator()));
  
  wxString subdir = path.BeforeLast(wxFileName::GetPathSeparator());
  
  if (!subdir.empty())
  {
    subdir = wxFileName::GetPathSeparator() + subdir;
  }
  
  wxDir dir(wxGetCwd() + subdir);
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
  v.push_back(expansion);
  v.push_back(filename);
    
  while (dir.GetNext(&filename))
  {
    v.push_back(filename);
  }

  if (v.size() > 2)
  {
    int rest_equal_size = 0;
    bool all_ok = true;
      
    for (int i = word.length(); i < v[1].size() && all_ok; i++)
    {
      for (int j = 2; j < v.size() && all_ok; j++)
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

#ifdef DEBUG  
    wxLogMessage("rest equal size: %d, word: %s", 
      rest_equal_size, word.c_str());
#endif
  
    v[0] = v[1].Mid(word.length(), rest_equal_size);
  }

  return true;
}

double wxExCalculator(const wxString& text, wxExEx* ex, int& width)
{
  wxString expr(text);
  expr.Trim();
  if (expr.empty())
  {
    return 0;
  }
  
  // Determine the width.
  wxRegEx re(",[0-9]+");
  
  if (re.Matches(text))
  {
    const wxString match = re.GetMatch(expr);
    
    if (!match.empty())
    {
      width = match.length() - 1;
    }
  }
  else
  {
    width = 0;
  }
  
  expr.Replace(".", wxString::Format("%d", ex->GetSTC()->GetCurrentLine() + 1));
  expr.Replace("$", wxString::Format("%d", ex->GetSTC()->GetLineCount()));
  
  wxStringTokenizer tkz(expr, "'" + wxString(wxUniChar(WXK_CONTROL_R)));

  while (tkz.HasMoreTokens())
  {
    tkz.GetNextToken();
    
    const wxString rest(tkz.GetString());
    
    if (!rest.empty())
    {
      if (tkz.GetLastDelimiter() == '\'')
      {
        const int line = ex->MarkerLine(rest.GetChar(0));
        
        if (line >= 0)
        {
          expr.Replace(tkz.GetLastDelimiter() + wxString(rest.GetChar(0)), 
            wxString::Format("%d", line + 1));
        }
      }
      else
      {
        expr.Replace(tkz.GetLastDelimiter() + wxString(rest.GetChar(0)), 
          ex->GetMacros().GetRegister(rest.GetChar(0)));
      }
    }
  }

  // https://github.com/bmars/shunting-yard/
  // https://github.com/bamos/cpp-expression-parser
  try
  {
    return calculator::calculate(expr);
  }
  catch(std::domain_error e)
  {
    wxLogError(e.what());
    return 0;
  }
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

const wxString wxExClipboardGet()
{
  wxClipboardLocker locker;

  if (!locker)
  {
    wxLogStatus("Cannot open clipboard");
    return wxEmptyString;
  }

  if (!wxTheClipboard->IsSupported(wxDF_TEXT))
  {
    return wxEmptyString;
  }

  wxTextDataObject data;
  
  if (!wxTheClipboard->GetData(data))
  {
    wxLogStatus("Cannot get clipboard data");
    return wxEmptyString;
  }

  return data.GetText();
}

#if wxUSE_GUI
void wxExComboBoxFromList(
  wxComboBox* cb,
  const std::list < wxString > & text)
{
  if (!text.empty())
  {
    wxASSERT(cb != NULL);
    cb->Clear();

    wxArrayString as;
    as.resize(text.size());
    copy(text.begin(), text.end(), as.begin());
    
    cb->Append(as);
    cb->SetValue(cb->GetString(0));
  }
}

const std::list < wxString > wxExComboBoxToList(
  const wxComboBox* cb,
  size_t max_items)
{
  wxASSERT(cb != NULL);

  std::list < wxString > l{cb->GetValue()};

  switch (cb->FindString(
    cb->GetValue(),
    true)) // case sensitive
  {
    case 0: 
      // No change necessary, the string is already present as the first one.
      for (size_t i = 1; i < cb->GetCount() && i < max_items; i++)
        l.push_back(cb->GetString(i));
      break;

    case wxNOT_FOUND:
      // Add the string, as it is not in the combo box, to the text,
      // simply by appending all combobox items.
      for (size_t i = 0; i < cb->GetCount() && i < max_items - 1; i++)
        l.push_back(cb->GetString(i));
    break;

    default:
      // Reorder. The new first element already present, just add all others.
      for (size_t i = 0; i < cb->GetCount(); i++)
      {
        const wxString cb_element = cb->GetString(i);
        if (cb_element != cb->GetValue())
          l.push_back(cb_element);
      }
  }

  return l;
}

#endif // wxUSE_GUI

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

const wxString wxExConfigFirstOf(const wxString& key)
{
  return 
    wxConfigBase::Get()->Read(key).BeforeFirst(wxExGetFieldSeparator());
}

const wxString wxExEllipsed(const wxString& text, const wxString& control)
{
  return 
    text + "..." + 
      (!control.empty() ? "\t" + control: wxString(wxEmptyString));
}

const wxString wxExGetEndOfText(
  const wxString& text,
  size_t max_chars)
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

const wxString wxExGetFindResult(
  const wxString& find_text,
  bool find_next, 
  bool recursive)
{
  wxString text;
  
  if (!recursive)
  {
    const wxString where = (find_next) ? _("bottom"): _("top");
    text <<
      _("Searching for") << " " << 
      wxExQuoted(wxExSkipWhiteSpace(find_text)) << " " <<
      _("hit") << " " << where;
  }
  else
  {
    wxBell();
      
    text << 
      wxExQuoted(wxExSkipWhiteSpace(find_text)) << " " << _("not found");
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

long wxExGetHexNumberFromUser(
  const wxString& message,
  const wxString& prompt,
  const wxString& caption,
  long value,
  long min,
  long max,
  wxWindow *parent,
  const wxPoint& pos)
{
  wxConfigBase::Get()->Write(message, value);

  wxExConfigDialog dlg(
    parent,
    std::vector<wxExConfigItem>{wxExConfigItem(
      message, 
      min, 
      max, 
      wxEmptyString,
      CONFIG_SPINCTRL_HEX)},
    caption,
    0,
    1,
    wxOK | wxCANCEL);
  
  if (dlg.ShowModal() == wxID_CANCEL)
  {
    return -1;
  }
  
  return wxConfigBase::Get()->ReadLong(message, 80);
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

const wxString wxExGetWord(
  wxString& text,
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

const std::list < wxString > wxExListFromConfig(
  const wxString& config)
{
  wxStringTokenizer tkz(
    wxConfigBase::Get()->Read(config), 
    wxExGetFieldSeparator());

  std::list < wxString > l;

  while (tkz.HasMoreTokens())
  {
    l.push_back(tkz.GetNextToken());
  }

  return l;
}

/// Saves entries from a list with strings to the config.
void wxExListToConfig(
  const std::list < wxString > & l, 
  const wxString& config)
{
  wxString text;
  const int commandsSaveInConfig = 75;
  int items = 0;

  for (
    auto it = l.begin();
    it != l.end() && items < commandsSaveInConfig;
    ++it)
  {
    text += *it + wxExGetFieldSeparator();
    items++;
  }

  wxConfigBase::Get()->Write(config, text);
}

void wxExLogStatus(const wxExFileName& fn, long flags)
{
  if (!fn.IsOk())
  {
    wxLogStatus(wxEmptyString);
    return;
  }
  
  wxString text = (flags & STAT_FULLPATH ? 
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
  wxExProcess process;

  return process.Execute(
    wxConfigBase::Get()->Read("Make", "make") + " " +
      wxConfigBase::Get()->Read("MakeSwitch", "-f") + " " +
      makefile.GetFullPath(),
    wxEXEC_ASYNC,
    makefile.GetPath());
}

int wxExMatch(
  const wxString& reg, 
  const wxString& text, 
  std::vector < wxString > & v)
{
  v.clear();
  
#ifdef __WXMSW__
// gcc 4.7.3 gives error code: 4
  try 
  {
    std::match_results<std::string::const_iterator> res;
    std::regex rx(reg);
    std::string str(text.c_str());
    std::regex_search(str, res, rx);
    
    if (res.size() > 1)
    {
      for (int i = 1; i < res.size(); i++)
      {
        v.push_back(wxString(res[i]));
      }
    }
  }
  catch (std::regex_error& e) 
  {
    wxLogError(wxString::Format("%s: in: %s code: %d",
      e.what(), reg.c_str(), e.code()));
  }
#else
  wxRegEx regex(reg, wxRE_ADVANCED);

  if (regex.Matches(text))
  {
    for (int i = 0; i < regex.GetMatchCount() - 1; i++)
    {
      v.push_back(regex.GetMatch(text, i + 1));
    }
  }
#endif

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

void wxExNodeProperties(
  const wxXmlNode* node,
  std::vector<wxExProperty>& properties)
{
  wxXmlNode *child = node->GetChildren();

  while (child)
  {
    if (child->GetName() == "property")
    {
      properties.push_back(wxExProperty(child));
    }
    
    child = child->GetNext();
  }
}

void wxExNodeStyles(
  const wxXmlNode* node,
  const wxString& lexer,
  std::vector<wxExStyle>& styles)
{
  wxXmlNode* child = node->GetChildren();

  while (child)
  {
    if (child->GetName() == "style")
    {
      styles.push_back(wxExStyle(child, lexer));
    }
    
    child = child->GetNext();
  }
}

#if wxUSE_GUI
void wxExOpenFiles(
  wxExFrame* frame,
  const std::vector< wxString > & files,
  long file_flags,
  int dir_flags)
{
  wxWindowUpdateLocker locker(frame);
  
  for (const auto& it : files)
  {
    if (it.Contains("*") || it.Contains("?"))
    {
      wxExDirOpenFile dir(frame, wxGetCwd(), it, file_flags, dir_flags);
      dir.FindFiles();
    }
    else
    {
      wxString file(it);
      int line_no = 0;
      int col_no = 0;

      if (!wxFileName(file).FileExists() && file.Contains(":"))
      {
        const wxString val = wxExLink().GetPath(file, line_no, col_no);
        
        if (!val.empty())
        {
          file = val;
        }
      }

      if (wxFileName(file).FileExists())
      {
        frame->OpenFile(file, line_no, wxEmptyString, col_no, file_flags);
      }
    }
  }
}

void wxExOpenFilesDialog(
  wxExFrame* frame,
  long style,
  const wxString& wildcards,
  bool ask_for_continue,
  long file_flags,
  int dir_flags)
{
  wxExSTC* stc = frame->GetSTC();
  wxArrayString paths;
  const wxString caption(_("Select Files"));
      
  if (stc != NULL)
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

const wxString wxExSkipWhiteSpace(
  const wxString& text,
  const wxString& replace_with)
{
  wxString output = text;
  wxRegEx("[ \t\n\v\f\r]+").ReplaceAll(&output, replace_with);
  output.Trim(true);
  output.Trim(false);
  return output;
}

bool wxExSetTextCtrlValue(
  wxTextCtrl* ctrl,
  int key,
  const std::list < wxString > & l,
  std::list < wxString >::const_iterator & it)
{
  if (l.empty())
  {
    return false;
  }
  
  switch (key)
  {
  case WXK_UP:
    if (it != l.end())
    {
      it++;
    }
    break;
  case WXK_DOWN:
    if (it != l.begin())
    {
      it--;
    }
    break;
  default:
    return false;
  }

  ctrl->SetValue(it != l.end() ? *it: wxString(wxEmptyString));
  ctrl->SetInsertionPointEnd();
  
  return true;
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

void wxExVCSCommandOnSTC(
  const wxExVCSCommand& command, 
  const wxExLexer& lexer,
  wxExSTC* stc)
{
  if (command.IsBlame())
  {
    // Do not show an edge for blamed documents, they are too wide.
    stc->SetEdgeMode(wxSTC_EDGE_NONE);
  }
  
  if (command.IsDiff())
  {
    stc->SetLexer("diff");
  }
  else if (command.IsHistory())
  {
    stc->ResetLexer();
  }
  else if (command.IsOpen())
  {
    stc->SetLexer(lexer, true); // fold
  }
  else
  {
    stc->ResetLexer();
  }
}

void wxExVCSExecute(
  wxExFrame* frame, int id, const std::vector< wxString > & files)
{
  wxExVCS vcs(files, id);
  
  if (vcs.GetEntry().GetCommand().IsOpen() && !files.empty())
  {
    if (vcs.ShowDialog(frame) == wxID_OK)
    {
      for (const auto& it : files)
      {
        wxExVCS vcs({it}, id);
        
        if (vcs.Execute())
        {
          frame->OpenFile(it, vcs.GetEntry(), wxExSTC::STC_WIN_READ_ONLY);
        }
      }
    }
  }
  else
  {
    vcs.Request(frame);
  }
}
