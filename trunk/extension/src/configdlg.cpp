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

#include <wx/aui/auibook.h>
#include <wx/clrpicker.h> // for wxColourPickerWidget
#include <wx/config.h>
#include <wx/filepicker.h>
#include <wx/fontpicker.h>
#include <wx/spinctrl.h>
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
  Config(false); // read
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
    if (it == m_ConfigItems.begin() && !it->m_Page.empty())
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
        (it->m_Page != previous_page &&
         it->m_Page != wxEmptyString))
    {
      first_time = false;

      if (notebook != NULL && it->m_Type != CONFIG_SPACER)
      {
        // Finish the current page.
        if (sizer != NULL)
        {
          page_panel->SetSizerAndFit(sizer);
        }

        // And make a new one.
        page_panel = new wxPanel(notebook);
        notebook->AddPage(page_panel, it->m_Page);
      }

      previous_page = it->m_Page;

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

    wxWindow* parent =
      (page_panel != NULL ? (wxWindow*)page_panel: this);

    it->Create(parent, GetButtonFlags() == wxCANCEL);

    switch (it->m_Type)
    {
    case CONFIG_CHECKBOX: AddCheckBox(parent, sizer, *it); break;

    case CONFIG_CHECKLISTBOX: Add(parent, sizer, *it); break;

    case CONFIG_CHECKLISTBOX_NONAME: AddCheckListBoxNoName(parent, sizer, *it); break;

    case CONFIG_COLOUR: Add(parent, sizer, *it, false); break;

    case CONFIG_COMBOBOX: Add(parent, sizer, *it, true, false); break;

    case CONFIG_COMBOBOXDIR: AddComboBoxDir(parent, sizer, *it); break;

    case CONFIG_COMBOBOX_NONAME: Add(parent, sizer, *it, true); break;

    case CONFIG_DIRPICKERCTRL: Add(parent, sizer, *it); break;

    case CONFIG_FILEPICKERCTRL: Add(parent, sizer, *it); break;

    case CONFIG_FONTPICKERCTRL: Add(parent, sizer, *it); break;

    case CONFIG_INT: Add(parent, sizer, *it); break;

    case CONFIG_RADIOBOX: AddRadioBox(parent, sizer, *it); break;

    case CONFIG_SPACER: sizer->AddSpacer(wxSizerFlags::GetDefaultBorder()); break;

    case CONFIG_SPINCTRL: Add(parent, sizer, *it, false); break;

    case CONFIG_SPINCTRL_DOUBLE: Add(parent, sizer, *it, false); break;

    case CONFIG_STRING: Add(parent, sizer, *it, true); break;

    default:
      wxFAIL;
      return;
      break;
    }

    if (sizer != NULL)
    {
      if ( sizer->GetRows() > 0 &&
          !sizer->IsRowGrowable(sizer->GetRows() - 1))
      {
        sizer->AddGrowableRow(sizer->GetRows() - 1);
      }
    }
  }

  if (page_panel != NULL && notebook_sizer != NULL && sizer != NULL)
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

void wxExConfigDialog::Add(
  wxWindow* parent,
  wxSizer* sizer,
  const wxExConfigItem& item,
  bool expand,
  bool hide)
{
  wxSizerFlags flags;
  flags.Border();

  if (!hide)
  {
    sizer->Add(new wxStaticText(parent, wxID_ANY, item.m_Name + ":"), flags.Right());
  }

  sizer->Add(item.m_Control, (expand ? flags.Left().Expand(): flags.Left()));
}

void wxExConfigDialog::AddCheckBox(wxWindow* parent,
  wxSizer* sizer, const wxExConfigItem& item)
{
  wxSizerFlags flags;
  flags.Expand().Left().Border();
  sizer->Add(item.m_Control, flags);
}

void wxExConfigDialog::AddCheckListBoxNoName(wxWindow* parent,
  wxSizer* sizer, const wxExConfigItem& item)
{
  wxSizerFlags flags;
  flags.Expand().Left().Border();
  sizer->Add(item.m_Control, flags);
}

void wxExConfigDialog::AddComboBoxDir(wxWindow* parent,
  wxSizer* sizer, const wxExConfigItem& item)
{
  m_BrowseDir = (wxComboBox *)item.m_Control;

  wxSizerFlags flag;

  wxFlexGridSizer* browse = new wxFlexGridSizer(2, 0, 0);
  browse->AddGrowableCol(0);
  browse->Add(m_BrowseDir, flag.Expand());

  // Tried to use a wxDirPickerCtrl here, is nice,
  // but does not use a combobox for keeping old values, so this is better.
  // And the text box that is used is not resizable as well.
  browse->Add(
    new wxButton(
      parent,
      ID_BROWSE_FOLDER,
      wxDirPickerWidgetLabel,
      wxDefaultPosition,
      wxDefaultSize,
      wxBU_EXACTFIT),
    flag.Center().Border(wxLEFT));

  sizer->Add(new wxStaticText(parent, wxID_ANY, item.m_Name + ":"), flag.Right().Border());
  sizer->Add(browse, flag.Center().Border());
}

void wxExConfigDialog::AddRadioBox(
  wxWindow* parent,
  wxSizer* sizer, 
  const wxExConfigItem& item)
{
  wxSizerFlags flags;
  flags.Expand().Left().Border();
  sizer->Add(item.m_Control, flags);
}

void wxExConfigDialog::Config(bool save)
{
  for (
    std::vector<wxExConfigItem>::const_iterator it = m_ConfigItems.begin();
    it != m_ConfigItems.end();
    ++it)
  {
    it->ToConfig(save);
  }
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
    Config(false); // (re)load from config
    break;

  default:
    // For rest of the buttons (wxID_OK, wxID_APPLY, wxID_CLOSE)
    // save to config.
    Config(true); // save to config
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
    switch (it->m_Type)
    {
    case CONFIG_CHECKBOX:
      if (m_ForceCheckBoxChecked)
      {
        wxCheckBox* cb = (wxCheckBox*)it->m_Control;

        if (cb->GetName().Lower().Contains(m_Contains.Lower()) && 
            cb->GetValue() &&
            it->m_Page == m_Page)
        {
          one_checkbox_checked = true;
        }
      }
      break;

    case CONFIG_CHECKLISTBOX_NONAME:
      if (m_ForceCheckBoxChecked)
      {
        wxCheckListBox* clb = (wxCheckListBox*)it->m_Control;

        for (
          size_t i = 0;
          i < clb->GetCount();
          i++)
        {
          if (clb->GetString(i).Lower().Contains(m_Contains.Lower()) && 
              clb->IsChecked(i) &&
              it->m_Page == m_Page)
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
      wxComboBox* cb = (wxComboBox*)it->m_Control;
      if (it->m_IsRequired)
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
      wxTextCtrl* tc = (wxTextCtrl*)it->m_Control;
      if (it->m_IsRequired)
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
      wxDirPickerCtrl* pc = (wxDirPickerCtrl*)it->m_Control;
      if (it->m_IsRequired)
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
      wxFilePickerCtrl* pc = (wxFilePickerCtrl*)it->m_Control;
      if (it->m_IsRequired)
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

void wxExConfigDialog::Update(wxComboBox* cb, const wxString& value) const
{
  if (!value.empty())
  {
    if (cb->FindString(value) == wxNOT_FOUND)
    {
      cb->Append(value);
    }

    cb->SetValue(value);
  }
}
#endif // wxUSE_GUI
