/******************************************************************************\
* File:          util.cpp
* Purpose:       Implementation of wxextension utility methods
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/clipbrd.h>
#include <wx/regex.h>
#include <wx/stdpaths.h>
#include <wx/textfile.h> // for wxTextFile::GetEOL()
#include <wx/tokenzr.h>
#include <wx/extension/util.h>

#if wxUSE_GUI
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

#endif // wxUSE_GUI

long exColourToLong(const wxColour& c)
{
  return c.Red() | (c.Green() << 8) | (c.Blue() << 16);
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

void exLog(const wxString& text, const exFileName& filename)
{
  wxFile(
    filename.GetFullPath(), 
    wxFile::write_append).Write(
      wxDateTime::Now().Format() + " " + text + wxTextFile::GetEOL());
}

const exFileName exLogfileName()
{
  if (wxTheApp == NULL)
  {
    return exFileName("app.log");
  }

#ifdef EX_PORTABLE
  return exFileName(
    wxPathOnly(wxStandardPaths::Get().GetExecutablePath()) + wxFileName::GetPathSeparator() + 
    wxTheApp->GetAppName().Lower() + ".log");
#else
  return exFileName(
    wxStandardPaths::Get().GetUserDataDir() + wxFileName::GetPathSeparator() + 
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

const wxString exSkipWhiteSpace(const wxString& text, const wxString& replace_with)
{
  wxString output = text;
  wxRegEx("[ \t\n]+").ReplaceAll(&output, replace_with);
  return output;
}

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
