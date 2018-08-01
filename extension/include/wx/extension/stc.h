////////////////////////////////////////////////////////////////////////////////
// Name:      stc.h
// Purpose:   Declaration of class wxExSTC
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/prntbase.h>
#include <wx/stc/stc.h>
#include <wx/extension/autocomplete.h>
#include <wx/extension/hexmode.h>
#include <wx/extension/link.h>
#include <wx/extension/marker.h>
#include <wx/extension/stcfile.h>
#include <wx/extension/stc-data.h>
#include <wx/extension/vi.h>

class wxExIndicator;
class wxExItemDialog;
class wxExLexer;
class wxExManagedFrame;
class wxExMenu;
class wxExPath;
class wxExVCSEntry;

/// Offers a styled text ctrl with:
/// - lexer support (syntax colouring, folding)
/// - vi support (default vi mode is off)
/// - find/replace 
/// - popup menu
/// - printing
class WXDLLIMPEXP_BASE wxExSTC : public wxStyledTextCtrl
{
public:
  /// Default constructor, sets text if not empty.
  wxExSTC(const std::string& text = std::string(), const wxExSTCData& data = wxExSTCData());

  /// Constructor, opens the file if it exists.
  wxExSTC(const wxExPath& file, const wxExSTCData& data = wxExSTCData());

  /// Virtual override methods.
  
  /// Will a cut succeed? 
  virtual bool CanCut() const override;

  /// Will a paste succeed? 
  virtual bool CanPaste() const override;
  
  /// Clear the selection.
  virtual void Clear() override;

  /// Copies text to clipboard.
  virtual void Copy() override;

  /// Cuts text to clipboard.
  virtual void Cut() override;

  /// Paste text from clipboard.
  virtual void Paste() override;

  /// Deselects selected text in the control.
  // Reimplemented, since scintilla version sets empty sel at 0, and sets caret on pos 0.
  virtual void SelectNone() override;
  
  /// If there is an undo facility and the last operation can be undone, 
  /// undoes the last operation. 
  virtual void Undo() override;

  /// Other methods.

  /// Returns autocomplete.
  auto & AutoComplete() {return m_AutoComplete;};

  /// After pressing enter, starts new line at same place
  /// as previous line.
  bool AutoIndentation(int c);
  
  // Clears the component: all text is cleared and all styles are reset.
  // Invoked by Open and DoFileNew.
  // (Clear is used by scintilla to clear the selection).
  void ClearDocument(bool set_savepoint = true);

  /// Shows a dialog with options, returns dialog return code.
  /// If used modeless, it uses the dialog id as specified,
  /// so you can use that id in wxExFrame::OnCommandItemDialog.
  static int ConfigDialog(const wxExWindowData& data = wxExWindowData());

  /// Sets the configurable parameters to values currently in config.
  void ConfigGet();

  /// Shows a menu with current line type checked, and allows you to change it.
  void FileTypeMenu();

  /// Finds next with settings from find replace data.
  bool FindNext(bool find_next = true);

  /// Finds next.
  bool FindNext(
    /// text to find
    const std::string& text, 
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

  /// Returns associated data.
  const auto& GetData() const {return m_Data;};

  /// Returns EOL string.
  /// If you only want to insert a newline, use NewLine()
  /// (from wxStyledTextCtrl).
  const std::string GetEOL() const;

  /// Returns the file.
  auto & GetFile() {return m_File;};

  /// Returns the filename, as used by the file.
  const auto & GetFileName() const {return m_File.GetFileName();};

  /// Returns find string, from selected text or from config.
  /// The search flags are taken from frd.
  /// If text is selected, it also sets the find string.
  const std::string GetFindString();

  /// Returns hex mode component.
  const auto & GetHexMode() const {return m_HexMode;};
  
  /// Returns writable hex mode component.
  auto & GetHexMode() {return m_HexMode;};
  
  /// Returns the lexer.
  const auto & GetLexer() const {return m_Lexer;};

  /// Returns the lexer.
  auto & GetLexer() {return m_Lexer;};

  /// Returns line on which text margin was clicked,
  /// or -1 if not.
  auto GetMarginTextClick() const {return m_MarginTextClick;};

  /// Returns vi component.
  const auto & GetVi() const {return m_vi;};
  
  /// Returns writable vi component.
  /// This allows you to do vi like editing:
  /// - GetVi().Command(":1,$s/xx/yy/g")
  /// - GetVi().Command(":w")
  /// to replace all xx by yy, and save the file.
  auto & GetVi() {return m_vi;};
  
  /// Returns word at position.
  const std::string GetWordAtPos(int pos) const;

  /// Guesses the file type using a small sample size from this document, 
  /// and sets EOL mode and updates statusbar if it found eols.
  void GuessType();
  
  /// Returns true if we are in hex mode.
  bool HexMode() const {return m_HexMode.Active();};

  /// If selected text is a link, opens the link.
  bool LinkOpen();

  /// Deletes all change markers.
  /// Returns false if marker change is not loaded.
  bool MarkerDeleteAllChange();
  
  /// Opens the file, reads the content into the window, then closes the file
  /// and sets the lexer.
  bool Open(const wxExPath& filename, const wxExSTCData& data = wxExSTCData());

  /// Restores saved position.
  /// Returns true if position was saved before.
  bool PositionRestore();
  
  /// Saves position.
  void PositionSave();

#if wxUSE_PRINTING_ARCHITECTURE
  /// Prints the document.
  void Print(bool prompt = true);

  /// Shows a print preview.
  void PrintPreview(wxPreviewFrameModalityKind kind = wxPreviewFrame_AppModal);
#endif

  /// Processes specified char.
  /// Default does nothing, but is invoked during ControlCharDialog,
  /// allowing you to add your own processing.
  /// Return true if char was processed.
  virtual bool ProcessChar(int c) {return false;};
  
  /// Shows properties on the statusbar.
  /// Flags used are from wxExStatusFlags.
  void PropertiesMessage(long flags = 0);
  
  /// Replaces all text.
  /// It there is a selection, it replaces in the selection, otherwise
  /// in the entire document.
  /// Returns the number of replacements.
  int ReplaceAll(
    const std::string& find_text, 
    const std::string& replace_text);
  
  /// Replaces text and calls find next.
  /// Uses settings from find replace data.
  bool ReplaceNext(bool find_next = true);

  /// Replaces text and calls find next.
  /// It there is a selection, it replaces in the selection, otherwise
  /// it starts at current position.
  bool ReplaceNext(
    /// text to find
    const std::string& find_text, 
    /// text to replace with
    const std::string& replace_text,
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
  
  /// Reset all margins.
  /// Default also resets the divider margin.
  void ResetMargins(wxExSTCMarginFlags flags = STC_MARGIN_ALL);

  /// Sets an indicator at specified start and end pos.
  bool SetIndicator(const wxExIndicator& indicator, int start, int end);

  /// search flags to be used:
  /// - wxSTC_FIND_WHOLEWORD
  /// - wxSTC_FIND_MATCHCASE
  /// - wxSTC_FIND_WORDSTART
  /// - wxSTC_FIND_REGEXP
  /// - wxSTC_FIND_POSIX
  /// - if -1, use flags from find replace data
  void SetSearchFlags(int flags);
  
  /// Sets the text.
  void SetText(const std::string& value);

  /// Returns true if line numbers are shown. 
  bool ShownLineNumbers() const {return
    GetMarginWidth(m_MarginLineNumber) > 0;};

  /// Shows or hides line numbers.
  void ShowLineNumbers(bool show);

  /// Shows vcs info in text margin.
  /// Returns true if info was added.
  bool ShowVCS(const wxExVCSEntry* vcs);
  
  /// Starts or stops syncing.
  /// Default syncing is started during construction.
  void Sync(bool start = true);
  
  /// Use and show modification markers in the margin.
  /// If you open a file, the modification markers are used.
  void UseModificationMarkers(bool use);
  
  // These methods are not yet available in scintilla, create stubs
  // (for the vi MOTION macro).
  void LineHome() {Home();};
  void LineHomeExtend() {HomeExtend();};
  void LineHomeRectExtend() {HomeRectExtend();};
  void LineScrollDownExtend() {;};
  void LineScrollDownRectExtend() {;};
  void LineScrollUpExtend() {;};
  void LineScrollUpRectExtend() {;};
  void ParaUpRectExtend() {;};
  void ParaDownRectExtend() {;};
  void WordLeftRectExtend();
  void WordRightRectExtend();
  void WordRightEndRectExtend() {;};
protected:
  void OnIdle(wxIdleEvent& event);
  void OnStyledText(wxStyledTextEvent& event);
private:
  enum
  {
    LINK_CHECK         = 0x0001,
    LINK_OPEN          = 0x0002,
    LINK_OPEN_MIME     = 0x0004,
  };

  void BindAll();
  void BuildPopupMenu(wxExMenu& menu);
  void CheckBrace();
  bool CheckBrace(int pos);
  bool FileReadOnlyAttributeChanged(); // sets changed read-only attribute
  void FoldAll();
  bool LinkOpen(int mode, std::string* filename = nullptr); // name of found file
  void MarkModified(const wxStyledTextEvent& event);

  const int 
    m_MarginDividerNumber {1}, m_MarginFoldingNumber {2},
    m_MarginLineNumber {0}, m_MarginTextNumber {3};

  const wxExMarker m_MarkerChange = wxExMarker(1);

  int 
    m_FoldLevel {0}, m_MarginTextClick {-1},
    m_SavedPos {-1}, m_SavedSelectionStart {-1}, m_SavedSelectionEnd {-1};
  
  bool m_AddingChars {false};

  wxExManagedFrame* m_Frame;
  wxExAutoComplete m_AutoComplete;
  wxExHexMode m_HexMode;
  wxExSTCFile m_File;
  // We use a separate lexer here as well
  // (though wxExSTCFile offers one), as you can manually override
  // the lexer.
  wxExLexer m_Lexer;
  wxExSTCData m_Data;
  wxExLink m_Link;
  wxExVi m_vi;
  
  wxFont m_DefaultFont;

  // All objects share the following:
  static inline wxExItemDialog* m_ConfigDialog = nullptr;
  static inline wxExSTCEntryDialog* m_EntryDialog = nullptr;
  static inline int m_Zoom = -1;
};
