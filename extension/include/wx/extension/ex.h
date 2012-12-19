////////////////////////////////////////////////////////////////////////////////
// Name:      ex.h
// Purpose:   Declaration of class wxExEx
// Author:    Anton van Wezenbeek
// Copyright: (c) 2012 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXEX_H
#define _EXEX_H

#include <map>
#include <wx/extension/indicator.h>
#include <wx/extension/marker.h>
#include <wx/extension/vimacros.h>

#if wxUSE_GUI

class wxExManagedFrame;
class wxExProcess;
class wxExSTC;

/// Offers a class that adds ex editor to wxExSTC.
class WXDLLIMPEXP_BASE wxExEx
{
public:
  /// Constructor. 
  /// Sets ex mode false.
  wxExEx(wxExSTC* stc);
  
  /// Destructor.
 ~wxExEx();
 
  /// Executes ex: command that was entered on the command line,
  /// or present as modeline command inside a file.
  /// Returns true if the command was executed.
  virtual bool Command(const wxString& command);
  
  /// Returns frame.
  wxExManagedFrame* GetFrame() {return m_Frame;};
  
  /// Returns whether ex is active.
  bool GetIsActive() const {return m_IsActive;};
  
  /// Returns last entered command.
  const wxString& GetLastCommand() const {return m_LastCommand;};
  
  /// Returns macro being or last played back.
  const wxString& GetMacro() const {return m_Macros.GetMacro();};

  /// Gets the macros.
  wxExViMacros& GetMacros() {return m_Macros;};

  /// Returns search flags.
  int GetSearchFlags() const {return m_SearchFlags;};
  
  /// Returns STC component.
  wxExSTC* GetSTC() {return m_STC;};
  
  /// Expands a variable.
  bool MacroExpand(const wxString& variable) {
    return m_Macros.Expand(this, variable);};
    
  /// A macro is being played back.
  bool MacroIsPlayback() const {
    return m_Macros.IsPlayback();};
    
  /// A macro has been recorded.
  /// If you do not specify a macro, then
  /// returns true if any macro has been recorded,
  /// otherwise true if specified macro has been recorded.
  bool MacroIsRecorded(const wxString& macro = wxEmptyString) const {
    return m_Macros.IsRecorded(macro);};

  /// A macro is now being recorded.
  bool MacroIsRecording() const {return m_Macros.IsRecording();};

  /// Plays back a recorded macro.
  /// If specified macro is empty,
  /// it asks for the name of the macro.
  /// Returns true if the macro was played back succesfully.
  bool MacroPlayback(
    const wxString& macro = wxEmptyString,
    int repeat = 1);
  
  /// Records text within a macro.
  void MacroRecord(const wxString& text, bool new_command = true) {
    m_Macros.Record(text, new_command);};
  
  /// Start recording a macro.  
  /// If specified macro is empty,
  /// it asks for the name of the macro.
  void MacroStartRecording(const wxString& macro = wxEmptyString);
  
  /// Stops recording current macro.
  void MacroStopRecording() {m_Macros.StopRecording();};

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
  
  /// Set using ex mode.
  void Use(bool mode) {m_IsActive = mode;};
protected:
  /// Deletes number of lines, starting at current line.
  bool Delete(int lines) const;
  
  /// Deletes lines from begin to end address.
  /// Returns false if address cannot be related to a line number.
  bool Delete(
    const wxString& begin_address, 
    const wxString& end_address);
    
  /// Sets last command.
  void SetLastCommand(
    const wxString& command,
    bool always = false);
    
  /// Sets selection from begin to end address.
  /// Returns false if address cannot be related to a line number.
  bool SetSelection(
    const wxString& begin_address, 
    const wxString& end_address) const;
    
  /// Converts address to a real line number, filtering out markers
  /// and special characters.
  /// Returns 0 if address is not valid.
  int ToLineNumber(const wxString& address) const;
  
  /// Yanks number of lines, starting at current line.
  void Yank(int lines);
private:
  bool CommandGlobal(const wxString& search);
  bool CommandRange(const wxString& command);
  bool CommandSet(const wxString& command);
  bool Indent(
    const wxString& begin_address, 
    const wxString& end_address, 
    bool forward);
  bool Move(
    const wxString& begin_address, 
    const wxString& end_address, 
    const wxString& destination);
  bool Substitute(
    const wxString& begin_address, 
    const wxString& end_address, 
    const wxString& pattern,
    const wxString& replacement);
  bool Write(
    const wxString& begin_address, 
    const wxString& end_address,
    const wxString& filename) const;
  bool Yank(
    const wxString& begin_address, 
    const wxString& end_address);
    
  const wxExIndicator m_FindIndicator;
  const wxExMarker m_MarkerSymbol;

  std::map<wxUniChar, int> m_Markers;
  
  static wxString m_LastCommand;
  static wxExViMacros m_Macros;

  bool m_IsActive; // are we actively using ex mode?
  
  int m_SearchFlags;
  
  wxString m_Replacement;
  
  wxExManagedFrame* m_Frame;  
  wxExProcess* m_Process;
  wxExSTC* m_STC;
};
#endif // wxUSE_GUI
#endif
