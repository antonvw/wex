/******************************************************************************\
* File:          base.h
* Purpose:       Declaration of wxWidgets base extension classes
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _EXBASE_H
#define _EXBASE_H

#include <map>
#include <vector>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/artprov.h> // for wxArtID
#include <wx/aui/auibook.h> // for wxAuiManager
#include <wx/datetime.h>
#include <wx/fdrepdlg.h> // for wxFindReplaceDialog
#include <wx/stockitem.h> // for wxGetStockLabel and MNEMONIC
#include <wx/extension/defs.h> // for ID_EDIT_STATUS_BAR
#include <wx/extension/file.h> // for wxExFileName

// Only if we have a gui.
#if wxUSE_GUI
class wxExListView;
class wxExStatusBar;
class wxExSTC;
class wxExToolBar;

/// Offers a general dialog, with a separated button sizer at the bottom.
/// Derived dialogs can use the user sizer for laying out their controls.
class wxExDialog : public wxDialog
{
public:
  /// Constructor.
  /// Flags is a bit list of the following flags:
  /// wxOK, wxCANCEL, wxYES, wxNO, wxHELP, wxNO_DEFAULT.
  wxExDialog(wxWindow* parent,
    const wxString& title,
    long flags = wxOK | wxCANCEL,
    wxWindowID id = wxID_ANY,
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER,
    const wxString& name = wxDialogNameStr);
protected:
  /// Adds to the user sizer using the sizer flags.
  wxSizerItem* AddUserSizer(
    wxWindow* window,
    const wxSizerFlags& flags = wxSizerFlags().Expand().Center());

  /// Adds to the user sizer using the sizer flags.
  wxSizerItem* AddUserSizer(
    wxSizer* sizer,
    const wxSizerFlags& flags = wxSizerFlags().Expand().Center());

  /// BUild the sizers. Should be invoked after adding to sizers.
  void BuildSizers();

  /// Gets the flags (as specified in constructor).
  long GetFlags() const {return m_Flags;};
private:
  const long m_Flags;
  wxFlexGridSizer* m_TopSizer;
  wxFlexGridSizer* m_UserSizer;
};

/// Offers a general find and printer interface.
class wxExInterface
{
public:
  /// Default constructor.
  wxExInterface()
    : m_FindReplaceDialog(NULL) {;};

  /// Destructor.
  virtual ~wxExInterface() {
    if (m_FindReplaceDialog != NULL) 
    {
      m_FindReplaceDialog->Destroy();
      m_FindReplaceDialog = NULL;
    };};

  /// Build the page, for the htmleasyprinting.
  virtual const wxString BuildPage() {return wxEmptyString;};

  /// Shows a find dialog.
  virtual void FindDialog(
    wxWindow* parent, 
    const wxString& caption = _("Find"));

  /// Finds next (or previous) occurrence.
  /// Default returns false.
  virtual bool FindNext(
    const wxString& WXUNUSED(text), 
    bool WXUNUSED(find_next) = true) {
    return false;};

  /// Shows searching for in the statusbar, and calls FindNext.
  bool FindResult(const wxString& text, bool find_next, bool& recursive);

  /// Invokes wxExApp PrintText with BuildPage.
  void Print();

  /// Adds a caption.
  virtual const wxString PrintCaption() const {return _("Printout");};

  /// You can use macros in PrintFooter and in PrintHeader:
  ///   \@PAGENUM\@ is replaced by page number
  ///   \@PAGESCNT\@ is replaced by total number of pages
  virtual const wxString PrintFooter() const
    {return _("Page @PAGENUM@ of @PAGESCNT@");};

  /// Adds a header.
  virtual const wxString PrintHeader() const
    {return _("Printed") + ": " + wxDateTime::Now().Format();};

  /// Invokes wxExApp PreviewText with BuildPage.
  void PrintPreview();

  /// Shows a replace dialog.
  virtual void ReplaceDialog(
    wxWindow* parent, 
    const wxString& caption = _("Replace"));
protected:
  void OnFindDialog(wxFindDialogEvent& event);
private:
  wxFindReplaceDialog* m_FindReplaceDialog;
};

/// Offers a collection of art, mapping stock id's to art id's.
class wxExStockArt
{
public:
  /// Constructor, fills the map first time it is invoked.
  wxExStockArt();
protected:
  /// If id is a stock id, fills stock_label and bitmap.
  void CheckStock(
    int id,
    wxString& stock_label,
    wxBitmap& bitmap,
    long flags = wxSTOCK_WITH_MNEMONIC | wxSTOCK_WITH_ACCELERATOR,
    const wxSize& bitmap_size = wxSize(16, 15));
private:
  static std::map<wxWindowID, wxArtID> m_StockArt;
};

#if wxUSE_STATUSBAR
/// This class defines our statusbar panes, to be used by wxExFrame::SetupStatusBar.
/// It just adds some members to the base class, and keeps a static total.
class wxExPane : public wxStatusBarPane
{
  friend class wxExFrame;
  friend class wxExStatusBar;
public:
  /// Default constructor.
  wxExPane(
    /// If you do no provide helptext, it is derived from the name, by using
    /// text after the first 'e' character (so after 'Pane') if name is
    /// not 'PaneText'.
    const wxString& name = wxEmptyString,
    /// Width of the pane.
    int width = 50,
    /// The helptext shown as a tooltip.
    const wxString& helptext = wxEmptyString,
    /// The style.
    int style = wxSB_NORMAL)
    : wxStatusBarPane(style, width)
    , m_Helptext(helptext.empty() && name != "PaneText" ? name.AfterFirst('e'): helptext)
    , m_Name(name)
    , m_No(m_Total)
    {m_Total++;};
private:
  wxString m_Helptext;
  wxString m_Name;
  int m_No;
  static int m_Total;
};
#endif // wxUSE_STATUSBAR

/// Offers a frame with easy statusbar methods, 
/// and a toolbar if you call CreateToolBar.
class wxExFrame : public wxFrame
{
  friend class wxExStatusBar;
public:
  /// Constructor, the frame position and size is taken from the config.
  wxExFrame(wxWindow* parent,
    wxWindowID id,
    const wxString& title,
    long style = wxDEFAULT_FRAME_STYLE,
    const wxString& name = wxFrameNameStr);

  /// Using specified position and size.
  wxExFrame(wxWindow* parent,
    wxWindowID id,
    const wxString& title,
    const wxPoint& pos,
    const wxSize& size,
    long style = wxDEFAULT_FRAME_STYLE,
    const wxString& name = wxFrameNameStr);

  /// Destructor, deletes the statusbar.
 ~wxExFrame();

  /// Called when a config dialog apply button was pressed.
  virtual void ConfigDialogApplied(wxWindowID WXUNUSED(dialogid)) {};

  /// Returns a listview, default returns the focused listview.
  virtual wxExListView* GetListView() {return GetFocusedListView();};

  /// Returns an STC, default returns the focused STC.
  virtual wxExSTC* GetSTC() {return GetFocusedSTC();};

  /// Default opens the file using the GetFocusedSTC.
  virtual bool OpenFile(
    const wxExFileName& filename,
    int line_number = 0,
    const wxString& match = wxEmptyString,
    long flags = 0);

  /// Override from base, we use a different style.
  virtual wxToolBar* CreateToolBar(
    long style = wxNO_BORDER | wxTB_FLAT | wxTB_NODIVIDER | wxTB_DOCKABLE,
    wxWindowID id = -1,
    const wxString& name = "toolBar") {return wxFrame::CreateToolBar(style, id, name);};

  /// If the window that has focus is a ListView, 
  /// then returns that, otherwise returns NULL.
  wxExListView* GetFocusedListView();

  /// If the window that has focus is an STC, then returns that, 
  /// otherwise returns NULL.
  wxExSTC* GetFocusedSTC();

#if wxUSE_STATUSBAR
  /// Returns the status bar pane.
  /// If pane could not be found, returns empty pane.
  const wxExPane GetPane(int pane) const;

  /// Returns the field number of status bar pane.
  /// If pane could not be fonud, returns -1.
  static int GetPaneField(const wxString& pane);

  /// Do something when statusbar is clicked.
  virtual void StatusBarClicked(
    int WXUNUSED(field), 
    const wxPoint& WXUNUSED(point)) {};

  /// When double clicked, uses the GetSTC() for some dialogs.
  virtual void StatusBarDoubleClicked(int field, const wxPoint& point);

  /// Sets text on specified pane.
  /// Don't forget to call SetupStatusBar first.
  static void StatusText(const wxString& text, const wxString& pane = "PaneText");
#endif
protected:
  /// Writes the current frame size and position to the config.
  void OnClose(wxCloseEvent& event);
  /// If there is a focused STC, updates the status bar.
  void OnUpdateUI(wxUpdateUIEvent& event);

#if wxUSE_STATUSBAR
  // Interface from wxFrame.
  virtual wxStatusBar* OnCreateStatusBar(int number,
    long style,
    wxWindowID id,
    const wxString& name);
#endif

  // Interface from wxFrame.
  virtual wxToolBar* OnCreateToolBar(
    long style,
    wxWindowID id,
    const wxString& name);

#if wxUSE_STATUSBAR
  /// Sets up the status bar if you want to use StatusText.
  void SetupStatusBar(
    const std::vector<wxExPane>& panes,
    long style = wxST_SIZEGRIP,
    wxWindowID id = ID_EDIT_STATUS_BAR,
    const wxString& name = "statusBar");
#endif
protected:
  wxExToolBar* m_ToolBar;
private:
#if wxUSE_STATUSBAR
  static wxExStatusBar* m_StatusBar;
  static std::map<wxString, wxExPane> m_Panes;
#endif

  const bool m_KeepPosAndSize;

  DECLARE_EVENT_TABLE()
};

/// Offers an aui managed frame with a notebook multiple document interface,
/// used by the notebook classes.
class wxExManagedFrame : public wxExFrame
{
public:
  /// Constructor, the frame position and size is taken from the config.
  wxExManagedFrame(wxWindow* parent,
    wxWindowID id,
    const wxString& title,
    long style = wxDEFAULT_FRAME_STYLE,
    const wxString& name = wxFrameNameStr);

  /// Destructor.
 ~wxExManagedFrame();

  // Interface for notebooks.
  /// Returns true if the page can be closed.
  virtual bool AllowClose(
    wxWindowID WXUNUSED(id), 
    wxWindow* WXUNUSED(page)) {return true;}

  /// Called if the notebook changed page.
  virtual void OnNotebook(
    wxWindowID WXUNUSED(id), 
    wxWindow* WXUNUSED(page)) {;};

  /// Called after all pages from the notebooks are deleted.
  virtual void SyncCloseAll(wxWindowID WXUNUSED(id)) {;};

  /// Gets the manager.
  wxAuiManager& GetManager() {return m_Manager;};

  /// Toggles the managed pane: if shown hides it, otherwise shows it.
  void TogglePane(const wxString& pane);
private:
  wxAuiManager m_Manager;
};

/// Adds artid, edit, printing and tool menu items to wxMenu.
class wxExMenu : public wxExStockArt, public wxMenu
{
public:
  /// The menu styles.
  enum
  {
    MENU_IS_READ_ONLY = 0x0001, ///< readonly control
    MENU_IS_SELECTED  = 0x0002, ///< text is selected somewhere on the control
    MENU_IS_EMPTY     = 0x0004, ///< control is empty

    MENU_ALLOW_CLEAR  = 0x0008, ///< add clear item in menu
    MENU_CAN_PASTE    = 0x0010, ///< add paste item in menu

    MENU_DEFAULT      = MENU_CAN_PASTE, ///< default
  };

  /// Default constructor.
  wxExMenu(long style = MENU_DEFAULT);

  /// Copy constructor.
  wxExMenu(const wxExMenu& menu);

  /// Adds automatic naming (for stock menu id's) and art id for menu items.
  wxMenuItem* Append(int id, // this can be a stock item, then name and art is derived from it
    const wxString& name = wxEmptyString,
    const wxString& helptext = wxEmptyString,
    wxArtID artid = wxEmptyString);

  /// Appends edit menu items, depending on the style specified during construction.
  /// Returns true if at least one item has been added.
  bool AppendEdit(bool add_invert = false);

  /// Appends print menu items.
  /// This always adds some items, so no boolean return needed.
  void AppendPrint();

  /// Appends a tools submenu.
  wxExMenu* AppendTools();

  /// Gets the style.
  long GetStyle() const {return m_Style;};
private:
  const long m_Style;
};

#if wxUSE_STATUSBAR
/// Offers a status bar with popup menu in relation to wxExFrame.
class wxExStatusBar : public wxStatusBar
{
public:
  /// Constructor.
  wxExStatusBar(wxExFrame* parent,
    wxWindowID id = wxID_ANY,
    long style = wxST_SIZEGRIP,
    const wxString& name = wxPanelNameStr);
protected:
  void OnMouse(wxMouseEvent& event);
private:
  wxExFrame* m_Frame;

  DECLARE_EVENT_TABLE()
};
#endif // wxUSE_STATUSBAR

/// Offers a toolbar together with stock art.
class wxExToolBar : public wxExStockArt, public wxToolBar
{
public:
  /// Constructor.
  wxExToolBar(wxWindow* parent,
    wxWindowID id = wxID_ANY,
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    long style = wxTB_HORIZONTAL | wxNO_BORDER,
    const wxSize& bitmap_size = wxSize(16, 15),
    const wxString& name = wxPanelNameStr);

  /// Adds automatic naming (for stock menu id's) and art id for toolbar items.
  wxToolBarToolBase* AddTool(
    int toolId, // this can be a stock item, then shortHelpString and bitmap (art) is derived from it
    const wxString& label = wxEmptyString,
    const wxBitmap& bitmap1 = wxNullBitmap,
    const wxString& shortHelpString = wxEmptyString,
    wxItemKind kind = wxITEM_NORMAL);

  /// As above.
  wxToolBarToolBase* AddTool(
    int toolId,
    const wxString& longHelpString,
    const wxString& label = wxEmptyString,
    const wxBitmap& bitmap1 = wxNullBitmap,
    const wxBitmap& bitmap2 = wxNullBitmap,
    wxItemKind kind = wxITEM_NORMAL,
    const wxString& shortHelpString = wxEmptyString,
    wxObject* clientData = NULL);

  /// And for check tools as well.
  wxToolBarToolBase* AddCheckTool(
    int toolId,
    const wxString& label = wxEmptyString,
    const wxBitmap& bitmap1 = wxNullBitmap,
    const wxBitmap& bitmap2 = wxNullBitmap,
    const wxString& shortHelpString = wxEmptyString,
    const wxString& longHelpString = wxEmptyString,
    wxObject* clientData = NULL);
};
#endif // wxUSE_GUI
#endif
