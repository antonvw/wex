////////////////////////////////////////////////////////////////////////////////
// Name:      ex.h
// Purpose:   Declaration of class wxExEx
// Author:    Anton van Wezenbeek
// Copyright: (c) 2014 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXEX_H
#define _EXEX_H

#include <map>
#include <wx/extension/indicator.h>
#include <wx/extension/marker.h>

#if wxUSE_GUI

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
  virtual ~wxExEx();
  
  /// Adds text (to STC or register, if register is active).
  void AddText(const std::string& text);
 
  /// Appends to insert register.
  void AddTextRegisterInsert(const std::string& value) const;
  
  /// Executes ex: command that was entered on the command line,
  /// or present as modeline command inside a file.
  /// The command should start with a ':'.
  /// Returns true if the command was executed.
  virtual bool Command(const std::string& command);
  
  /// Cuts selected text to yank register,
  /// and updates delete registers.
  void Cut(bool show_message = true);
  
  /// Returns frame.
  wxExManagedFrame* GetFrame() {return m_Frame;};
  
  /// Returns whether ex is active.
  bool GetIsActive() const {return m_IsActive;};
  
  /// Returns last entered command.
  const std::string& GetLastCommand() const {return m_LastCommand;};
  
  /// Gets the macros.
  wxExViMacros& GetMacros() {return m_Macros;};

  /// Returns register name.
  const char GetRegister() const {return m_Register;};
  
  /// Returns text to be inserted.
  const std::string GetRegisterInsert() const;
  
  /// Returns text from current register (or yank register if no register active).
  const std::string GetRegisterText() const;
  
  /// Returns search flags.
  int GetSearchFlags() const {return m_SearchFlags;};
  
  /// Returns selected text as a string.
  const std::string GetSelectedText() const;
  
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
  virtual void MacroRecord(const std::string& text);
  
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
  void SetRegistersDelete(const std::string& value) const;
  
  /// Deletes last char from insert register.
  void SetRegisterInsertDeleteBack() const;
  
  /// Clears insert register.
  void SetRegisterInsertEmpty() const;
  
  /// Sets yank register (if value not empty).
  void SetRegisterYank(const std::string& value) const;
  
  /// Set using ex mode.
  void Use(bool mode) {m_IsActive = mode;};
  
  /// Yanks selected text to yank register.
  void Yank(bool show_message = true);
protected:
  /// Sets last command.
  void SetLastCommand(
    const std::string& command,
    bool always = false);
  
  /// Sets register name.
  /// Setting register 0 results in
  /// disabling current register.
  void SetRegister(const char name) {m_Register = name;};
private:
  bool CommandGlobal(const wxString& search);
  bool CommandRange(const wxString& command);
  bool CommandSet(const wxString& command);
    
  const wxExIndicator m_FindIndicator;
  const wxExMarker m_MarkerSymbol;

  std::map<wxUniChar, int> m_Markers;
  
  static std::string m_LastCommand;
  static wxExViMacros m_Macros;

  bool m_IsActive; // are we actively using ex mode?
  
  int m_SearchFlags;
  
  char m_Register;
  
  wxExManagedFrame* m_Frame;  
  wxExProcess* m_Process;
  wxExSTC* m_STC;
};
#endif // wxUSE_GUI
#endif
