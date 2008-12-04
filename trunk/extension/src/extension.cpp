/******************************************************************************\
* File:          extension.cpp
* Purpose:       Implementation of wxWidgets extension methods
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2008 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/clipbrd.h>
#include <wx/colordlg.h>
#include <wx/dir.h>
#include <wx/stdpaths.h>
#include <wx/textfile.h> // for wxTextFile::GetEOL()
#include <wx/tokenzr.h>
#include <wx/extension/extension.h>
#include <wx/extension/stc.h>

bool exApp::m_Logging = false;
wxString exApp::m_CatalogDir;
exConfig* exApp::m_Config = NULL;
exLexers* exApp::m_Lexers = NULL;
wxLocale exApp::m_Locale;
#if wxUSE_HTML & wxUSE_PRINTING_ARCHITECTURE
wxHtmlEasyPrinting* exApp::m_Printer = NULL;
#endif

int exApp::OnExit()
{
#if wxUSE_GUI
  exSTC::CleanUp();
#endif

  delete m_Lexers;
  delete m_Config;
#if wxUSE_HTML & wxUSE_PRINTING_ARCHITECTURE
  delete m_Printer;
#endif
  return wxApp::OnExit();
}

bool exApp::OnInit()
{
  // Init the localization, from now on things will be translated.
  // So do this before constructing config and exTool::Initialize, as these use localization.
  if (m_Locale.Init())
  {
    // If there are catalogs in the catalog_dir, then add them to the locale.
    wxArrayString files;

    // README: We use the canonical name, also for windows, not sure whether that is
    // the best.
    m_CatalogDir = wxStandardPaths::Get().GetLocalizedResourcesDir(
      m_Locale.GetCanonicalName(),
      // This seems to be necessarty for wxGTK. For wxMSW it isn't used.
      wxStandardPaths::ResourceCat_Messages);

    if (wxFileName::DirExists(m_CatalogDir))
    {
      wxDir::GetAllFiles(m_CatalogDir, &files);

      for (size_t i = 0 ; i < files.GetCount(); i++)
      {
        // Default the wxstd is already loaded by m_Locale.Init(),
        // so do not do it twice.
        const wxFileName fn(files.Item(i));

        if (!m_Locale.IsLoaded(fn.GetName()))
        {
          if (!m_Locale.AddCatalog(fn.GetName()))
          {
            wxLogError("Catalog could not be added: " + fn.GetName());
          }
        }
      }
    }
  }

  // Now construct the config, as most classes use it.
#ifdef EX_PORTABLE
  m_Config = new exConfig(
    wxFileName(
      wxFileName(wxStandardPaths::Get().GetExecutablePath()).GetPath(),
      wxTheApp->GetAppName() + wxString(".cfg")).GetFullPath());
#else
  m_Config = new exConfig();
#endif

  // And construct and read the lexers.
  m_Lexers = new exLexers();
  m_Lexers->Read();

#if wxUSE_HTML & wxUSE_PRINTING_ARCHITECTURE
  m_Printer = new wxHtmlEasyPrinting();
#endif

  // Finally call all available static initializers.
  exTool::Initialize();
  exSTC::PathListInit();

  return wxApp::OnInit();
}

#if wxUSE_GUI
void exBackgroundColourDialog(wxWindow* parent, wxWindow* win)
{
  wxColourData data;
  data.SetColour(win->GetBackgroundColour());
  wxColourDialog dlg(parent, &data);

  if (dlg.ShowModal())
  {
    win->SetBackgroundColour(dlg.GetColourData().GetColour());
    win->Refresh();
  }
}

bool exSvnDialog(int svn_type)
{
  wxString caption;
  wxString svn_command;

  switch (svn_type)
  {
    case SVN_COMMIT: 
      caption = _("Commit"); 
      svn_command = "commit";
      break;
    case SVN_STAT: 
      caption = _("Stat"); 
      svn_command = "stat";
      break;
    case SVN_LOG: 
      caption = _("Log"); 
      svn_command = "log";
      break;
  }
  
  std::vector<exConfigItem> v;
  if (svn_type == SVN_COMMIT) v.push_back(exConfigItem(_("Revision comment")));
  v.push_back(exConfigItem(_("Base folder"), CONFIG_COMBOBOXDIR, wxEmptyString, true));

  if (exConfigDialog(wxTheApp->GetTopWindow(),
    v,
    caption).ShowModal() == wxID_CANCEL)
  {
    return false;
  }

  const wxString cwd = wxGetCwd();
  wxSetWorkingDirectory(exApp::GetConfig(_("Base folder")));  
  wxArrayString output;
  wxArrayString errors;
  
  wxString arg;
  if (svn_type == SVN_COMMIT) arg = " -m \"" + exApp::GetConfig(_("Revision comment")) + "\"";
  
  wxExecute(
    "svn " + svn_command + arg,
    output,
    errors);
    
  wxSetWorkingDirectory(cwd);

  wxString msg;

  for (size_t i = 0; i < output.GetCount(); i++)
  {
    // Take care that we have only one space between output lines.
    msg += output[i] + "\n";
  }
  
  wxLogMessage(msg);
                  
  return true;
}

bool exClipboardAdd(const wxString& text)
{
  wxClipboardLocker locker;
  if (!locker) return false;
  if (!wxTheClipboard->AddData(new wxTextDataObject(text))) return false;
  if (!wxTheClipboard->Flush()) return false; // take care that clipboard data remain after exiting
  return true;
}

const wxString exClipboardGet()
{
  wxBusyCursor wait;
  wxClipboardLocker locker;

  if (!locker)
  {
    wxLogError("Cannot open clipboard");
    return wxEmptyString;
  }

  if (!wxTheClipboard->IsSupported(wxDF_TEXT))
  {
    wxLogError("Clipboard format not supported");
    return wxEmptyString;
  }

  wxTextDataObject data;
  if (!wxTheClipboard->GetData(data))
  {
    wxLogError("Cannot get clipboard data");
    return wxEmptyString;
  }

  return data.GetText();
}

long exColourToLong(const wxColour& c)
{
  return c.Red() | (c.Green() << 8) | (c.Blue() << 16);
}

void exComboBoxFromString(
  wxComboBox* cb,
  const wxString& text,
  const wxChar field_separator)
{
  wxStringTokenizer tkz(text, field_separator);
  while (tkz.HasMoreTokens())
  {
    const wxString val = tkz.GetNextToken();
    if (cb->FindString(val) == wxNOT_FOUND)
    {
      cb->Append(val);
    }
  }

  if (cb->GetCount() > 0) cb->SetValue(cb->GetString(0));
}

bool exComboBoxToString(
  const wxComboBox* cb,
  wxString& text,
  const wxChar field_separator,
  size_t max_items)
{
  if (cb == NULL)
  {
    return false;
  }

  text = cb->GetValue();
  switch (cb->FindString(cb->GetValue()))
  {
    case wxNOT_FOUND:
    {
      // Add the string, as it is not in the combo box, to the text,
      // simply by appending all combobox items.
      for (size_t i = 0; i < max_items; i++)
        if (i < max_items - 1 && i < cb->GetCount())
          text += field_separator + cb->GetString(i);
    }
    break;
    // No change necessary, the string is already present as the first one.
    case 0: return false; break;
    default:
    {
      // Reorder. The new first element already present, just add all others.
      for (size_t i = 0; i < cb->GetCount(); i++)
      {
        const wxString cb_element = cb->GetString(i);
        if (cb_element != cb->GetValue())
          text += field_separator + cb_element;
      }
    }
  }

  return true;
}

const wxString exEllipsed(const wxString& text, const wxString& control)
{
  return text + "..." + (!control.empty() ? "\t" + control: wxString(wxEmptyString));
}

const wxString exGetEndOfWord(
  const wxString& text,
  size_t max_chars)
{
  wxString text_out(text);

  if (text_out.length() > max_chars)
  {
    text_out = "..." + text_out.substr(text_out.length() - max_chars);
  }

  return text_out;
}

int exGetNumberOfLines(const wxString& text)
{
  if (text.empty())
  {
    return 0;
  }
  else if (text.Find(wxChar('\r')) != wxNOT_FOUND)
  {
    return text.Freq(wxChar('\r')) + 1;
  }
  else if (text.Find(wxChar('\n')) != wxNOT_FOUND)
  {
    return text.Freq(wxChar('\n')) + 1;
  }
  else
  {
    return 1;
  }
}

const wxString exGetWord(
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

int exGetLineNumberFromText(const wxString& text)
{
  // Get text after :.
  const size_t pos_char = text.rfind(":");

  if (pos_char == wxString::npos)
  {
    return 0;
  }

  const wxString linenumber = text.substr(pos_char + 1);

  long line;

  if (linenumber.ToLong(&line))
  {
    return line;
  }
  else
  {
    return 0;
  }
}

bool exIsBrace(int ch)
{
  return ch == '[' || ch == ']' ||
         ch == '(' || ch == ')' ||
         ch == '{' || ch == '}' ||
         ch == '<' || ch == '>';
};

bool exIsCodewordSeparator(int c)
{
  return (isspace(c) || exIsBrace(c) || c == ',' || c == ';' || c == ':');
}

bool exIsWordCharacter(wxChar c)
{
  return isalnum(c) || c == '_';
}

void exLog(const wxString& text, const wxFileName& filename)
{
  wxFile(
    filename.GetFullPath(), 
    wxFile::write_append).Write(
      wxDateTime::Now().Format() + " " + text + wxTextFile::GetEOL());
}

const wxFileName exLogfileName()
{
#ifdef EX_PORTABLE
  return wxFileName(
    wxPathOnly(wxStandardPaths::Get().GetExecutablePath()),
    wxTheApp->GetAppName().Lower() + ".log");
#else
  return wxFileName(
    wxStandardPaths::Get().GetUserDataDir(),
    wxTheApp->GetAppName().Lower() + ".log");
#endif
}

bool exMatchesOneOf(const wxFileName& filename, const wxString& pattern)
{
  if (pattern == "*") return true; // asterix matches always.

  const wxString fullname_uppercase = filename.GetFullName().Upper();

  wxStringTokenizer tokenizer(pattern.Upper(), ";");
  while (tokenizer.HasMoreTokens())
  {
    if (fullname_uppercase.Matches(tokenizer.GetNextToken())) return true;
  }

  return false;
}

void exOpenFile(const wxFileName& filename, long open_flags)
{
  wxWindow* window = wxTheApp->GetTopWindow();
  exFrame* frame = wxDynamicCast(window, exFrame);

  if (frame != NULL)
  {
    frame->OpenFile(filename.GetFullPath(), -1, wxEmptyString, open_flags);
  }
}

const wxString exSkipWhiteSpace(const wxString& text, const wxString& replace_with)
{
  wxString output = text;
  wxRegEx("[ \t\n]+").ReplaceAll(&output, replace_with);
  return output;
}

#if wxUSE_STATUSBAR
void exStatusText(const exFileName& filename, long flags)
{
  wxString text; // clear status bar for empty or not existing or not initialized file names

  if (filename.IsOk())
  {
    const wxString path = (flags & STAT_FULLPATH
      ? filename.GetFullPath(): filename.GetFullName());

    text += path;

    if (filename.GetStat().IsOk())
    {
      const wxString what = (flags & STAT_SYNC
        ? _("Synchronized"): _("Modified"));
      const wxString time = (flags & STAT_SYNC
        ? wxDateTime::Now().Format(): filename.GetStat().GetModificationTime());
      text += " " + what + " " + time;
    }
  }

  exFrame::StatusText(text);
}
#endif // wxUSE_STATUSBAR

const wxString exTranslate(const wxString& text, int pageNum, int numPages)
{
  wxString translation = text;
  wxString num;

  num.Printf("%i", pageNum);
  translation.Replace("@PAGENUM@", num);

  num.Printf("%i", numPages);
  translation.Replace("@PAGESCNT@", num);

  return translation;
}
#endif // wxUSE_GUI
