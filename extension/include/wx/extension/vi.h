////////////////////////////////////////////////////////////////////////////////
// Name:      vi.h
// Purpose:   Declaration of class wxExVi
// Author:    Anton van Wezenbeek
// Copyright: (c) 2012 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXVI_H
#define _EXVI_H

#include <map>
#include <wx/extension/ex.h>

#if wxUSE_GUI

/// Offers a class that extends wxExSTC with vi behaviour.
class WXDLLIMPEXP_BASE wxExVi : public wxExEx
{
public:
  /// Constructor.
  wxExVi(wxExSTC* stc);
  
  /// Executes command in command mode (like 'j', or 'y').
  /// Returns true if the command was executed.
  virtual bool Command(const wxString& command);
  
  /// Returns whether we are in insert mode.
  bool GetInsertMode() const {return m_InsertMode;};
  
  /// Returns text to be inserted.
  const wxString& GetInsertText() const {return m_InsertText;};
  
  /// Handles char events.
  /// Returns true if event is allowed to be skipped.
  /// This means that the char is not handled by vi,
  /// e.g. vi mode is not active, or we are in insert mode,
  /// so the char should be handled by stc.
  bool OnChar(const wxKeyEvent& event);

  /// Handles keydown events.
  /// See OnChar.
  bool OnKeyDown(const wxKeyEvent& event);
private:
  bool ChangeNumber(bool inc);
  void FindWord(bool find_next = true);
  void GotoBrace();
  bool Indent(
    const wxString& begin_address, 
    const wxString& end_address, 
    bool forward);
  bool InsertMode(const wxString& command);
  void Put(bool after);
  void SetInsertMode(
    const wxString& command,
    int repeat = 1);
  void ToggleCase();

  static wxString m_LastFindCharCommand;

  bool m_Dot;  
  bool m_InsertMode;
  bool m_SearchForward;
  
  int m_InsertRepeatCount;
  
  wxString m_Command;
  wxString m_InsertText;
};
#endif // wxUSE_GUI
#endif
