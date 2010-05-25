/******************************************************************************\
* File:          stc.h
* Purpose:       Declaration of class wxExSTC
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _EXSTC_H
#define _EXSTC_H

#include <vector> 
#include <wx/stc/stc.h>
#include <wx/extension/lexer.h>
#include <wx/extension/menu.h> // for wxExMenu
#include <wx/extension/vi.h>

#if wxUSE_GUI

/// Offers a styled text ctrl with find/replace, printing, popup menu, macro support,
/// vi support, lexer support (syntax colouring, folding) and basic extensions.
class wxExSTC : public wxStyledTextCtrl
{
public:
  /// Menu and tooltip flags (0 is used for no menu).
  enum wxExStyledMenuFlags
  {
    STC_MENU_SIMPLE    = 0x0001, ///< for adding copy/paste etc. menu
    STC_MENU_FIND      = 0x0002, ///< for adding find menu
    STC_MENU_REPLACE   = 0x0004, ///< for adding replace menu
    STC_MENU_COMPARE_OR_VCS = 0x1000, ///< for adding compare or VCS menu

    STC_MENU_DEFAULT   = 0xFFFF, ///< all
  };

  /// Window flags (0 is used as default).
  enum wxExSTCWindowFlags
  {
    STC_WIN_READ_ONLY   = 0x0001, ///< window is readonly, this mode overrides real mode from disk
    STC_WIN_HEX         = 0x0002, ///< window in hex mode
    STC_WIN_BLAME       = 0x0004, ///< window in blame mode
  };

  /// Constructor.
  wxExSTC(wxWindow *parent, 
    const wxString& value = wxEmptyString,
    long win_flags = 0,
    long menu_flags = STC_MENU_DEFAULT,
    wxWindowID id = wxID_ANY,
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize, 
    long style = 0);

  /// Copy constructor.
  wxExSTC(const wxExSTC& stc);

  /// Adds an ascii table to current document.
  void AddAsciiTable();

  /// Appends text, possibly with timestamp, even if the document is readonly.
  /// If caret was at end, it is repositioned at the end.
  void AppendTextForced(const wxString& text, bool withTimestamp = true);

  /// Colourises the document.
  void Colourise();

  /// Edit the current selected char as a control char, or if nothing selected,
  /// add a new control char.
  void ControlCharDialog(const wxString& caption = _("Enter Control Character"));

  /// Shows a menu with current line type checked, and allows you to change it.
  void FileTypeMenu();

  // Finds next with settings from find replace data.
  bool FindNext(bool find_next = true);

  // Finds next.
  bool FindNext(
    const wxString& text, 
    int search_flags = 0,
    bool find_next = true);

  /// Gets EOL string.
  const wxString GetEOL() const;

  /// Gets current flags.
  long GetFlags() const {return m_Flags;};

  /// Gets the lexer.
  const wxExLexer& GetLexer() const {return m_Lexer;};

  /// Gets line number at current position.
  int GetLineNumberAtCurrentPos() const;

  /// Gets the menu flags.
  long GetMenuFlags() const {return m_MenuFlags;};

  /// Gets search text, as selected or from config.
  const wxString GetFindText() const;

  /// Gets text at current position.
  const wxString GetTextAtCurrentPos() const;

  /// Gets word at position.
  const wxString GetWordAtPos(int pos) const;

  /// Asks for a line number and goes to the line.
  bool GotoDialog(const wxString& caption = _("Enter Line Number"));

  /// Goes to line and selects the specified text in it.
  void GotoLineAndSelect(int line_number, const wxString& text);

  /// Returns true if specified target is a RE, to be used by
  /// ReplaceTargetRE.
  bool IsTargetRE(const wxString& target) const;

  /// Paste text from clipboard.
  void Paste();

#if wxUSE_PRINTING_ARCHITECTURE
  /// Prints the document.
  void Print(bool prompt = true);

  /// Shows a print preview.
  void PrintPreview();
#endif

  /// Replaces all text.
  /// It there is a selection, it replaces in the selection, otherwise
  /// in the entire document.
  void ReplaceAll(
    const wxString& find_text, 
    const wxString& replace_text);
  
  /// Replaces text and calls find next.
  /// Uses settings from find replace data.
  void ReplaceNext(bool find_next);

  /// Replaces text and calls find next.
  /// It there is a selection, it replaces in the selection, otherwise
  /// it starts at current position.
  void ReplaceNext(
    const wxString& find_text, 
    const wxString& replace_text,
    int search_flags = 0,
    bool find_next = true);
  
  /// Reset all margins.
  /// Default also resets the divider margin.
  void ResetMargins(bool divider_margin = true);

  /// Adds a sequence.
  void SequenceDialog();

  /// Sets the (scintilla) lexer for this document.
  /// Then colourises the document.
  void SetLexer(const wxString& lexer);

  /// Sets the text.
  void SetText(const wxString& value);

  /// Asks for confirmation to sort the selection.
  void SortSelectionDialog(
    bool sort_ascending,
    const wxString& caption = _("Enter Sort Position"));

#if wxUSE_STATUSBAR
  /// Updates the specified statusbar pane with current values.
  void UpdateStatusBar(const wxString& pane);
#endif

  /// Starts recording the macro, and empties the previous one.
  /// There is only one shared macro for all objects.
  void StartRecord();

  /// Stops recording the macro.
  void StopRecord();

  /// A macro has been recorded.
  bool MacroIsRecorded() const {return !m_Macro.empty();};

  /// A marco is now being recorded.
  bool MacroIsRecording() const {return m_MacroIsRecording;};

  /// Plays back the last recorded macro.
  void MacroPlayback();
protected:
  void AddTextHexMode(wxFileOffset start, const wxCharBuffer& buffer);
  void GuessType();
  /// Builds the popup menu.
  virtual void BuildPopupMenu(wxExMenu& menu);
  // Clears the component: all text is cleared and all styles are reset.
  // Invoked by Open and DoFileNew.
  // (Clear is used by scintilla to clear the selection).
  void ClearDocument();
  void OnChar(wxKeyEvent& event);
  void OnCommand(wxCommandEvent& event);
  void OnFocus(wxFocusEvent& event);
  void OnKeyDown(wxKeyEvent& event);
  void OnKeyUp(wxKeyEvent& event);
  void OnMouse(wxMouseEvent& event);
  void OnMouseCapture(wxMouseCaptureLostEvent& event);
  void OnStyledText(wxStyledTextEvent& event);
  void SetFlags(long flags) {m_Flags = flags;};
  void SetGlobalStyles();
  void SetViMode(bool mode) {m_vi.Use(mode);};

  const int m_MarginDividerNumber;
  const int m_MarginFoldingNumber;
  const int m_MarginLineNumber;
  const wxExMarker m_MarkerChange;
private:
  void CheckAutoComp(const wxUniChar& c);
  bool CheckBrace(int pos);
  bool CheckBraceHex(int pos);
  void EOLModeUpdate(int eol_mode);
  void FoldAll();
  void HexDecCalltip(int pos);
  void Initialize();
  /// After pressing enter, starts new line at same place
  /// as previous line.
  bool SmartIndentation();

  bool m_MacroIsRecording;

  long m_Flags; // win flags
  long m_GotoLineNumber;
  const long m_MenuFlags;

  static std::vector <wxString> m_Macro;

  wxExLexer m_Lexer;
  wxExVi m_vi;

  DECLARE_EVENT_TABLE()
};
#endif // wxUSE_GUI
#endif
