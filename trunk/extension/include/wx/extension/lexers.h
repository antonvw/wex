/******************************************************************************\
* File:          lexers.h
* Purpose:       Declaration of exLexers class
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 2009, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _EXLEXERS_H
#define _EXLEXERS_H

#include <map>
#include <vector>
#include <wx/filename.h>
#include <wx/xml/xml.h>
#include <wx/extension/lexer.h>

/// This class defines our markers, closely related to scintilla markers.
class exMarker
{
public:
  /// Constructor.
  exMarker(
    int markerNumber,
    int markerSymbol,
    const wxColour& foreground = wxNullColour,
    const wxColour& background = wxNullColour)
    : m_MarkerNumber(markerNumber)
    , m_MarkerSymbol(markerSymbol)
    , m_BackgroundColour(background)
    , m_ForegroundColour(foreground) {}

  /// Gets the background colour.
  const wxColour& GetBackgroundColour() const {return m_BackgroundColour;};

  /// Gets the foreground colour.
  const wxColour& GetForegroundColour() const {return m_ForegroundColour;};

  /// Gets the marker number.
  unsigned int GetMarkerNumber() const {return m_MarkerNumber;};

  /// Gets the marker symbol.
  unsigned int GetMarkerSymbol() const {return m_MarkerSymbol;};
private:
  unsigned int m_MarkerNumber;
  unsigned int m_MarkerSymbol;
  wxColour m_BackgroundColour;
  wxColour m_ForegroundColour;
};

/// Reads the lexers, keywords, markers and styles
/// from the configuration file and makes
/// them available.
class exLexers
{
public:
  /// Constructor for lexers from specified filename.
  exLexers(const wxFileName& filename);

  /// Builds a combobox string from available lexers.
  const wxString BuildComboBox() const;

  /// Builds a wildcard string from available lexers using specified filename.
  const wxString BuildWildCards(const wxFileName& filename) const;

  /// Returns the number of lexers.
  const size_t Count() const {return m_Lexers.size();};
  
  /// Finds a lexer specified by a filename.
  const exLexer FindByFileName(const wxFileName& filename) const;

  /// Finds a lexer specified by the (scintilla) name.
  const exLexer FindByName(const wxString& name) const;

  /// Finds a lexer if text starts with some special tokens.
  const exLexer FindByText(const wxString& text) const;

  /// Gets the filename.
  const wxFileName& GetFileName() const {return m_FileName;};
  
  /// Gets the indicators.
  const std::map<int, int>& GetIndicators() const {return m_Indicators;};

  /// Gets the markers.
  const std::vector<exMarker>& GetMarkers() const {return m_Markers;};

  /// Gets the styles.
  const std::vector<wxString>& GetStyles() const {return m_Styles;};

  /// Gets the styles hex.
  const std::vector<wxString>& GetStylesHex() const {return m_StylesHex;};

  /// Reads the lexers, keywords, markers and styles from file.
  /// Returns true if file exists and is valid xml document.
  bool Read();

  /// Shows a dialog with all lexers, allowing you to choose one.
  /// Returns true if you selected one.
  bool ShowDialog(
    wxWindow* parent,
    exLexer& lexer,
    const wxString& caption = _("Enter Lexer")) const;
private:
  const wxString ParseTagColourings(const wxXmlNode* node) const;
  void ParseTagGlobal(const wxXmlNode* node);
  const exLexer ParseTagLexer(const wxXmlNode* node) const;
  const exMarker ParseTagMarker(const wxString& number, const wxString& props) const;
  const wxString ParseTagProperties(const wxXmlNode* node) const;

  std::map<int, int> m_Indicators;
  std::vector<exLexer> m_Lexers;
  std::vector<exMarker> m_Markers;
  std::vector<wxString> m_Styles;
  std::vector<wxString> m_StylesHex;

  const wxFileName m_FileName;
};
#endif
