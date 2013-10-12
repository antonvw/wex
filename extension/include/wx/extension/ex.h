////////////////////////////////////////////////////////////////////////////////
// Name:      ex.h
// Purpose:   Declaration of class wxExEx
// Author:    Anton van Wezenbeek
// Copyright: (c) 2013 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXEX_H
#define _EXEX_H

#include <map>
#include <wx/extension/indicator.h>
#include <wx/extension/marker.h>

#if wxUSE_GUI

class wxExAddressRange;
class wxExManagedFrame;
class wxExProcess;
class wxExSTC;
class wxExViMacros;

/// Offers a class that adds ex editor to wxExSTC.
class WXDLLIMPEXP_BASE wxExEx
{
public:
  /// Constructor. 
  /// Sets ex mode.
  wxExEx(wxExSTC* stc);
  
  /// Destructor.
 ~wxExEx();
  
  /// Adds text (to STC or register, if register is active).
  void AddText(const wxString& text);
 
  /// Executes ex: command that was entered on the command line,
  /// or present as modeline command inside a file.
  /// The command should start with a ':'.
  /// Returns true if the command was executed.
  virtual bool Command(const wxString& command);
  
  /// Returns frame.
  wxExManagedFrame* GetFrame() {return m_Frame;};
  
  /// Returns whether ex is active.
  bool GetIsActive() const {return m_IsActive;};
  
  /// Returns last entered command.
  const wxString& GetLastCommand() const {return m_LastCommand;};
  
  /// Gets the macros.
  wxExViMacros& GetMacros() {return m_Macros;};

  /// Returns register name.
  const wxString& GetRegister() const {return m_Register;};
  
  /// Returns search flags.
  int GetSearchFlags() const {return m_SearchFlags;};
  
  /// Returns STC component.
  wxExSTC* GetSTC() {return m_STC;};
  
  /// Plays back a recorded macro or expands a variable.
  /// If specified macro is empty,
  /// it shows a list with all macros and variables,
  /// allowing you to choose one.
  /// Returns true if the macro was played back 
  /// or the variable was expanded succesfully.
  bool MacroPlayback(
    const wxString& macro = wxEmptyString,
    int repeat = 1);
      
  /// Adds recording to current macro.
  virtual void MacroRecord(const wxString& text);
  
  /// Start recording a macro.  
  /// If specified macro is empty,
  /// it asks for the name of the macro.
  /// You can stop recording by invoking GetMacros.StopRecording().
  void MacroStartRecording(const wxString& macro = wxEmptyString);
  
  /// Adds marker at the specified line.
  /// Returns true if marker could be added.
  bool MarkerAdd(
    /// marker
    const wxUniChar& marker,
    /// line to add marker, default current line
    int line = -1);
  
  /// Deletes specified marker.
  /// Returns true if marker was deleted.
  bool MarkerDelete(const wxUniChar& marker);
  
  /// Goes to specified marker.
  /// Returns true if marker exists.
  bool MarkerGoto(const wxUniChar& marker);
  
  /// Returns line for specified marker.
  /// Returns -1 if marker does not exist.
  int MarkerLine(const wxUniChar& marker) const;
  
  /// Sets delete registers 1 - 9 (if value not empty).
  void SetRegistersDelete(const wxString& value);
  
  /// Set using ex mode.
  void Use(bool mode) {m_IsActive = mode;};
protected:
  /// Sets last command.
  void SetLastCommand(
    const wxString& command,
    bool always = false);
  
  /// Sets register name.
  void SetRegister(const wxString& name) {m_Register = name;};
private:
  bool CommandGlobal(const wxString& search);
  bool CommandRange(const wxString& command);
  bool CommandSet(const wxString& command);
  bool Substitute(
    const wxExAddressRange& range, 
    const wxString& pattern,
    const wxString& replacement,
    const wxString& options);
    
  const wxExIndicator m_FindIndicator;
  const wxExMarker m_MarkerSymbol;

  std::map<wxUniChar, int> m_Markers;
  
  static wxString m_LastCommand;
  static wxExViMacros m_Macros;

  bool m_IsActive; // are we actively using ex mode?
  
  int m_SearchFlags;
  
  wxString m_Replacement;
  wxString m_Register;
  
  wxExManagedFrame* m_Frame;  
  wxExProcess* m_Process;
  wxExSTC* m_STC;
};
#endif // wxUSE_GUI
#endif
