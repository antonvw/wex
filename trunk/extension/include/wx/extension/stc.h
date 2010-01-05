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
#include <wx/extension/file.h> // for wxExFile
#include <wx/extension/menu.h> // for wxExMenu

class wxExConfigDialog;
class wxExVi;

#if wxUSE_GUI
/// Offers a styled text ctrl with find/replace, folding, printing, popup menu,
/// config support and syntax colouring. Also adds synchronizing to the window,
/// and if the file is a logfile and
/// the caret is at the end, it stays at the end after syncing.
class wxExSTC : public wxStyledTextCtrl, public wxExFile
{
public:
  /// Menu and tooltip flags (0 is used for no menu).
  enum wxExSTCMenuFlags
  {
    STC_MENU_SIMPLE    = 0x0002, ///< for adding copy/paste etc. menu
    STC_MENU_FIND      = 0x0004, ///< for adding find menu
    STC_MENU_REPLACE   = 0x0008, ///< for adding replace menu
    STC_MENU_INSERT    = 0x0010, ///< for adding sequence menu
    STC_MENU_OPEN_LINK = 0x0020, ///< for adding link open menu

    STC_MENU_DEFAULT   = 0xFFFF, ///< all
  };

  /// Open flags (0 is used as default).
  enum wxExSTCOpenFlags
  {
    STC_OPEN_HEX         = 0x0001, ///< open in hex mode
    STC_OPEN_READ_ONLY   = 0x0002, ///< open as readonly, this mode overrides real mode from disk
    STC_OPEN_FROM_OTHER  = 0x0008, ///< opened from within another file (e.g. a link)
  };

  /// Config dialog flags (0 gives
  /// a modal dialog with all options).
  enum wxExSTCConfigFlags
  {
    STC_CONFIG_MODELESS   = 0x0001, ///< use as modeless dialog
    STC_CONFIG_WITH_APPLY = 0x0002, ///< add the apply button
    STC_CONFIG_SIMPLE     = 0x0004, ///< only 'simple' options on dialog
  };

  /// Constructor. Does not open a file, but sets text to specified value,
  /// NULL's are allowed.
  /// This default value is overwritten by Open.
  wxExSTC(wxWindow* parent,
    const wxString& value = wxEmptyString,
    long menu_flags = STC_MENU_DEFAULT,
    wxWindowID id = wxID_ANY,
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    long style = 0,
    const wxString& name = wxSTCNameStr);

  /// Constructor, opens the file.
  /// See also Open.
  wxExSTC(wxWindow* parent,
    const wxExFileName& filename,
    int line_number = 0,
    const wxString& match = wxEmptyString,
    long open_flags = 0,
    long menu_flags = STC_MENU_DEFAULT,
    wxWindowID id = wxID_ANY,
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    long style = 0,
    const wxString& name = wxSTCNameStr);

  /// Destructor.
 ~wxExSTC();

  /// Copy constructor.
  wxExSTC(const wxExSTC& stc);

  /// Adds an ascii table to current document.
  void AddAsciiTable();

  /// Appends text, possibly with timestamp, even if the document is readonly.
  /// If caret was at end, it is repositioned at the end.
  void AppendTextForced(const wxString& text, bool withTimestamp = true);

  /// Colourises the document.
  void Colourise();

  /// Shows a dialog with options, returns dialog return code.
  /// If used modeless, it uses the dialog id as specified,
  /// so you can use that id in wxExFrame::ConfigDialogApplied.
  static int ConfigDialog(
    wxWindow* parent,
    const wxString& title = _("Editor Options"),
    long flags = 0,
    wxWindowID id = wxID_ANY);

  /// Sets the configurable parameters to values currently in config.
  void ConfigGet();

  /// Edit the current selected char as a control char, or if nothing selected,
  /// add a new control char.
  void ControlCharDialog(const wxString& caption = _("Enter Control Character"));

  /// Invokes base and clears document.
  void FileNew(const wxExFileName& filename = wxExFileName());

  /// Shows a menu with current line type checked, and allows you to change it.
  void FileTypeMenu();

  // Finds next with settings from find replace data.
  bool FindNext(bool find_next = true);

  // Finds next.
  bool FindNext(
    const wxString& text, 
    int search_flags = 0,
    bool find_next = true);

  virtual bool GetContentsChanged() const {return GetModify();};

  virtual void ResetContentsChanged();

  /// Gets EOL string.
  const wxString GetEOL() const;

  /// Gets current flags (used by Open).
  long GetFlags() const {return m_Flags;};

  /// Gets line number at current position.
  int GetLineNumberAtCurrentPos() const;

  /// Gets the menu flags.
  long GetMenuFlags() const {return m_MenuFlags;};

  /// Gets search text, as selected or from config.
  const wxString GetSearchText() const;

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

  /// Asks for a lexer for this document, choosing from a dialog of
  /// all available lexers. Then colours the document.
  void LexerDialog(const wxString& caption = _("Enter Lexer"));

  /// Opens the file, reads the content into the window, then closes the file
  /// and sets the lexer.
  /// If you specify a line number, goes to the line if > 0, if -1 goes to end of file.
  /// If you specify a match selects the text on that line.
  virtual bool Open(
    const wxExFileName& filename,
    int line_number = 0,
    const wxString& match = wxEmptyString,
    long flags = 0);

#if wxUSE_PRINTING_ARCHITECTURE
  /// Prints the document.
  void Print(bool prompt = true);

  /// Shows a print preview.
  void PrintPreview();
#endif

  /// Shows properties on the statusbar.
  virtual void PropertiesMessage() const;

  /// Reset all margins.
  /// Default also resets the divider margin.
  void ResetMargins(bool divider_margin = true);

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
  
  /// If set, then the popup menu will show a file save item
  /// if the document is modified.
  /// Default it is off.
  void SetFileSaveInMenu(bool val = true) {
    m_FileSaveInMenu = val;}

  /// Set the (scintilla) lexer for this document using the current filename by
  /// default, or the explicit lexer if specified.
  /// Then colourises the document.
  void SetLexer(const wxString& lexer = wxEmptyString, bool forced = false);

  /// Sets the text.
  void SetText(const wxString& value);

  /// Asks for confirmation to sort the selection.
  void SortSelectionDialog(
    bool sort_ascending,
    const wxString& caption = _("Enter Sort Position"));

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

#if wxUSE_STATUSBAR
  /// Updates the specified statusbar pane with current values.
  void UpdateStatusBar(const wxString& pane) const;
#endif
protected:
  /// Builds the popup menu.
  virtual void BuildPopupMenu(wxExMenu& menu);

  void OnChar(wxKeyEvent& event);
  void OnCommand(wxCommandEvent& event);
  void OnIdle(wxIdleEvent& event);
  void OnKeyDown(wxKeyEvent& event);
  void OnKeyUp(wxKeyEvent& event);
  void OnMouse(wxMouseEvent& event);
  void OnStyledText(wxStyledTextEvent& event);
private:
  void AddTextHexMode(wxFileOffset start, const wxCharBuffer& buffer);
  void AddBasePathToPathList();
  bool CheckAutoComp(const wxUniChar c);
  bool CheckBrace(int pos);
  bool CheckBraceHex(int pos);
  bool CheckSmartIndentation();
  // Clears the component: all text is cleared and all styles are reset.
  // Invoked by Open and FileNew.
  // (Clear is used by scintilla to clear the selection).
  void ClearDocument();
  void EOLModeUpdate(int eol_mode);
  virtual void DoFileLoad(bool synced = false);
  virtual void DoFileSave(bool save_as = false);
  bool FileReadOnlyAttributeChanged(); // sets changed read-only attribute
  int FindReplaceDataFlags() const;
  void FoldAll();
  void GuessType();
  void HexDecCalltip(int pos);
  void Initialize();
  bool LinkOpen(
    const wxString& link,
    wxString& filename, // name of found file
    int line_number = 0, 
    bool link_open = true);
  void ReadFromFile(bool get_only_new_data);
  void SequenceDialog();
  void SetFolding();
  void SetGlobalStyles();
  void SetKeyWords();
  void SetMarkers();
  void SetProperties();
  void SetStyle(const wxString& style);

  // All objects share the following:
  static wxExConfigDialog* m_ConfigDialog;
  static std::vector <wxString> m_Macro;

  bool m_FileSaveInMenu;
  bool m_MacroIsRecording;
  bool m_viMode;
  long m_Flags; // open flags
  long m_GotoLineNumber;
  long m_MenuFlags;
  int m_MarginDividerNumber;
  int m_MarginFoldingNumber;
  int m_MarginLineNumber;
  wxExVi* m_vi;
  wxFileOffset m_PreviousLength;
  wxPathList m_PathList;

  DECLARE_EVENT_TABLE()
};

#endif // wxUSE_GUI
#endif
