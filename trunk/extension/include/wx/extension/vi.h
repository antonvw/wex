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
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/stc/stc.h> // for wxStyledTextEvent
#include <wx/extension/indicator.h>
#include <wx/extension/marker.h>

#if wxUSE_GUI

class wxExConfigDialog;
class wxExStyledTextCtrl;

/// Offers a class that extends STC with vi behaviour.
class wxExVi
{
public:
  /// Constructor.
  wxExVi(wxExStyledTextCtrl* stc);

  /// Gets the search text.
  const wxString& GetSearchText() const {return m_SearchText;};

  /// Handles char events.
  /// Returns true if event is allowed to be skipped.
  bool OnChar(const wxKeyEvent& event);

  /// Handles char added events.
  void OnCharAdded(const wxStyledTextEvent& event);

  /// Handles keydown events.
  /// Returns true if event is allowed to be skipped.
  bool OnKeyDown(const wxKeyEvent& event);
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
  void FindWord(bool find_next = true);
  void GotoBrace();
  void InsertMode(
    const wxUniChar c = 'i', 
    int repeat = 1,
    bool overtype = false,
    bool dot = false);
  bool Move(
    const wxString& begin_address, 
    const wxString& end_address, 
    const wxString& destination);
  void Repeat();
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
  void Yank(int lines) const;
  bool Yank(
    const wxString& begin_address, 
    const wxString& end_address) const;

  static wxExConfigDialog* m_CommandDialog;
  static wxExConfigDialog* m_FindDialog;
  static wxString m_LastCommand;

  const wxExIndicator m_IndicatorInsert;
  const wxExIndicator m_IndicatorPut;
  const wxExIndicator m_IndicatorYank;
  const wxExMarker m_MarkerSymbol;
  const wxString m_FindDialogItem;
  
  std::map<wxUniChar, int> m_Markers;
  
  bool m_InsertMode;
  bool m_SearchForward;
  
  int m_InsertRepeatCount;
  int m_SearchFlags;
  
  wxExStyledTextCtrl* m_STC;
  
  wxString m_Command;
  wxString m_InsertText;
  wxString m_SearchText;
};

#endif // wxUSE_GUI
#endif
