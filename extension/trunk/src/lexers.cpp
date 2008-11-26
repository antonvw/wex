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
    std::vector<exLexer>::const_iterator it = m_Lexers.begin();
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
      std::vector<exLexer>::const_iterator it = m_Lexers.begin();
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

// TODO: Styles and Styles hex parse them here insteaad of at stc.
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
    else if (child->GetName() == "style")
    {
      m_Styles.push_back(child->PropVal() + "=" + child->NodeContent());
    }
    else if (child->GetName() == "hex")
    {
      m_StylesHex.push_back(child->PropVal() + "=" + child->NodeContent());
    }
    else
    {
      wxLogError("Undefined property: %s", child->GetName().c_str());
    }
    
    child = child->GetNext();
  }
}

const exLexer exLexers::ParseLexer(const wxXmlNode* node)
{
  exLexer lexer;
  lexer.m_ScintillaLexer = child->GetPropVal("name");

  if (lexer.m_ScintillaLexer == exLexer().Default().m_ScintillaLexer)
  {
    // As the comments for the configuration are itself comments,
    // they are skipped by ReportLine, so do it here.
    lexer = exLexer().Default();
  }
  
  lexer.m_Associations = child->GetPropVal("extensions");

  wxXmlNode *child = node>GetChildren();

  while (child) 
  {
    wxXmlProperty* prop = child->GetProperties();
    
    if (child->GetName() == "colourings")
    {
      lexer.m_Colourings = ParseLocalColourings(node);
    }
    else if (child->GetName() == "keywords")
    {
      lexer.SetKeywords(child->NodeContent());
    }
    else if (child->GetName() == "properties")
    {
      lexer.m_Properties = ParseLocalProperties(node);
    }
    else if (child->GetName() == "comment")
    {
      lexer.m_CommentBegin = child->GetPropVal("begin");
      lexer.m_CommentBegin2 = child->GetPropVal("begin2");
      lexer.m_CommentEnd = child->GetPropVal("end");
      lexer.m_CommentEnd2 = child->GetPropVal("end2");
    }
    else
    {
      wxLogError("Undefined property: %s", child->GetName().c_str());
    }
    
    child = child->GetNext();
  }

  return lexer;
}

void exLexers::ParseLocalColourings(const wxXmlNode* node)
{
  wxString text;

  wxXmlNode *child = node>GetChildren();

  while (child) 
  {
    wxXmlProperty* prop = child->GetProperties();
    
    if (child->GetName() == "colouring")
    {
      text += 
        child->GetPropVal("name") + "=" + child->GetNodeContent());
    }
    else
    {
      wxLogError("Undefined colouring property: %s", child->GetName().c_str());
    }
    
    child = child->GetNext();
  }
  
  return text;
}

void exLexers::ParseLocalProperties(const wxXmlNode* node)
{
  wxString text;
  
  wxXmlNode *child = node>GetChildren();

  while (child) 
  {
    wxXmlProperty* prop = child->GetProperties();
    
    if (child->GetName() == "property")
    {
      text += 
        child->GetPropVal("name") + "=" + child->GetNodeContent());
    }
    else
    {
      wxLogError("Undefined property: %s", child->GetName().c_str());
    }
    
    child = child->GetNext();
  }
  
  return text;
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
