/******************************************************************************\
* File:          configdlg.h
* Purpose:       Declaration of wxExConfigDialog class
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _EXCONFIGDIALOG_H
#define _EXCONFIGDIALOG_H

#include <map>
#include <set>
#include <vector>
#include <wx/extension/configitem.h>
#include <wx/extension/dialog.h>

#if wxUSE_GUI
/// Offers a dialog to set several items in the config.
/// You can also use the dialog modeless (then you can use wxAPPLY
/// to store the items in the config).
/// When pressing the apply button OnCommandConfigDialog is invoked from wxExFrame.
/// If you only specify a wxCANCEL button, the dialog is readonly.
class wxExConfigDialog: public wxExDialog
{
public:
  /// Constructor, specify the vector of config items
  /// to be used. When wxOK or wxAPPLY is pressed, any change in one of the
  /// config items is saved in the config.
  /// \todo The dialog does not set it's window size correctly when
  /// notebooks are used, you have to specify size yourself.
  wxExConfigDialog(wxWindow* parent,
    const std::vector<wxExConfigItem>& v,
    const wxString& title = _("Options"),
    int rows = 0,
    int cols = 2,
    long flags = wxOK | wxCANCEL,
    wxWindowID id = wxID_ANY,
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER,
    const wxString& name = wxDialogNameStr);

  /// If you specified some checkboxes, calling this method
  /// requires that one of them should be checked for the OK button
  /// to be enabled.
  void ForceCheckBoxChecked(
    const wxString& contains = wxEmptyString,
    const wxString& page = wxEmptyString);
protected:
  void OnCommand(wxCommandEvent& event);
  void OnUpdateUI(wxUpdateUIEvent& event);
private:
  void Add(int rows, int cols, const wxPoint& pos, const wxSize& size);
  void AddSizerItem(wxSizer* sizer, const wxExConfigItem& item);
  void AddSizerItemBrowse(wxSizer* sizer,  const wxExConfigItem& item);
  void AddSizerItemLabeled(wxSizer* sizer, 
    const wxExConfigItem& item, bool expand = true, bool hide = false);

  wxComboBox* m_BrowseDir;

  std::vector<wxExConfigItem> m_ConfigItems;
  bool m_ForceCheckBoxChecked;
  wxString m_Contains;
  wxString m_Page;

  DECLARE_EVENT_TABLE()
};

/// Returns a special config dialog using only one combobox.
wxExConfigDialog* wxExConfigComboBoxDialog(
  /// Parent.
  wxWindow* parent,
  /// Title.
  const wxString& title,
  /// The item that is used for the label before the combobox.
  const wxString& item,
  /// The flags.
  long flags = wxOK | wxCANCEL,
  /// The window id.
  wxWindowID id = wxID_ANY,
  const wxPoint& pos = wxDefaultPosition,
  const wxSize& size = wxDefaultSize,
  long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER,
  const wxString& name = wxDialogNameStr);

#endif // wxUSE_GUI
#endif
