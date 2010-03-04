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
#include <wx/aui/auibook.h>
#include <wx/filepicker.h>
#include <wx/extension/configdlg.h>
#include <wx/extension/frame.h>

#if wxUSE_GUI

wxExConfigDialog* wxExConfigComboBoxDialog(wxWindow* parent,
  const wxString& title,
  const wxString& item,
  long flags,
  wxWindowID id,
  const wxPoint& pos,
  const wxSize& size,
  long style,
  const wxString& name)
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
    pos, 
    size, 
    style, 
    name);
}

const long ID_BROWSE_FOLDER = 1000; //wxNewId(); not constant

BEGIN_EVENT_TABLE(wxExConfigDialog, wxExDialog)
  EVT_BUTTON(wxID_APPLY, wxExConfigDialog::OnCommand)
  EVT_BUTTON(wxID_CANCEL, wxExConfigDialog::OnCommand)
  EVT_BUTTON(wxID_CLOSE, wxExConfigDialog::OnCommand)
  EVT_BUTTON(wxID_OK, wxExConfigDialog::OnCommand)
  EVT_BUTTON(ID_BROWSE_FOLDER, wxExConfigDialog::OnCommand)
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
  const wxPoint& pos,
  const wxSize& size,
  long style,
  const wxString& name)
  : wxExDialog(parent, title, flags, id, pos, size, style, name)
  , m_ForceCheckBoxChecked(false)
  , m_Page(wxEmptyString)
  , m_ConfigItems(v)
  , m_BrowseDir(NULL)
{
  Add(rows, cols, pos, size);

  for_each (m_ConfigItems.begin(), m_ConfigItems.end(), 
    std::bind2nd(std::mem_fun_ref(&wxExConfigItem::ToConfig), false)); // read
}

void wxExConfigDialog::Add(
  int rows, int cols, const wxPoint& pos, const wxSize& size)
{
  bool first_time = true;
  wxFlexGridSizer* sizer = NULL;
  wxFlexGridSizer* notebook_sizer = NULL;
  wxAuiNotebook* notebook = NULL;
  wxString previous_page = "XXXXXX";
  wxPanel* page_panel = NULL;

  for (
    std::vector<wxExConfigItem>::iterator it = m_ConfigItems.begin();
    it != m_ConfigItems.end();
    ++it)
  {
    // Check if we need a notebook.
    if (it == m_ConfigItems.begin() && !it->GetPage().empty())
    {
      // Do not give it a close button.
      notebook = new wxAuiNotebook(this,
        wxID_ANY,
        pos,
        size,
        wxAUI_NB_TAB_MOVE | wxAUI_NB_SCROLL_BUTTONS | wxAUI_NB_TAB_SPLIT);

      notebook_sizer = new wxFlexGridSizer(1);
      notebook_sizer->AddGrowableCol(0);
      notebook_sizer->Add(notebook, wxSizerFlags().Expand().Center());
      notebook_sizer->AddGrowableRow(0);
      notebook_sizer->SetMinSize(size);
    }

    if (first_time ||
        (it->GetPage() != previous_page &&
         it->GetPage() != wxEmptyString))
    {
      first_time = false;

      if (notebook != NULL && it->GetType() != CONFIG_SPACER)
      {
        // Finish the current page.
        if (sizer != NULL)
        {
          page_panel->SetSizerAndFit(sizer);
        }

        // And make a new one.
        page_panel = new wxPanel(notebook);
        notebook->AddPage(page_panel, it->GetPage());
      }

      previous_page = it->GetPage();

      if (rows != 0)
        sizer = new wxFlexGridSizer(rows, cols, 0, 0);
      else
        sizer = new wxFlexGridSizer(cols);

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

    it->Create(
      (page_panel != NULL ? (wxWindow*)page_panel: this), 
      GetButtonFlags() == wxCANCEL);

    it->Layout(sizer, ID_BROWSE_FOLDER);

    if (it->GetType() == CONFIG_COMBOBOXDIR)
    {
      wxASSERT(m_BrowseDir == NULL);
      m_BrowseDir = (wxComboBox *)it->GetControl();
    }

    if ( sizer->GetRows() > 0 &&
        !sizer->IsRowGrowable(sizer->GetRows() - 1))
    {
      sizer->AddGrowableRow(sizer->GetRows() - 1);
    }
  }

  if (page_panel != NULL && notebook_sizer != NULL)
  {
    page_panel->SetSizer(sizer);

    AddUserSizer(notebook_sizer);

    SetMinSize(size);

    SendSizeEvent();
  }
  else
  {
    AddUserSizer(sizer);
  }

  LayoutSizers();
}

void wxExConfigDialog::ForceCheckBoxChecked(
  const wxString& contains,
  const wxString& page)
{
  m_ForceCheckBoxChecked = true;
  m_Contains = contains;
  m_Page = page;
}

void wxExConfigDialog::OnCommand(wxCommandEvent& command)
{
  switch (command.GetId())
  {
  case ID_BROWSE_FOLDER:
    {
    wxASSERT(m_BrowseDir != NULL);

    wxDirDialog dir_dlg(
      this,
      wxDirSelectorPromptStr,
      m_BrowseDir->GetValue(),
      wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);

    if (dir_dlg.ShowModal() == wxID_OK)
    {
      m_BrowseDir->SetValue(dir_dlg.GetPath());
    }
    }
    break;

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
