////////////////////////////////////////////////////////////////////////////////
// Name:      vi.h
// Purpose:   Declaration of class wxExVi
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXVI_H
#define _EXVI_H

#include <map>
#include <wx/extension/indicator.h>
#include <wx/extension/marker.h>
#include <wx/extension/vimacros.h>

#if wxUSE_GUI

class wxExManagedFrame;
class wxExProcess;
class wxExSTC;

/// Offers a class that extends wxExSTC with vi behaviour.
class WXDLLIMPEXP_BASE wxExVi
{
public:
  /// Constructor.
  wxExVi(wxExSTC* stc);
  
  /// Destructor.
 ~wxExVi();
 
  /// Executes command in command mode.
  /// Returns true if the command was executed.
  bool Command(const wxString& command) {return DoCommand(command, false);};
  
  /// Executes vi: command that was entered on the vi bar,
  /// or present as modeline command inside a file.
  /// Returns true if the command was executed.
  bool ExecCommand(const wxString& command);

  /// Finds next.
  /// Returns true if text was found.
  bool FindNext(
    /// text to find
    const wxString& text, 
    /// finds next or previous
    bool find_next = true);

  /// Returns whether we are in insert mode.
  bool GetInsertMode() const {return m_InsertMode;};
  
  /// Returns whether vi is active.
  bool GetIsActive() const {return m_IsActive;};
  
  /// Returns search flags.
  int GetSearchFlags() const {return m_SearchFlags;};
  
  /// Returns STC component.
  wxExSTC* GetSTC() {return m_STC;};
  
  /// A macro has been recorded.
  /// If you do not specify a macro, then
  /// returns true if any macro has been recorded,
  /// otherwise true if specified macro has been recorded.
  bool MacroIsRecorded(const wxString& macro = wxEmptyString) const {
    return m_IsActive && m_Macros.IsRecorded(macro);};

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
  void MacroRecord(const wxString& text) {
    if (m_Macros.IsRecording()) m_Macros.Record(text);};
  
  /// Start recording a macro.  
  /// If specified macro is empty,
  /// it asks for the name of the macro.
  void MacroStartRecording(const wxString& macro = wxEmptyString);
  
  /// Stops recording current macro.
  void MacroStopRecording() {m_Macros.StopRecording();};
  
  /// Handles char events.
  /// Returns true if event is allowed to be skipped.
  /// This means that the char is not handled by vi,
  /// e.g. vi mode is not active, or we are in insert mode,
  /// so the char should be handled by stc.
  bool OnChar(const wxKeyEvent& event);

  /// Handles keydown events.
  /// See OnChar.
  bool OnKeyDown(const wxKeyEvent& event);

  /// Set using vi mode.
  void Use(bool mode) {m_IsActive = mode;};
private:
  bool ChangeNumber(bool inc);
  void Delete(int lines) const;
  bool Delete(
    const wxString& begin_address, 
    const wxString& end_address);
  void DeleteMarker(const wxUniChar& marker);
  bool DoCommand(const wxString& command, bool dot);
  bool DoCommandRange(const wxString& command);
  bool DoCommandSet(const wxString& command);
  void FindWord(bool find_next = true) const;
  void GotoBrace() const;
  bool Indent(
    const wxString& begin_address, 
    const wxString& end_address, 
    bool forward);
  void InsertMode(
    const wxUniChar c = 'i', 
    int repeat = 1,
    bool overtype = false,
    bool dot = false);
  bool Move(
    const wxString& begin_address, 
    const wxString& end_address, 
    const wxString& destination);
  void Put(bool after) const;
  void SetIndicator(const wxExIndicator& indicator, int start, int end) const;
  bool SetSelection(
    const wxString& begin_address, 
    const wxString& end_address) const;
  bool Substitute(
    const wxString& begin_address, 
    const wxString& end_address, 
    const wxString& pattern,
    const wxString& replacement) const;
  void ToggleCase() const;
  int ToLineNumber(const wxString& address) const;
  bool Write(
    const wxString& begin_address, 
    const wxString& end_address,
    const wxString& filename) const;
  void Yank(int lines) const;
  bool Yank(
    const wxString& begin_address, 
    const wxString& end_address) const;

  static wxString m_LastCommand;
  static wxString m_LastFindCharCommand;
  static wxExViMacros m_Macros;

  const wxExMarker m_MarkerSymbol;
  
  std::map<wxUniChar, int> m_Markers;
  
  bool m_InsertMode;
  bool m_IsActive; // are we actively using vi mode?
  bool m_SearchForward;
  
  int m_InsertRepeatCount;
  int m_SearchFlags;
  
  wxExManagedFrame* m_Frame;  
  wxExProcess* m_Process;
  wxExSTC* m_STC;
  
  wxString m_Command;
  wxString m_InsertText;
  wxString m_Macro; // macro played back
};
#endif // wxUSE_GUI
#endif
