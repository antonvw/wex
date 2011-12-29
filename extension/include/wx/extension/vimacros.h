////////////////////////////////////////////////////////////////////////////////
// Name:      vimacros.h
// Purpose:   Declaration of class wxExViMacros
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXVIMACROS_H
#define _EXVIMACROS_H

#include <map>
#include <vector>
#include <wx/filename.h>
#include <wx/xml/xml.h>

#if wxUSE_GUI

class wxExVi;

/// Offers the macro collection, and allows
/// recording and playback to vi component.
class WXDLLIMPEXP_BASE wxExViMacros
{
public:  
  /// Default constructor.
  wxExViMacros();
  
  /// Returns all macros (names) as an array of strings.
  const wxArrayString Get() const;
  
  /// Returns contents of macro (for testing).
  const std::vector< wxString > Get(const wxString& macro) const;
  
  /// Returns current or last macro.
  const wxString& GetMacro() {return m_Macro;};
  
  /// Is macro recorded.
  bool IsRecorded(const wxString& macro = wxEmptyString) const;
  
  /// Are we recording?
  bool IsRecording() const {return m_IsRecording;};
  
  /// Plays back macro a number of repeat times on the vi component.
  /// Returns true if all text could be executed.
  bool Playback(wxExVi* vi, const wxString& macro, int repeat = 1);
  
  /// Records (separated) text to current macro.
  /// The text to be recorded should be valid vi command,
  /// though it is not checked here.
  /// If you playback this macro however, the text
  /// is sent to the vi component to execute it.
  void Record(const wxString& text);
  
  /// Records single char to current macro.
  void Record(char c, bool separated = false);
  
  /// Records a separator only to current macro.
  void RecordSeparator();

  /// Starts recording a macro (overwrites if exists).
  void StartRecording(const wxString& macro);
  
  /// Stops recording.
  void StopRecording();
  
  /// Loads all macros from xml document.
  static void LoadDocument();
  
  /// Saves all macros to xml document.
  static void SaveDocument();
private:  
  static const wxFileName GetFileName();
  static bool Load(wxXmlDocument& doc);
    
  static std::map <wxString, std::vector< wxString > > m_Macros;
  bool m_IsRecording;
  wxString m_Macro;
};
#endif // wxUSE_GUI
#endif
