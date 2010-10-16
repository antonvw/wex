////////////////////////////////////////////////////////////////////////////////
// Name:      vi.h
// Purpose:   Declaration of class wxExVi
// Author:    Anton van Wezenbeek
// Created:   2009-11-21
// RCS-ID:    $Id$
// Copyright: (c) 2009 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXVI_H
#define _EXVI_H

#include <map>
#include <wx/extension/indicator.h>
#include <wx/extension/marker.h>

#if wxUSE_GUI

class wxExConfigDialog;
class wxExSTC;

/// Offers a class that extends STC with vi behaviour.
class WXDLLIMPEXP_BASE wxExVi
{
public:
  /// Constructor.
  wxExVi(wxExSTC* stc);

  /// Returns whether vi is active.
  bool GetIsActive() const {return m_IsActive;};

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
  void DoCommandFind(const wxUniChar& c);
  void DoCommandLine();
  bool DoCommandRange(const wxString& command);
  void FindWord(bool find_next = true) const;
  void GotoBrace() const;
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

  static wxExConfigDialog* m_CommandDialog;
  static wxExConfigDialog* m_FindDialog;
  static wxString m_LastCommand;
  static wxString m_LastFindCharCommand;

  const wxExIndicator m_IndicatorYank;
  const wxExMarker m_MarkerSymbol;
  
  std::map<wxUniChar, int> m_Markers;
  
  bool m_IsActive; // are we actively using vi mode?
  bool m_InsertMode;
  bool m_SearchForward;
  
  int m_InsertRepeatCount;
  int m_SearchFlags;
  
  wxExSTC* m_STC;
  
  wxString m_Command;
  wxString m_InsertText;
};
#endif // wxUSE_GUI
#endif
