/******************************************************************************\
* File:          lexers.cpp
* Purpose:       Implementation of wxExLexers classes
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 2008-2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/config.h>
#include <wx/stdpaths.h>
#include <wx/tokenzr.h>
#include <wx/stc/stc.h>
#include <wx/textfile.h>
#include <wx/extension/lexers.h>
#include <wx/extension/frd.h>
#include <wx/extension/util.h> // for wxExMatchesOneOf

wxExLexers* wxExLexers::m_Self = NULL;

wxExLexers::wxExLexers(const wxFileName& filename)
  : m_FileName(filename)
{
}

const wxString wxExLexers::ApplyMacro(const wxString& text) const
{
  if (text.IsNumber() || text.Contains(","))
  {
    return text;
  }

  std::map<wxString, wxString>::const_iterator it = m_Macros.find(text);

  if (it != m_Macros.end())
  {
    return it->second;
  }
  else
  {
    wxLogError(_("Undefined macro: %s"), text.c_str());
    return text;
  }
}

const wxString wxExLexers::BuildWildCards(const wxFileName& filename) const
{
  const wxString allfiles_wildcard =
    _("All Files") + wxString::Format(" (%s)|%s",
      wxFileSelectorDefaultWildcardStr,
      wxFileSelectorDefaultWildcardStr);

  wxString wildcards = allfiles_wildcard;

  // Build the wildcard string using all available lexers.
  for (
    std::map<wxString, wxExLexer>::const_iterator it = m_Lexers.begin();
    it != m_Lexers.end();
    ++it)
  {
    if (!it->second.m_Extensions.empty())
    {
      const wxString wildcard =
        it->second.GetScintillaLexer() +
        " (" + it->second.m_Extensions + ") |" +
        it->second.m_Extensions;

      if (wxExMatchesOneOf(filename, it->second.m_Extensions))
      {
        wildcards = wildcard + "|" + wildcards;
      }
      else
      {
        wildcards = wildcards + "|" + wildcard;
      }
    }
  }

  return wildcards;
}

const wxExLexer wxExLexers::FindByFileName(const wxFileName& filename) const
{
  if (!filename.IsOk())
  {
    return wxExLexer();
  }

  for (
    std::map<wxString, wxExLexer>::const_iterator it = m_Lexers.begin();
    it != m_Lexers.end();
    ++it)
  {
    if (wxExMatchesOneOf(filename, it->second.m_Extensions))
    {
      return it->second;
    }
  }

  return wxExLexer();
}

const wxExLexer wxExLexers::FindByName(
  const wxString& name,
  bool fail_if_not_found) const
{
  std::map<wxString, wxExLexer>::const_iterator it = m_Lexers.find(name);

  if (it != m_Lexers.end())
  {
    return it->second;
  }

  if (!m_Lexers.empty() && fail_if_not_found)
  {
    wxFAIL;
  }

  return wxExLexer();
}

const wxExLexer wxExLexers::FindByText(const wxString& text) const
{
  // Add automatic lexers if text starts with some special tokens.
  const wxString text_lowercase = text.Lower();

  if (text_lowercase.StartsWith("#") ||
      // .po files that do not have comment headers, start with msgid, so set them
      text_lowercase.StartsWith("msgid"))
  {
    return FindByName("bash", false); // don't show error
  }
  else if (text_lowercase.StartsWith("<html>") ||
           text_lowercase.StartsWith("<?php"))
  {
    return FindByName("hypertext", false); // don't show error
  }
  else if (text_lowercase.StartsWith("<?xml"))
  {
    return FindByName("xml", false); // don't show error
  }
  // cpp files like #include <map> really do not have a .h extension (e.g. /usr/include/c++/3.3.5/map)
  // so add here.
  else if (text_lowercase.StartsWith("//"))
  {
    return FindByName("cpp", false); // don't show error
  }

  return wxExLexer();
}

wxExLexers* wxExLexers::Get(bool createOnDemand)
{
  if (m_Self == NULL && createOnDemand)
  {
    m_Self = new wxExLexers(wxFileName(
#ifdef wxExUSE_PORTABLE
      wxPathOnly(wxStandardPaths::Get().GetExecutablePath())
#else
      wxStandardPaths::Get().GetUserDataDir()
#endif
      + wxFileName::GetPathSeparator() + "lexers.xml")
    );

    m_Self->Read();
  }

  return m_Self;
}

const wxString wxExLexers::GetLexerAssociations() const
{
  wxString text;

  for (
    std::map<wxString, wxExLexer>::const_iterator it = m_Lexers.begin();
    it != m_Lexers.end();
    ++it)
  {
    if (!it->second.m_Extensions.empty())
    {
      if (!text.empty())
      {
        text += wxExFindReplaceData::Get()->GetFieldSeparator();
      }

      text += it->second.m_Extensions;
    }
  }

  return text;
}

const wxString wxExLexers::ParseTagColourings(const wxXmlNode* node) const
{
  wxString text;

  wxXmlNode* child = node->GetChildren();

  while (child)
  {
    if (child->GetName() == "colouring")
    {
      wxString content = child->GetNodeContent().Strip(wxString::both);

      std::map<wxString, wxString>::const_iterator it = m_Macros.find(content);

      if (it != m_Macros.end())
      {
        content = it->second;
      }

      text +=
        ApplyMacro(
          child->GetAttribute("no", "0"))
        + "=" + content + wxTextFile::GetEOL();
    }
    else if (child->GetName() == "comment")
    {
      // Ignore comments.
    }
    else
    {
      wxLogError(_("Undefined colourings tag: %s on: %d"),
        child->GetName().c_str(), child->GetLineNumber());
    }

    child = child->GetNext();
  }

  return text;
}

void wxExLexers::ParseTagGlobal(const wxXmlNode* node)
{
  wxXmlNode* child = node->GetChildren();

  while (child)
  {
    if (child->GetName() == "comment")
    {
      // Ignore comments.
    }
    else if (child->GetName() == "hex")
    {
      m_StylesHex.push_back(
        ApplyMacro(child->GetAttribute("no", "0")) + "=" +
        child->GetNodeContent().Strip(wxString::both));
    }
    else if (child->GetName() == "indicator")
    {
      m_Indicators.insert(std::make_pair(
        atoi(ApplyMacro(child->GetAttribute("no", "0")).c_str()),
        atoi(child->GetNodeContent().Strip(wxString::both).c_str())));
    }
    else if (child->GetName() == "marker")
    {
      const wxExMarker marker(ParseTagMarker(
        ApplyMacro(child->GetAttribute("no", "0")),
        child->GetNodeContent().Strip(wxString::both)));

      if (marker.GetMarkerNumber() < wxSTC_STYLE_MAX &&
          marker.GetMarkerSymbol() < wxSTC_STYLE_MAX)
      {
        m_Markers.push_back(marker);
      }
      else
      {
        wxLogError(_("Illegal marker number: %d or symbol: %d on: %d"),
          marker.GetMarkerNumber(),
          marker.GetMarkerSymbol(),
          child->GetLineNumber());
      }
    }
    else if (child->GetName() == "properties")
    {
      m_GlobalProperties = ParseTagProperties(child);
    }
    else if (child->GetName() == "style")
    {
      const wxString attrib = ApplyMacro(child->GetAttribute("no", "0"));
      const wxString content = child->GetNodeContent().Strip(wxString::both);
      int attrib_no = atoi(attrib.c_str());

      if (attrib_no == wxSTC_STYLE_DEFAULT)
      {
        m_DefaultStyle = attrib + "=" + content;
      }
      else
      {
        m_Styles.push_back(attrib + "=" + content);
      }
    }
    else
    {
      wxLogError(_("Undefined global tag: %s on: %d"),
        child->GetName().c_str(), child->GetLineNumber());
    }

    child = child->GetNext();
  }
}

const wxExLexer wxExLexers::ParseTagLexer(const wxXmlNode* node) const
{
  wxExLexer lexer;
  lexer.m_ScintillaLexer = node->GetAttribute("name", "");
  lexer.m_Extensions = node->GetAttribute(
    "extensions", 
    "*." + lexer.m_ScintillaLexer);

  wxXmlNode *child = node->GetChildren();

  while (child)
  {
    if (child->GetName() == "colourings")
    {
      lexer.m_Colourings = ParseTagColourings(child);
    }
    else if (child->GetName() == "keywords")
    {
      if (!lexer.SetKeywords(child->GetNodeContent().Strip(wxString::both)))
      {
        wxLogError(_("Keywords could not be set on: %d"), child->GetLineNumber());
      }
    }
    else if (child->GetName() == "properties")
    {
      lexer.m_Properties = ParseTagProperties(child);
    }
    else if (child->GetName() == "comments")
    {
      lexer.m_CommentBegin = child->GetAttribute("begin1", "");
      lexer.m_CommentEnd = child->GetAttribute("end1", "");
      lexer.m_CommentBegin2 = child->GetAttribute("begin2", "");
      lexer.m_CommentEnd2 = child->GetAttribute("end2", "");
    }
    else if (child->GetName() == "comment")
    {
      // Ignore comments.
    }
    else
    {
      wxLogError(_("Undefined lexer tag: %s on: %d"),
        child->GetName().c_str(), child->GetLineNumber());
    }

    child = child->GetNext();
  }

  return lexer;
}

void wxExLexers::ParseTagMacro(const wxXmlNode* node)
{
  wxXmlNode* child = node->GetChildren();

  while (child)
  {
    if (child->GetName() == "comment")
    {
      // Ignore comments.
    }
    else if (child->GetName() == "def")
    {
      const wxString attrib = child->GetAttribute("no", "0");
      const wxString content = child->GetNodeContent().Strip(wxString::both);
      m_Macros[attrib] = content;
    }
    else
    {
      wxLogError(_("Undefined macro tag: %s on: %d"),
        child->GetName().c_str(), child->GetLineNumber());
    }

    child = child->GetNext();
  }
}

const wxExMarker wxExLexers::ParseTagMarker(
  const wxString& number,
  const wxString& props) const
{
  wxStringTokenizer prop_fields(props, ",");

  const wxString symbol = ApplyMacro(prop_fields.GetNextToken());

  wxColour foreground;
  wxColour background;

  if (prop_fields.HasMoreTokens())
  {
    foreground = ApplyMacro(
      prop_fields.GetNextToken().Strip(wxString::both));

    if (prop_fields.HasMoreTokens())
    {
      background = ApplyMacro(
        prop_fields.GetNextToken().Strip(wxString::both));
    }
  }

  const int no = atoi(number.c_str());
  const int symbol_no = atoi(symbol.c_str());

  if (no <= wxSTC_MARKER_MAX && symbol_no <= wxSTC_MARKER_MAX)
  {
    return wxExMarker(no, symbol_no, foreground, background);
  }
  else
  {
    wxLogError(_("Illegal marker number: %d or symbol: %d"), no, symbol_no);
    return wxExMarker(0, 0, foreground, background);
  }
}

const wxString wxExLexers::ParseTagProperties(const wxXmlNode* node) const
{
  wxString text;

  wxXmlNode *child = node->GetChildren();

  while (child)
  {
    if (child->GetName() == "property")
    {
      text +=
        child->GetAttribute("name", "0") + "=" +
        child->GetNodeContent().Strip(wxString::both) + wxTextFile::GetEOL();
    }
    else if (child->GetName() == "comment")
    {
      // Ignore comments.
    }
    else
    {
      wxLogError(_("Undefined properties tag: %s on %d"),
        child->GetName().c_str(), child->GetLineNumber());
    }

    child = child->GetNext();
  }

  return text;
}

void wxExLexers::Read()
{
  // This test is to prevent showing an error if the lexers file does not exist,
  // as this is not required.
  if (!m_FileName.FileExists())
  {
    return;
  } 

  wxXmlDocument doc;

  if (!doc.Load(m_FileName.GetFullPath()))
  {
    return;
  }

  // Initialize members.
  m_Indicators.clear();
  m_Lexers.clear();
  m_Markers.clear();
  m_SortedLexerNames.clear();
  m_Styles.clear();
  m_StylesHex.clear();

  wxXmlNode* child = doc.GetRoot()->GetChildren();

  while (child)
  {
    if (child->GetName() == "macro")
    {
      ParseTagMacro(child);
    }
    else if (child->GetName() == "global")
    {
      ParseTagGlobal(child);
    }
    else if (child->GetName() == "lexer")
    {
      const wxExLexer& lexer = ParseTagLexer(child);

      if (!lexer.GetScintillaLexer().empty())
      {
        if (lexer.GetScintillaLexer() == "hypertext")
        {
          // As our lexers.xml files cannot use xml comments,
          // add them here.
          wxExLexer l(lexer);
          l.m_CommentBegin = "<!--";
          l.m_CommentEnd = "-->";
          m_Lexers.insert(std::make_pair(lexer.GetScintillaLexer(), l));
        }
        else
        {
          m_Lexers.insert(std::make_pair(lexer.GetScintillaLexer(), lexer));
          m_SortedLexerNames.Add(lexer.GetScintillaLexer());
        }
      }
    }

    child = child->GetNext();
  }

  // Add the <none> lexer.
  m_SortedLexerNames.Add(_("<none>"));

  if (!wxConfigBase::Get()->Exists(_("In files")))
  {
    wxConfigBase::Get()->Write(_("In files"), GetLexerAssociations());
  }

  if (!wxConfigBase::Get()->Exists(_("Add what")))
  {
    wxConfigBase::Get()->Write(_("Add what"), GetLexerAssociations());
  }
}

wxExLexers* wxExLexers::Set(wxExLexers* lexers)
{
  wxExLexers* old = m_Self;
  m_Self = lexers;
  return old;
}

bool wxExLexers::ShowDialog(
  wxWindow* parent,
  wxString& lexer,
  const wxString& caption) const
{
  wxSingleChoiceDialog dlg(
    parent, 
    _("Input") + ":", 
    caption, 
    m_SortedLexerNames);
  
  dlg.SetSelection(lexer.empty() ? 
    m_SortedLexerNames.Index(_("<none>")):
    m_SortedLexerNames.Index(lexer));

  if (dlg.ShowModal() == wxID_CANCEL)
  {
    return false;
  }

  lexer = (dlg.GetSelection() == m_SortedLexerNames.Index(_("<none>")) ?
    wxEmptyString:
    FindByName(dlg.GetStringSelection()).GetScintillaLexer());

  return true;
}
