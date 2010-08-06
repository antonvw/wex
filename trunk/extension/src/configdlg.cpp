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

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <functional>
#include <algorithm>
#include <wx/filepicker.h>
#include <wx/notebook.h> 
#include <wx/persist/treebook.h>
#include <wx/spinctrl.h>
#include <wx/extension/configdlg.h>
#include <wx/extension/frame.h>

#if wxUSE_GUI

wxExConfigDialog* wxExConfigComboBoxDialog(wxWindow* parent,
  const wxString& title,
  const wxString& item,
  int max_items,
  long flags,
  wxWindowID id,
  long style)
{
  std::vector<wxExConfigItem> v;

  v.push_back(wxExConfigItem(
    item, 
    CONFIG_COMBOBOX,
    wxEmptyString,
    true, // is required
    wxID_ANY,
    max_items,
    false,// add name
    -1)); 

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
  : wxExDialog(
      parent, 
      title, 
      flags, 
      id, 
      wxDefaultPosition, 
      wxDefaultSize, 
      style)
  , m_ForceCheckBoxChecked(false)
  , m_Page(wxEmptyString)
  , m_ConfigItems(v)
{
  wxASSERT(!m_ConfigItems.empty());
  
  Layout(rows, cols);
}

std::vector< wxExConfigItem >::const_iterator 
wxExConfigDialog::FindConfigItem(int id) const
{
  for (
    auto it = m_ConfigItems.begin();
    it != m_ConfigItems.end();
    ++it)
  {
    if (it->GetId() == id)
    {
      return it;
    }
  }

  return m_ConfigItems.end();
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
  wxFlexGridSizer* previous_item_sizer = NULL;
  int previous_item_type = -1;
  wxNotebook* notebook = 
    (m_ConfigItems.begin()->GetPage().empty() ? 
       NULL: 
       new wxNotebook(this, wxID_ANY));
  wxString previous_page = "XXXXXX";

  for (
    auto it = m_ConfigItems.begin();
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
        notebook->AddPage(
          new wxWindow(notebook, wxID_ANY), it->GetPage(), true); // select
      }

      previous_page = it->GetPage();

      const int use_cols = (it->GetColumns() != -1 ? it->GetColumns(): cols);

      sizer = (rows != 0 ? 
        new wxFlexGridSizer(rows, use_cols, 0, 0):
        new wxFlexGridSizer(use_cols));

      for (auto i = 0; i < use_cols; i++)
      {
        sizer->AddGrowableCol(i);
      }
    }

    wxFlexGridSizer* use_item_sizer = (it->GetType() == previous_item_type ?
      previous_item_sizer: NULL);

    previous_item_sizer = it->Layout(
      (notebook != NULL ? notebook->GetCurrentPage(): this), 
      sizer, 
      GetButtonFlags() == wxCANCEL,
      use_item_sizer);

    previous_item_type = it->GetType();

    if (it->GetType() == CONFIG_COMBOBOXDIR)
    {
      Bind(
        wxEVT_COMMAND_BUTTON_CLICKED, 
        &wxExConfigDialog::OnCommand, 
        this, 
        it->GetId());
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
    notebook->SetName("book" + GetName());

    if (!wxPersistenceManager::Get().RegisterAndRestore(notebook))
    {
      // nothing was restored, so choose the default page ourselves
      notebook->SetSelection(0);
    }

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
    auto it = FindConfigItem(command.GetId());

    if (it != m_ConfigItems.end())
    {
      auto browse = (wxComboBox*)it->GetControl();

      wxDirDialog dir_dlg(
        this,
        _(wxDirSelectorPromptStr),
        browse->GetValue(),
        wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);

      if (dir_dlg.ShowModal() == wxID_OK)
      {
        browse->SetValue(dir_dlg.GetPath());
      }
    }
  }
  else if (command.GetId() == wxID_CANCEL)
  {
    Reload();
  }
  else
  {
    // Save to config.
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
    auto it = m_ConfigItems.begin();
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

void wxExConfigDialog::Reload() const
{
  for_each (m_ConfigItems.begin(), m_ConfigItems.end(), 
    std::bind2nd(std::mem_fun_ref(&wxExConfigItem::ToConfig), false));
}

void wxExConfigDialog::SelectAll()
{
  for (
    auto it = m_ConfigItems.begin();
    it != m_ConfigItems.end();
    ++it)
  {
    switch (it->GetType())
    {
    case CONFIG_COMBOBOX:
    case CONFIG_COMBOBOXDIR:
      {
      wxComboBox* c = (wxComboBox*)it->GetControl();
      c->SelectAll();
      }
      break;

    case CONFIG_INT:
    case CONFIG_STRING:
      {
      wxTextCtrl* c = (wxTextCtrl*)it->GetControl();
      c->SelectAll();
      }
      break;
      
    case CONFIG_SPINCTRL:
      {
      wxSpinCtrl* c = (wxSpinCtrl*)it->GetControl();
      c->SetSelection(-1, -1);
      }
      break;
      
    case CONFIG_SPINCTRL_DOUBLE:
      {
      wxSpinCtrlDouble* c = (wxSpinCtrlDouble*)it->GetControl();
      c->SetSelection(-1, -1);
      }
      break;
    }
  }
}

#endif // wxUSE_GUI
