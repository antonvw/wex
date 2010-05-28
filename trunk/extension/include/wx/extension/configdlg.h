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
  wxExConfigDialog(wxWindow* parent,
    const std::vector<wxExConfigItem>& v,
    const wxString& title = _("Options"),
    int rows = 0,
    int cols = 1,
    long flags = wxOK | wxCANCEL,
    wxWindowID id = wxID_ANY,
    long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);

  /// If you specified some checkboxes, calling this method
  /// requires that one of them should be checked for the OK button
  /// to be enabled.
  void ForceCheckBoxChecked(
    const wxString& contains = wxEmptyString,
    const wxString& page = wxEmptyString);

  /// Reloads all items from config.
  void Reload();
    
  /// Selects all in the control, if appropriate.
  void SelectAll();
protected:
  void OnCommand(wxCommandEvent& event);
  void OnUpdateUI(wxUpdateUIEvent& event);
private:
  const wxExConfigItem FindConfigItem(int id) const;
  void Layout(int rows, int cols);

  std::vector<wxExConfigItem> m_ConfigItems;
  bool m_ForceCheckBoxChecked;
  wxString m_Contains;
  wxString m_Page;

  DECLARE_EVENT_TABLE()
};

/// Returns a special config dialog using only one combobox.
/// Default pressing return acts as OK and escape as CANCEL.
wxExConfigDialog* wxExConfigComboBoxDialog(
  /// Parent.
  wxWindow* parent,
  /// Title.
  const wxString& title,
  /// The item that is used for the label before the combobox.
  const wxString& item,
  /// The flags.
  long flags = 0,
  /// The window id.
  wxWindowID id = wxID_ANY,
  long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);

#endif // wxUSE_GUI
#endif
