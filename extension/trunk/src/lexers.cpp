/******************************************************************************\
* File:          lexers.cpp
* Purpose:       Implementation of exLexers classes
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 2008 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/stdpaths.h>
#include <wx/tokenzr.h>
#include <wx/stc/stc.h>
#include <wx/extension/extension.h>

using namespace std;

exLexers::exLexers()
  : m_FileName(
#ifdef EX_PORTABLE
      wxPathOnly(wxStandardPaths::Get().GetExecutablePath()),
#else
      wxStandardPaths::Get().GetUserDataDir(),
#endif
      "lexers.xml",
      false)
{
}

const exLexer exLexers::FindByFileName(const wxFileName& filename) const
{
  if (!filename.IsOk() || m_Lexers.empty())
  {
    return exLexer();
  }

  for (
    vector<exLexer>::const_iterator it = m_Lexers.begin();
    it != m_Lexers.end();
    ++it)
  {
    if (exMatchesOneOf(filename, it->GetAssociations()))
    {
      return *it;
    }
  }

  return exLexer();
}

const exLexer exLexers::FindByName(const wxString& name) const
{
  if (!m_Lexers.empty())
  {
    for (
      vector<exLexer>::const_iterator it = m_Lexers.begin();
      it != m_Lexers.end();
      ++it)
    {
      if (name == it->GetScintillaLexer())
      {
        return *it;
      }
    }

    // We did not find a lexer, so give an error.
    // The same error is shown in exSTC::SetLexer as well.
    wxLogError("Lexer is not known: " + name);
  }

  return exLexer();
}

void exLexers::ParseGlobalProperties(const wxXmlNode* node)
{
  wxXmlNode *child = node>GetChildren();

  while (child) 
  {
    wxXmlProperty* prop = child->GetProperties();
    
    if (child->GetName() == "marker")
    {
      const exMarker marker(ParseMarker(
        child->GetPropVal("marker"),
        child->GetNodeContent());

      if (marker.GetMarkerNumber() < wxSTC_STYLE_MAX &&
          marker.GetMarkerSymbol() < wxSTC_STYLE_MAX)
      {
        m_Markers.push_back(marker);
      }
      else
      {
        wxLogError("Illegal marker number: %d or symbol: %d on line: %d",
          marker.GetMarkerNumber(),
          marker.GetMarkerSymbol(),
          GetCurrentLine() + 1);
      }
    }
    else if (child->HasProp("style");
    {
      m_Styles.push_back(value.AfterFirst('.'));
    }
    else if (child->HasProp("hex");
    {
      m_StylesHex.push_back(value.AfterFirst('.'));
    }
    else
    {
      wxLogError("Undefined property: %s on", value.c_str());
    }
    
    child = child->GetNext();
  }
}

const exLexer exLexers::ParseLexer(const wxXmlNode* node)
{
  wxStringTokenizer fields(str, "\t", wxTOKEN_RET_EMPTY);

  if (fields.CountTokens() < 3)
  {
    wxLogError("Syntax error in: %s on line: %d",
      str.c_str(),
      GetCurrentLine() + 1);

    return exLexer();
  }

  exLexer lexer;
  lexer.m_ScintillaLexer = fields.GetNextToken().Strip(wxString::both);

  if (lexer.m_ScintillaLexer == exLexer().Default().m_ScintillaLexer)
  {
    // As the comments for the configuration are itself comments,
    // they are skipped by ReportLine, so do it here.
    lexer = exLexer().Default();
  }

  lexer.m_Associations = fields.GetNextToken().Strip(wxString::both);
  lexer.m_Colourings = fields.GetNextToken().Strip(wxString::both);
  lexer.SetKeywords(fields.GetNextToken().Strip(wxString::both));
  lexer.m_Properties = fields.GetNextToken().Strip(wxString::both);
  lexer.m_CommentEnd = "\0"; // set the default
  lexer.m_CommentEnd2 = "\0"; // set the default

  if (fields.HasMoreTokens())
  {
    lexer.m_CommentBegin = fields.GetNextToken().Strip(wxString::both);

    if (fields.HasMoreTokens())
    {
      lexer.m_CommentEnd = fields.GetNextToken().Strip(wxString::both);

      if (fields.HasMoreTokens())
      {
        lexer.m_CommentBegin2 = fields.GetNextToken().Strip(wxString::both);

        if (fields.HasMoreTokens())
        {
          lexer.m_CommentEnd2 = fields.GetNextToken().Strip(wxString::both);
        }
      }
    }
  }

  return lexer;
}

const exMarker exLexers::ParseMarker(const wxString& number, const wxString& props)
{
  wxStringTokenizer prop_fields(props, ",");

  const wxString symbol = prop_fields.GetNextToken();
  wxColour foreground;
  wxColour background;

  if (prop_fields.HasMoreTokens())
  {
    foreground = prop_fields.GetNextToken();

    if (prop_fields.HasMoreTokens())
    {
      background = prop_fields.GetNextToken();
    }
  }

  const exMarker marker(
    atoi(number.c_str()),
    atoi(symbol.c_str()),
    foreground,
    background);

  return marker;
}

void exLexers::Read()
{
  if (!m_FileName.FileExists()) return;

  // Initialize members.
  m_Lexers.clear();
  m_Markers.clear();
  m_Styles.clear();
  m_StylesHex.clear();

  m_Skip = false;

  wxXmlDocument doc;
  
  if (!doc.Load(m_FileName.GetFullPath())
  {
    return false;
  }
 
  wxXmlNode *child = doc.GetRoot()->GetChildren();

  while (child) 
  {
    if (child->GetName() == "global") 
    {
      ParseGlobalProperties(child);
    }
    else if (child->GetName() == "lexer") 
    {
      const exLexer& lexer = ParseLexer(child);

      if (!lexer.GetScintillaLexer().empty())
      {
        m_Lexers.push_back(lexer);
      }
    } 

    child = child->GetNext();
  }
 
  if (m_Skip)
  {
    wxLogError("Found #if statement without matching #endif");
  }
}
