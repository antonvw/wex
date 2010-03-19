/******************************************************************************\
* File:          configdlg.cpp
* Purpose:       Implementation of wxExtension config dialog class
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <functional>
#include <algorithm>
#include <wx/filepicker.h>
#include <wx/notebook.h> 
#include <wx/extension/configdlg.h>
#include <wx/extension/frame.h>

#if wxUSE_GUI

wxExConfigDialog* wxExConfigComboBoxDialog(wxWindow* parent,
  const wxString& title,
  const wxString& item,
  long flags,
  wxWindowID id,
  long style)
{
  std::vector<wxExConfigItem> v;

  v.push_back(wxExConfigItem(
    item, 
    CONFIG_COMBOBOX_NONAME));

  return new wxExConfigDialog(
    parent, 
    v,
    title, 
    0, 
    1, 
    flags, 
    id, 
    style);
}

BEGIN_EVENT_TABLE(wxExConfigDialog, wxExDialog)
  EVT_BUTTON(wxID_APPLY, wxExConfigDialog::OnCommand)
  EVT_BUTTON(wxID_CANCEL, wxExConfigDialog::OnCommand)
  EVT_BUTTON(wxID_CLOSE, wxExConfigDialog::OnCommand)
  EVT_BUTTON(wxID_OK, wxExConfigDialog::OnCommand)
  EVT_UPDATE_UI(wxID_APPLY, wxExConfigDialog::OnUpdateUI)
  EVT_UPDATE_UI(wxID_OK, wxExConfigDialog::OnUpdateUI)
END_EVENT_TABLE()

// wxPropertySheetDialog has been tried as well,
// then you always have a notebook, and apply button is not supported.
wxExConfigDialog::wxExConfigDialog(wxWindow* parent,
  const std::vector<wxExConfigItem>& v,
  const wxString& title,
  int rows,
  int cols,
  long flags,
  wxWindowID id,
  long style)
  : wxExDialog(parent, title, flags, id, style)
  , m_ForceCheckBoxChecked(false)
  , m_Page(wxEmptyString)
  , m_ConfigItems(v)
{
  Layout(rows, cols);

  for_each (m_ConfigItems.begin(), m_ConfigItems.end(), 
    std::bind2nd(std::mem_fun_ref(&wxExConfigItem::ToConfig), false)); // read
}

wxExConfigItem wxExConfigDialog::FindConfigItem(int id) const
{
  for (
    std::vector<wxExConfigItem>::const_iterator it = m_ConfigItems.begin();
    it != m_ConfigItems.end();
    ++it)
  {
    if (it->GetId() == id)
    {
      return *it;
    }
  }

  return wxExConfigItem();
}

void wxExConfigDialog::ForceCheckBoxChecked(
  const wxString& contains,
  const wxString& page)
{
  m_ForceCheckBoxChecked = true;
  m_Contains = contains;
  m_Page = page;
}

void wxExConfigDialog::Layout(int rows, int cols)
{
  bool first_time = true;
  wxFlexGridSizer* sizer = NULL;
  wxNotebook* notebook = 
    (m_ConfigItems.begin()->GetPage().empty() ? NULL: new wxNotebook(this, wxID_ANY));
  wxString previous_page = "XXXXXX";

  for (
    std::vector<wxExConfigItem>::iterator it = m_ConfigItems.begin();
    it != m_ConfigItems.end();
    ++it)
  {
    if (first_time ||
        (it->GetPage() != previous_page && !it->GetPage().empty()))
    {
      first_time = false;

      if (notebook != NULL)
      {
        // Finish the current page.
        if (notebook->GetCurrentPage() != NULL)
        {
          notebook->GetCurrentPage()->SetSizerAndFit(sizer);
        }

        // And make a new one.
        notebook->AddPage(new wxWindow(notebook, wxID_ANY), it->GetPage(), true); // select
      }

      previous_page = it->GetPage();

      sizer = (rows != 0 ? 
        new wxFlexGridSizer(rows, cols, 0, 0):
        new wxFlexGridSizer(cols));

      if (cols == 2)
      {
        sizer->AddGrowableCol(1);
      }
      else
      {
        for (int i = 0; i < cols; i++)
        {
          sizer->AddGrowableCol(i);
        }
      }
    }

    it->Layout(
      (notebook != NULL ? notebook->GetCurrentPage(): this), 
      sizer, 
      GetButtonFlags() == wxCANCEL);

    if (it->GetType() == CONFIG_COMBOBOXDIR)
    {
      Bind(wxEVT_COMMAND_BUTTON_CLICKED, &wxExConfigDialog::OnCommand, this, it->GetId());
    }

    if ( sizer->GetRows() > 0 &&
        !sizer->IsRowGrowable(sizer->GetRows() - 1))
    {
      sizer->AddGrowableRow(sizer->GetRows() - 1);
    }
  }

  if (notebook != NULL)
  {
    notebook->GetCurrentPage()->SetSizerAndFit(sizer);
    notebook->SetSelection(0);

    AddUserSizer(notebook);
  }
  else
  {
    AddUserSizer(sizer);
  }

  LayoutSizers(notebook == NULL); // add separator line if no notebook
}

void wxExConfigDialog::OnCommand(wxCommandEvent& command)
{
  if (command.GetId() < wxID_LOWEST)
  {
    wxComboBox* browse = (wxComboBox*)FindConfigItem(command.GetId()).GetControl();

    if (browse != NULL)
    {
      wxDirDialog dir_dlg(
        this,
        wxDirSelectorPromptStr,
        browse->GetValue(),
        wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);

      if (dir_dlg.ShowModal() == wxID_OK)
      {
        browse->SetValue(dir_dlg.GetPath());
      }
    }

    return;
  }

  switch (command.GetId())
  {
  case wxID_CANCEL:
    // For wxID_CANCEL reload from config.
    for_each (m_ConfigItems.begin(), m_ConfigItems.end(), 
      std::bind2nd(std::mem_fun_ref(&wxExConfigItem::ToConfig), false));
    break;

  default:
    // For rest of the buttons (wxID_OK, wxID_APPLY, wxID_CLOSE)
    // save to config.
    for_each (m_ConfigItems.begin(), m_ConfigItems.end(), 
      std::bind2nd(std::mem_fun_ref(&wxExConfigItem::ToConfig), true));
  }

  if ( command.GetId() == wxID_APPLY ||
      ((command.GetId() == wxID_OK ||
        command.GetId() == wxID_CANCEL) && !IsModal()))
  {
    wxASSERT(wxTheApp != NULL);
    wxWindow* window = wxTheApp->GetTopWindow();
    wxASSERT(window != NULL);
    wxExFrame* frame = wxDynamicCast(window, wxExFrame);
    wxASSERT(frame != NULL);

    frame->OnCommandConfigDialog(GetId(), command.GetId());
  }

  command.Skip();
}

void wxExConfigDialog::OnUpdateUI(wxUpdateUIEvent& event)
{
  bool one_checkbox_checked = false;

  for (
    std::vector<wxExConfigItem>::const_iterator it = m_ConfigItems.begin();
    it != m_ConfigItems.end();
    ++it)
  {
    switch (it->GetType())
    {
    case CONFIG_CHECKBOX:
      if (m_ForceCheckBoxChecked)
      {
        wxCheckBox* cb = (wxCheckBox*)it->GetControl();

        if (it->GetName().Lower().Contains(m_Contains.Lower()) && 
            cb->GetValue() &&
            it->GetPage() == m_Page)
        {
          one_checkbox_checked = true;
        }
      }
      break;

    case CONFIG_CHECKLISTBOX_NONAME:
      if (m_ForceCheckBoxChecked)
      {
        wxCheckListBox* clb = (wxCheckListBox*)it->GetControl();

        for (
          size_t i = 0;
          i < clb->GetCount();
          i++)
        {
          if (clb->GetString(i).Lower().Contains(m_Contains.Lower()) && 
              clb->IsChecked(i) &&
              it->GetPage() == m_Page)
          {
            one_checkbox_checked = true;
          }
        }
      }
      break;

    case CONFIG_COMBOBOX:
    case CONFIG_COMBOBOXDIR:
    case CONFIG_COMBOBOX_NONAME:
      {
      wxComboBox* cb = (wxComboBox*)it->GetControl();
      if (it->GetIsRequired())
      {
        if (cb->GetValue().empty())
        {
          event.Enable(false);
          return;
        }
      }
      }
      break;

    case CONFIG_INT:
    case CONFIG_STRING:
      {
      wxTextCtrl* tc = (wxTextCtrl*)it->GetControl();
      if (it->GetIsRequired())
      {
        if (tc->GetValue().empty())
        {
          event.Enable(false);
          return;
        }
      }
      }
      break;

    case CONFIG_DIRPICKERCTRL:
      {
      wxDirPickerCtrl* pc = (wxDirPickerCtrl*)it->GetControl();
      if (it->GetIsRequired())
      {
        if (pc->GetPath().empty())
        {
          event.Enable(false);
          return;
        }
      }
      }
      break;

    case CONFIG_FILEPICKERCTRL:
      {
      wxFilePickerCtrl* pc = (wxFilePickerCtrl*)it->GetControl();
      if (it->GetIsRequired())
      {
        if (pc->GetPath().empty())
        {
          event.Enable(false);
          return;
        }
      }
      }
      break;
    }
  }

  if (m_ForceCheckBoxChecked)
  {
    event.Enable(one_checkbox_checked);
  }
  else
  {
    event.Enable(true);
  }
}
#endif // wxUSE_GUI
