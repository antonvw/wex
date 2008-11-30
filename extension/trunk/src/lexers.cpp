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

// TODO: Styles and Styles hex parse them here instead of at stc.
const wxString exLexers::ParseTagColourings(const wxXmlNode* node)
{
  wxString text;

  wxXmlNode* child = node->GetChildren();

  while (child) 
  {
    if (child->GetName() == "colouring")
    {
      text += 
        child->GetAttribute("name", "0") + "=" + child->GetNodeContent();
    }
    else if (child->GetName() == "comment")
    { 
      // Ignore comments.
    }
    else
    {
      wxLogError("Undefined colourings tag: %s on: %d", child->GetName().c_str(), child->GetLineNumber());
    }
    
    child = child->GetNext();
  }
  
  return text;
}

void exLexers::ParseTagGlobal(const wxXmlNode* node)
{
  wxXmlNode* child = node->GetChildren();

  while (child) 
  {
    if (child->GetName() == "marker")
    {
      const exMarker marker(ParseTagMarker(
        child->GetAttribute("no", "0"),
        child->GetNodeContent()));

      if (marker.GetMarkerNumber() < wxSTC_STYLE_MAX &&
          marker.GetMarkerSymbol() < wxSTC_STYLE_MAX)
      {
        m_Markers.push_back(marker);
      }
      else
      {
        wxLogError("Illegal marker number: %d or symbol: %d on: %d",
          marker.GetMarkerNumber(),
          marker.GetMarkerSymbol(),
          child->GetLineNumber());
      }
    }
    else if (child->GetName() == "style")
    {
      m_Styles.push_back(child->GetAttribute("no", "0") + "=" + child->GetNodeContent());
    }
    else if (child->GetName() == "hex")
    {
      m_StylesHex.push_back(child->GetAttribute("no", "0") + "=" + child->GetNodeContent());
    }
    else if (child->GetName() == "comment")
    { 
      // Ignore comments.
    }
    else
    {
      wxLogError("Undefined global tag: %s on: %d", child->GetName().c_str(), child->GetLineNumber());
    }
    
    child = child->GetNext();
  }
}

const exLexer exLexers::ParseTagLexer(const wxXmlNode* node)
{
  exLexer lexer;
  lexer.m_ScintillaLexer = node->GetAttribute("name", "cpp");

  if (lexer.m_ScintillaLexer == exLexer().Default().m_ScintillaLexer)
  {
    // As the comments for the configuration are itself comments,
    // they are skipped by ReportLine, so do it here.
    lexer = exLexer().Default();
  }
  
  lexer.m_Associations = node->GetAttribute("extensions", "*.cpp");

  wxXmlNode *child = node->GetChildren();

  while (child) 
  {
    if (child->GetName() == "colourings")
    {
      lexer.m_Colourings = ParseTagColourings(node->GetChildren());
    }
    else if (child->GetName() == "keywords")
    {
      lexer.SetKeywords(child->GetNodeContent());
    }
    else if (child->GetName() == "properties")
    {
      lexer.m_Properties = ParseTagProperties(node->GetChildren());
    }
    else if (child->GetName() == "comments")
    {
      lexer.m_CommentBegin = child->GetAttribute("begin", "");
      lexer.m_CommentBegin2 = child->GetAttribute("begin2", "");
      lexer.m_CommentEnd = child->GetAttribute("end", "");
      lexer.m_CommentEnd2 = child->GetAttribute("end2", "\n");
    }
    else if (child->GetName() == "comment")
    { 
      // Ignore comments.
    }
    else
    {
      wxLogError("Undefined lexer tag: %s on: %d", child->GetName().c_str(), child->GetLineNumber());
    }
    
    child = child->GetNext();
  }

  return lexer;
}

const exMarker exLexers::ParseTagMarker(const wxString& number, const wxString& props)
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

const wxString exLexers::ParseTagProperties(const wxXmlNode* node)
{
  wxString text;
  
  wxXmlNode *child = node->GetChildren();

  while (child) 
  {
    if (child->GetName() == "property")
    {
      text += 
        child->GetAttribute("name", "0") + "=" + child->GetNodeContent();
    }
    else if (child->GetName() == "comment")
    { 
      // Ignore comments.
    }
    else
    {
      wxLogError("Undefined properties tag: %s on %d", child->GetName().c_str(), child->GetLineNumber());
    }
    
    child = child->GetNext();
  }
  
  return text;
}

bool exLexers::Read()
{
  if (!m_FileName.FileExists()) 
  {
    return false;
  }

  // Initialize members.
  m_Lexers.clear();
  m_Markers.clear();
  m_Styles.clear();
  m_StylesHex.clear();

  wxXmlDocument doc;
  
  if (!doc.Load(m_FileName.GetFullPath()))
  {
    return false;
  }
 
  wxXmlNode* child = doc.GetRoot()->GetChildren();

  while (child) 
  {
    if (child->GetName() == "global") 
    {
      ParseTagGlobal(child);
    }
    else if (child->GetName() == "lexer") 
    {
      const exLexer& lexer = ParseTagLexer(child);

      if (!lexer.GetScintillaLexer().empty())
      {
        m_Lexers.push_back(lexer);
      }
    } 

    child = child->GetNext();
  }

  return true; 
}

