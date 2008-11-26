/******************************************************************************\
* File:          lexers.h
* Purpose:       Declaration of exLexers class
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 2008, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _EXLEXERS_H
#define _EXLEXERS_H

#include <wx/xml/xml.h>

/// Contains the lexers.
/// Reads the lexers, keywords, markers and styles
/// from configuration file lexers.xml and makes
/// them available (without the comment lines).
/// See for documentation the lexers.xml file.
class exLexers
{
public:
  /// Constructor.
  exLexers();

  /// Finds a lexer specified by a filename.
  const exLexer FindByFileName(const wxFileName& filename) const;

  /// Finds a lexer specified by the (scintilla) name.
  const exLexer FindByName(const wxString& name) const;

  /// Gets the lexers.
  const std::vector<exLexer>& Get() const {return m_Lexers;};

  /// Gets the markers.
  const std::vector<exMarker>& GetMarkers() const {return m_Markers;};

  /// Gets the styles.
  const std::vector<wxString>& GetStyles() const {return m_Styles;};

  /// Gets the styles hex.
  const std::vector<wxString>& GetStylesHex() const {return m_StylesHex;};

  /// Reads the lexers, keywords, markers and styles from xml configuration file.
  void Read();
private:
  void ParseGlobalProperties(const wxXmlNode* node);
  const exLexer ParseLexer(const wxXmlNode* node);
  const exMarker ParseMarker(const wxString& number, const wxString& props);

  std::vector<exLexer> m_Lexers;
  std::vector<exMarker> m_Markers;
  std::vector<wxString> m_Styles;
  std::vector<wxString> m_StylesHex;

  bool m_Skip;

  exFileName m_FileName;
};
#endif
