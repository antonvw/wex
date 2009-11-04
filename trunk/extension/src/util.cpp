/******************************************************************************\
* File:          util.cpp
* Purpose:       Implementation of wxExtension utility methods
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/clipbrd.h>
#include <wx/config.h>
#include <wx/file.h>
#include <wx/regex.h>
#include <wx/stdpaths.h>
#include <wx/textfile.h> // for wxTextFile::GetEOL()
#include <wx/tokenzr.h>
#include <wx/extension/util.h>
#include <wx/extension/frame.h>
#include <wx/extension/frd.h>

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
    (header.size() == 0) ? wxEmptyString : wxString(' ', header.size());

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
      line += (!line.empty() && !at_begin ? wxString(" "): wxEmptyString) + word;
      at_begin = false;
    }
  }

  out << lexer.MakeSingleLineComment(line, fill_out_with_space, fill_out);

  return out;
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

#if wxUSE_GUI
void wxExComboBoxFromString(
  wxComboBox* cb,
  const wxString& text)
{
  wxASSERT(cb != NULL);

  wxStringTokenizer tkz(
    text, 
    wxExFindReplaceData::Get()->GetFieldSeparator());

  while (tkz.HasMoreTokens())
  {
    const wxString val = tkz.GetNextToken();
    if (cb->FindString(val) == wxNOT_FOUND)
    {
      cb->Append(val);
    }
  }

  if (cb->GetCount() > 0) 
  {
    cb->SetValue(cb->GetString(0));
  }
}

const wxString wxExComboBoxToString(
  const wxComboBox* cb,
  size_t max_items)
{
  wxASSERT(cb != NULL);

  wxString text = cb->GetValue();

  switch (cb->FindString(cb->GetValue()))
  {
    case 0: 
      text.clear();
      // No change necessary, the string is already present as the first one.
      for (size_t i = 0; i < max_items; i++)
        if (i < cb->GetCount())
          text += cb->GetString(i) + wxExFindReplaceData::Get()->GetFieldSeparator();
      break;

    case wxNOT_FOUND:
      // Add the string, as it is not in the combo box, to the text,
      // simply by appending all combobox items.
      for (size_t i = 0; i < max_items - 1; i++)
        if (i < cb->GetCount())
          text += wxExFindReplaceData::Get()->GetFieldSeparator() + cb->GetString(i);
    break;

    default:
      // Reorder. The new first element already present, just add all others.
      for (size_t i = 0; i < cb->GetCount(); i++)
      {
        const wxString cb_element = cb->GetString(i);
        if (cb_element != cb->GetValue())
          text += wxExFindReplaceData::Get()->GetFieldSeparator() + cb_element;
      }
  }

  return text;
}

const wxString wxExConfigFirstOf(const wxString& key)
{
  const wxString value = wxConfigBase::Get()->Read(key);
  return value.BeforeFirst(wxExFindReplaceData::Get()->GetFieldSeparator());
}

#endif // wxUSE_GUI

const wxString wxExEllipsed(const wxString& text, const wxString& control)
{
  return text + "..." + (!control.empty() ? "\t" + control: wxString(wxEmptyString));
}

void wxExFindResult(
  const wxString& find_text,
  bool find_next, 
  bool recursive)
{
  if (!recursive)
  {
    const wxString where = (find_next) ? _("bottom"): _("top");

#if wxUSE_STATUSBAR
    wxExFrame::StatusText(
      _("Searching for") + " " + wxExQuoted(wxExSkipWhiteSpace(find_text)) + " " + 
      _("hit") + " " + where);
#endif
  }
  else
  {
    wxBell();

#if wxUSE_STATUSBAR
    wxExFrame::StatusText(
      wxExQuoted(wxExSkipWhiteSpace(find_text)) + " " + _("not found"));
#endif
  }
}

const wxString wxExGetEndOfText(
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

int wxExGetNumberOfLines(const wxString& text)
{
  if (text.empty())
  {
    return 0;
  }
  else if (text.Find(wxUniChar('\r')) != wxNOT_FOUND)
  {
    return text.Freq(wxUniChar('\r')) + 1;
  }
  else if (text.Find(wxUniChar('\n')) != wxNOT_FOUND)
  {
    return text.Freq(wxUniChar('\n')) + 1;
  }
  else
  {
    return 1;
  }
}

int wxExGetLineNumberFromText(const wxString& text)
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

bool wxExMatchesOneOf(const wxFileName& filename, const wxString& pattern)
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

#if wxUSE_GUI
bool wxExOpenFile(const wxFileName& filename, long open_flags)
{
  wxASSERT(wxTheApp != NULL);

  wxWindow* window = wxTheApp->GetTopWindow();
  wxExFrame* frame = wxDynamicCast(window, wxExFrame);

  if (frame != NULL)
  {
    return frame->OpenFile(
      wxExFileName(filename.GetFullPath()), -1, wxEmptyString, open_flags);
  }
  else
  {
    return false;
  }
}
#endif

const wxString wxExPrintCaption(const wxFileName& filename)
{
  if (filename.FileExists())
  {
    return filename.GetFullPath();
  }
  else
  {
    return _("Printout");
  }
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
      wxExGetEndOfText(filename.GetFullPath(), 20) + " " +
      filename.GetModificationTime().Format() + " " +
      wxDateTime::Now().Format();
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

const wxString wxExSkipWhiteSpace(const wxString& text, const wxString& replace_with)
{
  wxString output = text;
  wxRegEx("[ \t\n]+").ReplaceAll(&output, replace_with);
  return output;
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
