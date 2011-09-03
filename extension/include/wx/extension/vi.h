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
  
  /// Executes vi: command that was entered on the vi bar,
  /// or present as modeline command inside a file.
  /// Returns true if the command was handled.
  bool ExecCommand(const wxString& command);

  /// Returns whether we are in insert mode.
  bool GetInsertMode() const {return m_InsertMode;};
  
  /// Returns whether vi is active.
  bool GetIsActive() const {return m_IsActive;};
  
  /// Returns search flags.
  int GetSearchFlags() const {return m_SearchFlags;};
  
  /// Returns STC component.
  wxExSTC* GetSTC() {return m_STC;};
  
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
};
#endif // wxUSE_GUI
#endif
