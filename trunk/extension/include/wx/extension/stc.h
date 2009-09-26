/******************************************************************************\
* File:          stc.h
* Purpose:       Declaration of class wxExSTC and related classes
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
#include <wx/fdrepdlg.h> // for wxFindDialogEvent
#include <wx/print.h> 
#include <wx/stc/stc.h>
#include <wx/extension/file.h> // for wxExFile
#include <wx/extension/menu.h> // for wxExMenu
#include <wx/extension/dialog.h> // for wxExDialog

class wxExConfigDialog;
class wxExFindReplaceDialog;

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
    STC_MENU_TOOLTIP   = 0x0001, ///< for adding tooltip
    STC_MENU_SIMPLE    = 0x0002, ///< for adding copy/paste etc. menu
    STC_MENU_FIND      = 0x0004, ///< for adding find menu
    STC_MENU_REPLACE   = 0x0008, ///< for adding replace menu
    STC_MENU_INSERT    = 0x0010, ///< for adding insert timestamp, sequence menu
    STC_MENU_OPEN_LINK = 0x0020, ///< for adding link open menu

    STC_MENU_DEFAULT  =  0xFFFF, ///< all
  };

  /// Open flags (0 is used as default).
  enum wxExSTCOpenFlags
  {
    STC_OPEN_HEX             = 0x0001, ///< open in hex mode
    STC_OPEN_READ_ONLY       = 0x0002, ///< open as readonly, this mode overrides real mode from disk
    STC_OPEN_IS_SYNCED       = 0x0004, ///< added as file is synced from disk
    STC_OPEN_FROM_LINK       = 0x0008, ///< opened from within a link
    STC_OPEN_FROM_STATISTICS = 0x0010, ///< opened from statistics
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

  /// Copy constructor.
  wxExSTC(const wxExSTC& stc);

  /// Opens the file, reads the content into the window, then closes the file
  /// and sets the lexer.
  /// If you specify a line number, goes to the line if > 0, if -1 goes to end of file.
  /// If you specify a match selects the text on that line.
  virtual bool Open(
    const wxExFileName& filename,
    int line_number = 0,
    const wxString& match = wxEmptyString,
    long flags = 0);

  /// Shows properties on the statusbar.
  virtual void PropertiesMessage();

  // Interface, for wxExFile overriden methods.
  virtual bool FileNew(const wxExFileName& filename = wxExFileName());
  virtual bool FileSave();
  virtual bool FileSaveAs();
  virtual bool GetContentsChanged() {return GetModify();};
  virtual void ResetContentsChanged();

  /// Adds an ascii table to current document.
  void AddAsciiTable();

  /// Appends text, possibly with timestamp, even if the document is readonly.
  /// If caret was at end, it is repositioned at the end.
  void AppendTextForced(const wxString& text, bool withTimestamp = true);

  // Called by wxExApp::OnExit, so not for doxygen.
  static void CleanUp();

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

  /// Shows a menu with current line type checked, and allows you to change it.
  void FileTypeMenu();

  // Finds next.
  bool FindNext(const wxString& text, bool find_next = true);

  /// Gets EOL string.
  const wxString GetEOL() const;

  /// Gets current flags (used by Open).
  long GetFlags() const {return m_Flags;};

  /// Gets line number at current position.
  int GetLineNumberAtCurrentPos();

  /// Gets the menu flags.
  long GetMenuFlags() const {return m_MenuFlags;};

  /// Gets search text, as selected or from config.
  const wxString GetSearchText(); // cannot be const, it uses GetSelectedText

  /// Gets text at current position.
  const wxString GetTextAtCurrentPos(); // cannot be const, it uses GetSelectedText

  /// Gets word at position.
  const wxString GetWordAtPos(int pos);

  /// Asks for a line number and goes to the line.
  bool GotoDialog(const wxString& caption = _("Enter Line Number"));

  /// Goes to line and selects the specified text in it.
  void GotoLineAndSelect(int line_number, const wxString& text);

  /// Asks for a lexer for this document, choosing from a dialog of
  /// all available lexers. Then colours the document.
  void LexerDialog(const wxString& caption = _("Enter Lexer"));

  // Sets the path list from config.
  // Handled by framework, so not for doxygen.
  static void PathListInit();

#if wxUSE_PRINTING_ARCHITECTURE
  /// Prints the document.
  void Print(bool prompt = true);

  /// Shows a print preview.
  void PrintPreview();
#endif

  /// Reset all margins.
  /// Default also resets the divider margin.
  void ResetMargins(bool divider_margin = true);

  /// If set, then the popup menu will show a file save item
  /// if the document is modified.
  /// Default it is off.
  void SetFileSaveInMenu(bool val = true) {
    m_FileSaveInMenu = val;}

  /// Set the (scintilla) lexer for this document using the current filename by
  /// default, or the explicit lexer if specified.
  /// Then colourises the document.
  void SetLexer(const wxString& lexer = wxEmptyString, bool forced = false);

  /// TODO: not necessary, just use GetSearchText.
  void SetFindReplaceData() {GetSearchText();};

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
  void UpdateStatusBar(const wxString& pane);
#endif
protected:
  /// Builds the popup menu.
  virtual void BuildPopupMenu(wxExMenu& menu);

  void OnCommand(wxCommandEvent& event);
  void OnFindDialog(wxFindDialogEvent& event);
  void OnIdle(wxIdleEvent& event);
  void OnKey(wxKeyEvent& event);
  void OnMouse(wxMouseEvent& event);
  void OnStyledText(wxStyledTextEvent& event);
private:
  void AddTextHexMode(wxFileOffset start, const wxCharBuffer& buffer);
  void AddBasePathToPathList();
  bool CheckAutoComp(int key);
  bool CheckBrace(int pos);
  bool CheckSmartIndentation();
  // Clears the component: all text is cleared and all styles are reset.
  // Invoked by Open and FileNew.
  // (Clear is used by scintilla to clear the selection).
  void ClearDocument();
  void EOLModeUpdate(int eol_mode);
  virtual void FileSync();
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
  /// Adds a path to the path list, does not change it in the config.
  void PathListAdd(const wxString& path);
  void ReadFromFile(bool get_only_new_data);
  void SequenceDialog();
  void SetFolding();
  void SetGlobalStyles();
  void SetKeyWords();
  void SetMarkers();
  void SetProperties();
  void SetStyle(const wxString& style);

  // static access
  static wxString GetConfigKeyBase() {return "Edit/";};

  // All objects share the following:
  static wxExConfigDialog* m_ConfigDialog;
  static std::vector <wxString> m_Macro;
  static wxPathList m_PathList;
#if wxUSE_PRINTING_ARCHITECTURE
  static wxPrinter* m_Printer;
#endif

  wxExFindReplaceDialog* m_FindReplaceDialog;

  bool m_FileSaveInMenu;
  long m_Flags; // open flags
  long m_GotoLineNumber;
  bool m_MacroIsRecording;
  int m_MarginDividerNumber;
  int m_MarginFoldingNumber;
  int m_MarginLineNumber;
  long m_MenuFlags;
  wxFileOffset m_PreviousLength;

  DECLARE_EVENT_TABLE()
};

/// Offers an wxExSTC as a dialog (like wxTextEntryDialog).
/// The prompt is allowed to be empty, in that case no sizer is used for it.
class wxExSTCEntryDialog : public wxExDialog
{
public:
  /// Constructor.
  wxExSTCEntryDialog(
    wxWindow* parent,
    const wxString& caption,
    const wxString& text,
    const wxString& prompt = wxEmptyString,
    long button_style = wxOK | wxCANCEL,
    wxWindowID id = wxID_ANY,
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER,
    const wxString& name = wxDialogNameStr);

  /// Gets the STC lexer.
  const wxString GetLexer() const {
    return m_STC->GetFileName().GetLexer().GetScintillaLexer();};

  /// Gets the normal text value.
  const wxString GetText() const {return m_STC->GetText();};

  /// Gets raw text value.
  wxCharBuffer GetTextRaw() const {return m_STC->GetTextRaw();};

  /// Sets the STC lexer.
  void SetLexer(const wxString& lexer) {m_STC->SetLexer(lexer);};

  /// Sets the text (either normal or raw).
  void SetText(const wxString& text);
private:
  wxExSTC* m_STC;
};

#endif // wxUSE_GUI
#endif
