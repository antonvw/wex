/******************************************************************************\
* File:          extension.cpp
* Purpose:       Implementation of wxWidgets extension methods
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/extension/extension.h>

const wxString GetUnFormattedText(
  const exFileName& filename,
  const wxString& lines,
  const wxString& header,
  bool is_comment)
{
  const size_t line_length = filename.GetLexer().UsableCharactersPerLine();

  // Use the header, with one space extra to separate, or no header at all.
  const wxString header_with_spaces =
    (header.length() == 0) ? wxString(wxEmptyString) : wxString(' ', header.length());

  wxString in = lines, line = header;

  bool at_begin = true;

  wxString out;

  while (!in.empty())
  {
    const wxString word = exGetWord(in, false, false);

    if (line.length() + 1 + word.length() > line_length)
    {
      const wxString& newline =
        (is_comment ? filename.GetLexer().MakeComment(line, true, true): line);

      out << newline;

      line = header_with_spaces + word;
      at_begin = true;
    }
    else
    {
      line += (!line.empty() && !at_begin ? " ": wxString(wxEmptyString)) + word;
      at_begin = false;
    }
  }

  const wxString& newline =
    (is_comment ? filename.GetLexer().MakeComment(line, true, true): line);

  out << newline;

  return out;
}

const wxString GetFormattedText(
  const exFileName& filename,
  const wxString& lines,
  const wxString& header,
  bool is_comment)
{
  wxString text = lines, header_to_use = header;
  size_t nCharIndex;

  wxString out;

  // Process text between the carriage return line feeds.
  while ((nCharIndex = text.find("\n")) != wxString::npos)
  {
    out << GetUnFormattedText(
      filename,
      text.substr(0, nCharIndex),
      header_to_use,
      is_comment);

    text = text.substr(nCharIndex + 1);
    header_to_use = wxString(' ', header.length());
  }

  if (!text.empty())
  {
    out << GetUnFormattedText(
      filename,
      text,
      header_to_use,
      is_comment);
  }

  return out;
}

const wxString exGetTextWithPrefix(
  const exFileName& filename,
  const wxString& text,
  const wxString& prefix,
  bool is_comment) 
{
  wxString out;

  text.find("\n") != wxString::npos ?
    out << GetFormattedText(filename, text, prefix, is_comment):
    out << GetUnFormattedText(filename, text, prefix, is_comment);

  return out;
}

const wxString exHeader(
  const exFileName& filename,
  exConfig* config,
  const wxString& description)
{
  wxString header;

  const wxString author = config->Get("Header/Author");
  const wxString address = config->Get("Header/Address");
  const wxString company = config->Get("Header/Company");
  const wxString country = config->Get("Header/Country");
  const wxString place = config->Get("Header/Place");
  const wxString zipcode = config->Get("Header/Zipcode");

  const exLexer l = filename.GetLexer();

  header << l.MakeComment(wxEmptyString, true) << "\n";
  header << l.MakeComment("File:        " + filename.GetFullName(), true) << "\n";
  header << exGetTextWithPrefix(filename, description, "Purpose:     ") << "\n";
  header << l.MakeComment("Author:      " + author, true) << "\n";
  header << l.MakeComment("Created:     " + wxDateTime::Now().Format("%Y/%m/%d %H:%M:%S"), true) << "\n";

  if (config->GetBool("SVN"))
  {
    // Prevent the Id to be expanded by SVN itself here.
    header << l.MakeComment("RCS-ID:      $" + wxString("Id$"), true) << "\n";
  }

  header << l.MakeComment(wxEmptyString, true, true) << "\n";
  header << l.MakeComment(
    "Copyright (c) " + wxDateTime::Now().Format("%Y") + 
    (!company.empty() ? " " + company: wxString(wxEmptyString))
      + ". All rights reserved.", true) << "\n";

  if (!address.empty() && !country.empty() && !place.empty() && !zipcode.empty())
  {
    header << l.MakeComment(address + ", " + zipcode + " " + place + ", " + country, true) << "\n";
  }

  header << l.MakeComment(wxEmptyString, true) << "\n";

  header << "\n";

  if (filename.GetExt() == "h" && 
      filename.GetStat().st_size == 0)
  {
    wxString argument = "__" + filename.GetName() + "_h";

    header << "\n";
    header << "#if !defined (" << argument << ")" << "\n";
    header << "#define " << argument << "\n";
    header << "#endif" << "\n";
  }

  return header;
}

void exOpenFile(const wxFileName& filename, long open_flags)
{
  if (wxTheApp == NULL)
  {
    return;
  }

  wxWindow* window = wxTheApp->GetTopWindow();
  exFrame* frame = wxDynamicCast(window, exFrame);

  if (frame != NULL)
  {
    frame->OpenFile(exFileName(filename.GetFullPath()), -1, wxEmptyString, open_flags);
  }
}
