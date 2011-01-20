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
/// If you only specify a wxCANCEL button, the dialog is readonly.
/// You can also use the dialog modeless (then you can use wxAPPLY
/// to store the items in the config).
/// When pressing the apply button OnCommandConfigDialog is invoked from wxExFrame.
class WXDLLIMPEXP_BASE wxExConfigDialog: public wxExDialog
{
public:
  enum
  {
    CONFIG_NOTEBOOK = 1,
    CONFIG_TREEBOOK,
    CONFIG_CHOICEBOOK,
    CONFIG_LISTBOOK,
    CONFIG_TOOLBOOK,
  };
  
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
    int notebook_style = CONFIG_NOTEBOOK,
    long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);

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
