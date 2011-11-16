////////////////////////////////////////////////////////////////////////////////
// Name:      configdlg.cpp
// Purpose:   Implementation of wxExtension config dialog class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <functional>
#include <algorithm>
#include <wx/bookctrl.h> 
#include <wx/choicebk.h>
#include <wx/listbook.h>
#include <wx/filepicker.h>
#include <wx/persist/treebook.h>
#include <wx/toolbook.h>
#include <wx/extension/configdlg.h>
#include <wx/extension/frame.h>

#if wxUSE_GUI

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
  int bookctrl_style,
  long style,
  const wxString& name)
  : wxExDialog(
      parent, 
      title, 
      flags, 
      id, 
      wxDefaultPosition, 
      wxDefaultSize, 
      style,
      name)
  , m_ForceCheckBoxChecked(false)
  , m_Page(wxEmptyString)
  , m_ConfigItems(v)
{
  Layout(rows, cols, bookctrl_style);
}

std::vector< wxExConfigItem >::const_iterator 
wxExConfigDialog::FindConfigItem(int id) const
{
  for (
#ifdef wxExUSE_CPP0X	
    auto it = m_ConfigItems.begin();
#else
    std::vector<wxExConfigItem>::const_iterator it = m_ConfigItems.begin();
#endif	
    it != m_ConfigItems.end();
    ++it)
  {
    if (it->GetWindow()->GetId() == id)
    {
      return it;
    }
  }

  return m_ConfigItems.end();
}

void wxExConfigDialog::Click(int id) const
{
  wxExFrame* frame = wxDynamicCast(wxTheApp->GetTopWindow(), wxExFrame);
  
  if (frame != NULL)
  {
    frame->OnCommandConfigDialog(GetId(), id);
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

void wxExConfigDialog::Layout(int rows, int cols, int bookctrl_style)
{
  if (m_ConfigItems.empty())
  {
    AddUserSizer(CreateTextSizer(
      _("No further info available")),
      wxSizerFlags().Center());
    LayoutSizers();
    return;
  }
  
  bool first_time = true;
  wxFlexGridSizer* sizer = NULL;
  wxFlexGridSizer* previous_item_sizer = NULL;
  int previous_item_type = -1;
  
  wxBookCtrlBase* bookctrl = NULL;
  
  if (!m_ConfigItems.begin()->GetPage().empty())
  {
    switch (bookctrl_style)
    {
    case CONFIG_CHOICEBOOK:
      bookctrl = new wxChoicebook(this, wxID_ANY);
      break;

    case CONFIG_LISTBOOK:
      bookctrl = new wxListbook(this, wxID_ANY);
      break;

    case CONFIG_NOTEBOOK:
      bookctrl = new wxNotebook(this, wxID_ANY);
      break;

    case CONFIG_TOOLBOOK:
      bookctrl = new wxToolbook(this, wxID_ANY);
      break;

    case CONFIG_TREEBOOK:
      bookctrl = new wxTreebook(this, wxID_ANY);
      break;

    default:
      wxFAIL;  
    }
  }
       
  wxString previous_page = "XXXXXX";

  for (
#ifdef wxExUSE_CPP0X	
    auto it = m_ConfigItems.begin();
#else
    std::vector<wxExConfigItem>::iterator it = m_ConfigItems.begin();
#endif	
    it != m_ConfigItems.end();
    ++it)
  {
    if (first_time ||
        (it->GetPage() != previous_page && !it->GetPage().empty()))
    {
      first_time = false;

      if (bookctrl != NULL)
      {
        // Finish the current page.
        if (bookctrl->GetCurrentPage() != NULL)
        {
          bookctrl->GetCurrentPage()->SetSizerAndFit(sizer);
        }

        // And make a new one.
        bookctrl->AddPage(
          new wxWindow(bookctrl, wxID_ANY), it->GetPage(), true); // select
      }

      previous_page = it->GetPage();

      const int use_cols = (it->GetColumns() != -1 ? it->GetColumns(): cols);

      sizer = (rows != 0 ? 
        new wxFlexGridSizer(rows, use_cols, 0, 0):
        new wxFlexGridSizer(use_cols));

      for (int i = 0; i < use_cols; i++)
      {
        sizer->AddGrowableCol(i);
      }
    }

    wxFlexGridSizer* use_item_sizer = (it->GetType() == previous_item_type ?
      previous_item_sizer: NULL);

    previous_item_sizer = it->Layout(
      (bookctrl != NULL ? bookctrl->GetCurrentPage(): this), 
      sizer, 
      GetButtonFlags() == wxCANCEL,
      use_item_sizer);

    previous_item_type = it->GetType();

    if (
      it->GetType() == CONFIG_BUTTON ||
      it->GetType() == CONFIG_COMBOBOXDIR)
    {
      Bind(
        wxEVT_COMMAND_BUTTON_CLICKED, 
        &wxExConfigDialog::OnCommand, 
        this, 
        it->GetWindow()->GetId());
    }

    if ( sizer->GetRows() > 0 &&
        !sizer->IsRowGrowable(sizer->GetRows() - 1))
    {
      sizer->AddGrowableRow(sizer->GetRows() - 1);
    }
  }

  if (bookctrl != NULL)
  {
    bookctrl->GetCurrentPage()->SetSizerAndFit(sizer);
    bookctrl->SetName("book" + GetName());

    if (!wxPersistenceManager::Get().RegisterAndRestore(bookctrl))
    {
      // nothing was restored, so choose the default page ourselves
      bookctrl->SetSelection(0);
    }

    AddUserSizer(bookctrl);
  }
  else
  {
    AddUserSizer(sizer);
  }

  LayoutSizers(bookctrl == NULL); // add separator line if no bookctrl
}

void wxExConfigDialog::OnCommand(wxCommandEvent& command)
{
  if (command.GetId() < wxID_LOWEST)
  {
#ifdef wxExUSE_CPP0X	
    auto it = FindConfigItem(command.GetId());
#else
    std::vector<wxExConfigItem>::const_iterator it = FindConfigItem(command.GetId());
#endif	
    if (it != m_ConfigItems.end())
    {
      if (it->GetType() == CONFIG_COMBOBOXDIR)
      {
        wxComboBox* browse = (wxComboBox*)it->GetWindow();

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
      else if (it->GetType() == CONFIG_BUTTON)
      {
        Click(command.GetId());
      }
      else
      {
        wxFAIL;
      }
    }
  }
  else if (command.GetId() == wxID_CANCEL)
  {
    Reload();
  }
  else
  {
    for_each (m_ConfigItems.begin(), m_ConfigItems.end(), 
      std::bind2nd(std::mem_fun_ref(&wxExConfigItem::ToConfig), true));
  }

  if (  command.GetId() == wxID_APPLY ||
      ((command.GetId() == wxID_OK ||
        command.GetId() == wxID_CANCEL) && !IsModal()))
  {
    Click(command.GetId());
  }

  command.Skip();
}

void wxExConfigDialog::OnUpdateUI(wxUpdateUIEvent& event)
{
  bool one_checkbox_checked = false;

  for (
#ifdef wxExUSE_CPP0X	
    auto it = m_ConfigItems.begin();
#else
    std::vector<wxExConfigItem>::iterator it = m_ConfigItems.begin();
#endif	
    it != m_ConfigItems.end();
    ++it)
  {
    switch (it->GetType())
    {
    case CONFIG_CHECKBOX:
      if (m_ForceCheckBoxChecked)
      {
        wxCheckBox* cb = (wxCheckBox*)it->GetWindow();

        if (it->GetLabel().Lower().Contains(m_Contains.Lower()) && 
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
        wxCheckListBox* clb = (wxCheckListBox*)it->GetWindow();

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
      wxComboBox* cb = (wxComboBox*)it->GetWindow();
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
      wxTextCtrl* tc = (wxTextCtrl*)it->GetWindow();
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
      wxDirPickerCtrl* pc = (wxDirPickerCtrl*)it->GetWindow();
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
      wxFilePickerCtrl* pc = (wxFilePickerCtrl*)it->GetWindow();
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

  event.Enable(m_ForceCheckBoxChecked ? one_checkbox_checked: true);
}

void wxExConfigDialog::Reload() const
{
  for_each (m_ConfigItems.begin(), m_ConfigItems.end(), 
    std::bind2nd(std::mem_fun_ref(&wxExConfigItem::ToConfig), false));
}
#endif // wxUSE_GUI
