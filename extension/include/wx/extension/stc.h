////////////////////////////////////////////////////////////////////////////////
// Name:      stc.h
// Purpose:   Declaration of class wxExSTC
// Author:    Anton van Wezenbeek
// Copyright: (c) 2014 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXSTC_H
#define _EXSTC_H

#include <vector> 
#include <wx/stc/stc.h>
#include <wx/extension/hexmode.h>
#include <wx/extension/link.h>
#include <wx/extension/marker.h>
#include <wx/extension/stcfile.h>
#include <wx/extension/vi.h>

#if wxUSE_GUI
class wxFindDialogEvent;
class wxExConfigDialog;
class wxExFile;
class wxExFileName;
class wxExIndicator;
class wxExLexer;
class wxExMenu;

/// Offers a styled text ctrl with:
/// - lexer support (syntax colouring, folding)
/// - vi support (default vi mode is off)
/// - find/replace 
/// - popup menu
/// - printing
class WXDLLIMPEXP_BASE wxExSTC : public wxStyledTextCtrl
{
public:
  /// Menu and tooltip flags.
  enum wxExMenuFlags
  {
    STC_MENU_NONE      = 0x0000, ///< no context menu
    STC_MENU_DEFAULT   = 0x0001, ///< default, standard context menu
    STC_MENU_OPEN_LINK = 0x0002, ///< for adding link open menu
    STC_MENU_VCS       = 0x0004, ///< for adding vcs menu
  };

  /// Window flags.
  enum wxExWindowFlags
  {
    STC_WIN_DEFAULT      = 0x0000, ///< default, not readonly, not hex mode
    STC_WIN_READ_ONLY    = 0x0001, ///< window is readonly, 
                                   ///<   overrides real mode from disk
    STC_WIN_HEX          = 0x0002, ///< window in hex mode
    STC_WIN_NO_INDICATOR = 0x0004, ///< a change indicator is not used
    STC_WIN_FROM_OTHER   = 0x0010  ///< opened from within another file (e.g. a link)
  };

  /// Config dialog flags.
  enum wxExConfigFlags
  {
    STC_CONFIG_DEFAULT    = 0x0000, ///< modal dialog with all options
    STC_CONFIG_MODELESS   = 0x0001, ///< use as modeless dialog
    STC_CONFIG_WITH_APPLY = 0x0002, ///< add the apply button
    STC_CONFIG_SIMPLE     = 0x0004  ///< only 'simple' options on dialog
  };

  /// Constructor.
  wxExSTC(wxWindow *parent, 
    const wxString& value = wxEmptyString,
    long win_flags = STC_WIN_DEFAULT,
    const wxString& title = wxEmptyString,
    long menu_flags = STC_MENU_DEFAULT,
    wxWindowID id = wxID_ANY,
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize, 
    long style = 0);

  /// Constructor, opens the file if it exists.
  /// See also Open.
  wxExSTC(wxWindow* parent,
    const wxExFileName& filename,
    int line_number = 0,
    const wxString& match = wxEmptyString,
    int col_number = 0,
    long win_flags = STC_WIN_DEFAULT,
    long menu_flags = STC_MENU_DEFAULT & STC_MENU_OPEN_LINK,
    wxWindowID id = wxID_ANY,
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    long style = 0);

  /// Copy constructor.
  wxExSTC(const wxExSTC& stc);
  
  /// Is a change indicator allowed.
  bool AllowChangeIndicator() const {return m_AllowChangeIndicator;};

  /// After pressing enter, starts new line at same place
  /// as previous line.
  bool AutoIndentation(int c);
  
  /// Will a cut succeed? 
  virtual bool CanCut() const override;

  /// Will a paste succeed? 
  virtual bool CanPaste() const override;
  
  // Clears the component: all text is cleared and all styles are reset.
  // Invoked by Open and DoFileNew.
  // (Clear is used by scintilla to clear the selection).
  void ClearDocument(bool set_savepoint = true);

  /// Shows a dialog with options, returns dialog return code.
  /// If used modeless, it uses the dialog id as specified,
  /// so you can use that id in wxExFrame::OnCommandConfigDialog.
  static int ConfigDialog(
    wxWindow* parent,
    const wxString& title = _("Editor Options"),
    long flags = 0,
    wxWindowID id = wxID_ANY);

  /// Sets the configurable parameters to values currently in config.
  void ConfigGet(bool init = false);

  /// Copies text to clipboard.
  virtual void Copy() override;

  /// Cuts text to clipboard.
  virtual void Cut() override;

  /// Shows a menu with current line type checked, and allows you to change it.
  void FileTypeMenu();

  /// Finds next with settings from find replace data.
  bool FindNext(bool find_next = true);

  /// Finds next.
  bool FindNext(
    /// text to find
    const wxString& text, 
    /// search flags to be used:
    /// - wxSTC_FIND_WHOLEWORD
    /// - wxSTC_FIND_MATCHCASE
    /// - wxSTC_FIND_WORDSTART
    /// - wxSTC_FIND_REGEXP
    /// - wxSTC_FIND_POSIX
    /// - if -1, use flags from find replace data
    int find_flags = -1,
    /// finds next or previous
    bool find_next = true);
    
  /// Enables or disables folding depending on fold property.
  void Fold(
    /// if document contains more than 'Auto fold' lines,
    /// or if foldall (and fold propertry is on) is specified, 
    /// always all lines are folded.
    bool foldall = false);

  /// Gets EOL string.
  /// If you only want to insert a newline, use NewLine()
  /// (from wxStyledTextCtrl).
  const wxString GetEOL() const;

  /// Gets the file.
  wxExFile& GetFile() {return m_File;};

  /// Gets the filename, as used by the file.
  const wxExFileName& GetFileName() const {return m_File.GetFileName();};

  /// Gets find string, from selected text or from config.
  /// The search flags are taken from frd.
  /// If text is selected, it also sets the find string.
  const wxString GetFindString();

  /// Gets current flags.
  long GetFlags() const {return m_Flags;};
  
  /// Gets hex mode component.
  const wxExHexMode& GetHexMode() const {return m_HexMode;};
  
  /// Gets writable hex mode component.
  wxExHexMode& GetHexMode() {return m_HexMode;};
  
  /// Gets the lexer.
  const wxExLexer& GetLexer() const {return m_Lexer;};

  /// Gets vi component.
  const wxExVi& GetVi() const {return m_vi;};
  
  /// Gets writable vi component.
  /// This allows you to do vi like editing:
  /// - GetVi().Command(":1,$s/xx/yy/g")
  /// - GetVi().Command(":w")
  /// to replace all xx by yy, and save the file.
  wxExVi& GetVi() {return m_vi;};
  
  /// Gets word at position.
  const wxString GetWordAtPos(int pos) const;

  /// Asks for a line number and goes to the line.
  /// In hex mode asks for a byte offset, and goes to that byte.
  bool GotoDialog();

  /// Goes to line and selects the line or the specified text in it.
  void GotoLineAndSelect(
    int line_number, 
    const wxString& text = wxEmptyString,
    int col_number = 0);

  /// Guesses the file type using a small sample size from this document, 
  /// and sets EOL mode and updates statusbar if it found eols.
  void GuessType();
  
   /// Returns true if we are in hex mode.
   bool HexMode() const {return m_HexMode.Active();};
 
  /// Deletes all change markers.
  /// Returns false if marker change is not loaded.
  bool MarkerDeleteAllChange();
  
  /// Opens the file, reads the content into the window, then closes the file
  /// and sets the lexer.
  virtual bool Open(
    /// file to open
    const wxExFileName& filename,
    /// goes to the line if > 0, if -1 goes to end of file
    int line_number = 0,
    /// if not empty selects the text on that line (if line was specified)
    /// or finds text from begin (if line was 0) or end (line was -1)
    const wxString& match = wxEmptyString,
    /// goes to column if col_number > 0
    int col_number = 0,
    /// flags
    long flags = 0);

  /// Paste text from clipboard.
  virtual void Paste() override;

  /// Restores saved position.
  /// Returns true if position was saved before.
  bool PositionRestore();
  
  /// Saves position.
  void PositionSave();

#if wxUSE_PRINTING_ARCHITECTURE
  /// Prints the document.
  void Print(bool prompt = true);

  /// Shows a print preview.
  void PrintPreview();
#endif

  /// Processes specified char.
  /// Default does nothing, but is invoked during ControlCharDialog,
  /// allowing you to add your own processing.
  /// Return true if char was processed.
  virtual bool ProcessChar(int c) {return false;};
  
  /// Shows properties on the statusbar.
  /// Flags used are from wxExStatusFlags.
  virtual void PropertiesMessage(long flags = 0);
  
  /// Reloads current document using specified flags.
  void Reload(long flags = STC_WIN_DEFAULT);

  /// Replaces all text.
  /// It there is a selection, it replaces in the selection, otherwise
  /// in the entire document.
  /// Returns the number of replacements.
  int ReplaceAll(
    const wxString& find_text, 
    const wxString& replace_text);
  
  /// Replaces text and calls find next.
  /// Uses settings from find replace data.
  bool ReplaceNext(bool find_next = true);

  /// Replaces text and calls find next.
  /// It there is a selection, it replaces in the selection, otherwise
  /// it starts at current position.
  bool ReplaceNext(
    /// text to find
    const wxString& find_text, 
    /// text to replace with
    const wxString& replace_text,
    /// search flags to be used:
    /// - wxSTC_FIND_WHOLEWORD
    /// - wxSTC_FIND_MATCHCASE
    /// - wxSTC_FIND_WORDSTART
    /// - wxSTC_FIND_REGEXP
    /// - wxSTC_FIND_POSIX
    /// - if -1, use flags from find replace data
    int find_flags = 0,
    /// argument passed on to FindNext
    bool find_next = true);
  
  /// Resets lexer.
  void ResetLexer();
  
  /// Reset all margins.
  /// Default also resets the divider margin.
  void ResetMargins(bool divider_margin = true);

  /// Deselects selected text in the control.
  // Reimplemented, since scintilla version sets empty sel at 0, and sets caret on pos 0.
  virtual void SelectNone() override;
  
  /// Sets an indicator at specified start and end pos.
  bool SetIndicator(const wxExIndicator& indicator, int start, int end);

  /// Sets the (scintilla) lexer for this document.
  bool SetLexer(const wxExLexer& lexer, bool fold = false);
  
  /// Sets the (scintilla) lexer for this document.
  bool SetLexer(const wxString& lexer, bool fold = false);
  
  /// Sets lexer prop name and value, and applies it.
  void SetLexerProperty(const wxString& name, const wxString& value);

  /// search flags to be used:
  /// - wxSTC_FIND_WHOLEWORD
  /// - wxSTC_FIND_MATCHCASE
  /// - wxSTC_FIND_WORDSTART
  /// - wxSTC_FIND_REGEXP
  /// - wxSTC_FIND_POSIX
  /// - if -1, use flags from find replace data
  void SetSearchFlags(int flags);
  
  /// Sets the text.
  void SetText(const wxString& value);

  /// Shows or hides line numbers.
  void ShowLineNumbers(bool show);
  
  /// Starts or stops syncing.
  /// Default syncing is started during construction.
  void Sync(bool start = true);
  
  /// Use autocomplete lists.
  /// Default on.
  void UseAutoComplete(bool use);

  /// Use and show modification markers in the margin.
  /// If you open a file, the modification markers are used.
  void UseModificationMarkers(bool use);
protected:
  /// Builds the popup menu.
  virtual void BuildPopupMenu(wxExMenu& menu);

  /// Char event handler.  
  void OnChar(wxKeyEvent& event);
  
  /// Command event handler.
  void OnCommand(wxCommandEvent& event);
  
  /// Find dialog event handler.
  void OnFindDialog(wxFindDialogEvent& event);
  
  /// Idle event handler.
  void OnIdle(wxIdleEvent& event);
  
  /// Key down event handler.
  void OnKeyDown(wxKeyEvent& event);
  
  /// Key up event handler.
  void OnKeyUp(wxKeyEvent& event);
  
  // Mouse event handler.
  void OnMouse(wxMouseEvent& event);
  
  /// Styled text event handler.
  void OnStyledText(wxStyledTextEvent& event);
private:
  void CheckAutoComp(const wxUniChar& c);
  void CheckBrace();
  bool CheckBrace(int pos);
  void ControlCharDialog(const wxString& caption = _("Enter Control Character"));
  void EOLModeUpdate(int eol_mode);
  bool FileReadOnlyAttributeChanged(); // sets changed read-only attribute
  void FoldAll();
  void HexDecCalltip(int pos);
  void Initialize(bool file_exists);
  bool LinkOpen(wxString* filename = NULL); // name of found file
  void MarkerNext(bool next);
  void MarkModified(const wxStyledTextEvent& event);
  bool SetHexMode(
    bool on, 
    bool modified = false, 
    const wxCharBuffer& text = wxCharBuffer());
  void SetLexerCommon(bool fold);
  void ShowProperties();
  void SortSelectionDialog(
    bool sort_ascending,
    const wxString& caption = _("Enter Sort Position"));

  const int m_MarginDividerNumber;
  const int m_MarginFoldingNumber;
  const int m_MarginLineNumber;
  const wxExMarker m_MarkerChange;
  const long m_MenuFlags;

  int m_FoldLevel;
  int m_SavedPos;
  int m_SavedSelectionStart;
  int m_SavedSelectionEnd;
  
  long m_Flags; // win flags
  long m_Goto;
  
  bool m_AddingChars;
  bool m_AllowChangeIndicator;
  bool m_UseAutoComplete;

  wxExHexMode m_HexMode;
  // We use a separate lexer here as well
  // (though wxExSTCFile offers one), as you can manually override
  // the lexer.
  wxExLexer m_Lexer;
  wxExLink m_Link;
  wxExSTCFile m_File;
  wxExVi m_vi;

  wxFont m_DefaultFont;
  
  wxString m_AutoComplete;

  // All objects share the following:
  static wxExConfigDialog* m_ConfigDialog;
  static wxExSTCEntryDialog* m_EntryDialog;
  static int m_Zoom;

  DECLARE_EVENT_TABLE()
};
#endif // wxUSE_GUI
#endif
