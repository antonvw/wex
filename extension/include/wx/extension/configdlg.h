////////////////////////////////////////////////////////////////////////////////
// Name:      configdlg.h
// Purpose:   Declaration of wxExConfigDialog class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2013 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXCONFIGDIALOG_H
#define _EXCONFIGDIALOG_H

#include <vector>
#include <wx/extension/configitem.h>
#include <wx/extension/dialog.h>

#if wxUSE_GUI
/// Offers a dialog to set several items in the config.
/// If you only specify a wxCANCEL button, the dialog is readonly.
/// You can also use the dialog modeless (then you can use wxAPPLY
/// to store the items in the config).
/// When pressing the apply button OnCommandConfigDialog is invoked from wxExFrame.
class WXDLLIMPEXP_BASE wxExConfigDialog: public wxExDialog
{
public:
  enum
  {
    CONFIG_NOTEBOOK,    ///< traditional notebook
    CONFIG_AUINOTEBOOK, ///< aui notebook
    CONFIG_TREEBOOK,    ///< a tree book
    CONFIG_CHOICEBOOK,  ///< a choice book
    CONFIG_LISTBOOK,    ///< a list book
    CONFIG_TOOLBOOK     ///< a tool book
  };

  /// Constructor.
  wxExConfigDialog(
    /// parent
    wxWindow* parent,
    /// vector with config items 
    const std::vector<wxExConfigItem>& v,
    /// title
    const wxString& title = _("Options"),
    /// number of rows
    int rows = 0,
    /// number of columns
    int cols = 1,
    /// dialog flags for buttons
    /// When wxOK or wxAPPLY is pressed, any change in one of the
    /// config items is saved in the config.
    long flags = wxOK | wxCANCEL,
    /// the window id
    wxWindowID id = wxID_ANY,
    /// bookctrl style, only used if you specified pages for your config items
    int bookctrl_style = CONFIG_AUINOTEBOOK,
    /// position
    const wxPoint& pos = wxDefaultPosition,
    /// size
    const wxSize& size = wxDefaultSize, 
    /// dialog style
    long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER,
    /// name
    const wxString& name = "wxExConfigDialog");

  /// If you specified some checkboxes, calling this method
  /// requires that one of them should be checked for the OK button
  /// to be enabled.
  void ForceCheckBoxChecked(
    /// specify the (part of) the name of the checkbox
    const wxString& contains = wxEmptyString,
    /// specify on which page
    const wxString& page = wxEmptyString);
    
  /// Reloads dialog from config.
  void Reload() const;
protected:
  void OnCommand(wxCommandEvent& event);
  void OnUpdateUI(wxUpdateUIEvent& event);
private:
  void Click(int id) const;
  std::vector< wxExConfigItem >::const_iterator FindConfigItem(int id) const;
  void Layout(int rows, int cols, int notebook_style);

  std::vector<wxExConfigItem> m_ConfigItems;
  bool m_ForceCheckBoxChecked;
  wxString m_Contains;
  wxString m_Page;

  DECLARE_EVENT_TABLE()
};
#endif // wxUSE_GUI
#endif
