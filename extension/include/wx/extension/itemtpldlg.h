////////////////////////////////////////////////////////////////////////////////
// Name:      itemtpldlg.h
// Purpose:   Declaration of wxExItemTemplateDialog class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <vector>
#include <wx/filepicker.h>
#include <wx/imaglist.h>
#include <wx/stc/stc.h>
#include <wx/tglbtn.h> // for wxEVT_TOGGLEBUTTON
#include <wx/extension/dialog.h>
#include <wx/extension/frame.h>

#if wxUSE_GUI
/// Offers a dialog template to set several items.
/// If you only specify a wxCANCEL button, the dialog is readonly.
/// When pressing the:
/// - wxAPPLY button
/// - wxOK, wxCANCEL button for a modeless dialog
/// - a ITEM_BUTTON
/// - a ITEM_COMBOBOXDIR
/// the method wxExFrame::OnCommandItemDialog is invoked.
template <class T> class WXDLLIMPEXP_BASE wxExItemTemplateDialog: public wxExDialog
{
public:
  /// Constructor.
  wxExItemTemplateDialog(
    /// parent
    wxWindow* parent,
    /// vector with items 
    const std::vector< T >& v,
    /// title
    const wxString& title = _("Options"),
    /// number of rows
    int rows = 0,
    /// number of columns (if -1 use number of cols from items)
    int cols = -1,
    /// dialog flags for buttons
    long flags = wxOK | wxCANCEL,
    /// the window id
    wxWindowID id = wxID_ANY,
    /// bookctrl style, only used if you specified pages for your items
    int bookctrl_style = 0,
    /// image list to be used by notebook (required for a tool book)
    wxImageList* imageList = nullptr,
    /// position
    const wxPoint& pos = wxDefaultPosition,
    /// size
    const wxSize& size = wxDefaultSize, 
    /// dialog style
    long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER,
    /// name
    const wxString& name = "wxExItemTemplateDialog")
  : wxExDialog(parent, title, flags, id, pos, size, style, name)
  , m_ForceCheckBoxChecked(false)
  , m_Page(wxEmptyString)
  , m_Items(v) {
    Layout(rows, cols, imageList);
    Bind(wxEVT_BUTTON, &wxExItemTemplateDialog::OnCommand, this, wxID_APPLY);
    Bind(wxEVT_BUTTON, &wxExItemTemplateDialog::OnCommand, this, wxID_CANCEL);
    Bind(wxEVT_BUTTON, &wxExItemTemplateDialog::OnCommand, this, wxID_CLOSE);
    Bind(wxEVT_BUTTON, &wxExItemTemplateDialog::OnCommand, this, wxID_OK);
    Bind(wxEVT_UPDATE_UI, &wxExItemTemplateDialog::OnUpdateUI, this, wxID_APPLY);
    Bind(wxEVT_UPDATE_UI, &wxExItemTemplateDialog::OnUpdateUI, this, wxID_OK);};

  /// If you specified some checkboxes, calling this method
  /// requires that one of them should be checked for the OK button
  /// to be enabled.
  void ForceCheckBoxChecked(
    /// specify the (part of) the name of the checkbox
    const wxString& contains = wxEmptyString,
    /// specify on which page
    const wxString& page = wxEmptyString) {
    m_ForceCheckBoxChecked = true;
    m_Contains = contains;
    m_Page = page;};
  
  /// Returns the (first) item (on specified page) that has specified label,
  /// or empty item if item does not exist.
  const T GetItem(const wxString& label, const wxString& page = wxEmptyString) const {
    for (const auto& item : m_Items)
    {
      if (item.GetLabel() == label)
      {
        if (page.empty())
        {
          return item;
        }
        else if (item.GetPage() == page)
        {
          return item;
        }
      }
    };
    return T();};
  /// Returns the item actual value for specified label, or 
  /// IsNull value if item does not exist.
  const wxAny GetItemValue(const wxString& label, const wxString& page = wxEmptyString) const {
    return GetItem(label, page).GetValue();};
  /// Sets the item actual value for specified label.
  bool SetItemValue(const wxString& label, const wxAny& value, const wxString& page = wxEmptyString) const {
    for (auto& item : m_Items)
    {
      if (item.GetLabel() == label)
      {
        if (page.empty())
        {
          return item.SetValue(value);
        }
        else if (item.GetPage() == page)
        {
          return item.SetValue(value);
        }
      }
    };
    return false;};
protected:
  const auto & GetItems() const {return m_Items;};
  
  void OnCommand(wxCommandEvent& event) {
    if (  event.GetId() == wxID_APPLY ||
        ((event.GetId() == wxID_OK ||
          event.GetId() == wxID_CANCEL) && !IsModal()))
    {
      Click(event);
    }
    event.Skip();};
  
  void OnUpdateUI(wxUpdateUIEvent& event) {
    bool one_checkbox_checked = false;
    for (const auto& item : m_Items)
    {
      switch (item.GetType())
      {
      case ITEM_CHECKBOX:
        if (m_ForceCheckBoxChecked)
        {
          wxCheckBox* cb = (wxCheckBox*)item.GetWindow();
          if (item.GetLabel().Lower().Contains(m_Contains.Lower()) && 
              cb->IsChecked() &&
              item.GetPage() == m_Page)
          {
            one_checkbox_checked = true;
          }
        }
        break;

      case ITEM_CHECKLISTBOX_BOOL:
        if (m_ForceCheckBoxChecked)
        {
          wxCheckListBox* clb = (wxCheckListBox*)item.GetWindow();
          for (
            size_t i = 0;
            i < clb->GetCount();
            i++)
          {
            if (clb->GetString(i).Lower().Contains(m_Contains.Lower()) && 
                clb->IsChecked(i) &&
                item.GetPage() == m_Page)
            {
              one_checkbox_checked = true;
            }
          }
        }
        break;

      case ITEM_COMBOBOX:
      case ITEM_COMBOBOXDIR:
        {
        wxComboBox* cb = (wxComboBox*)item.GetWindow();
        if (item.GetIsRequired())
        {
          if (cb->GetValue().empty())
          {
            event.Enable(false);
            return;
          }
        }
        }
        break;

      case ITEM_INT:
      case ITEM_STRING:
        {
        wxTextCtrl* tc = (wxTextCtrl*)item.GetWindow();
        if (item.GetIsRequired())
        {
          if (tc->GetValue().empty())
          {
            event.Enable(false);
            return;
          }
        }
        }
        break;

      case ITEM_DIRPICKERCTRL:
        {
        wxDirPickerCtrl* pc = (wxDirPickerCtrl*)item.GetWindow();
        if (item.GetIsRequired())
        {
          if (pc->GetPath().empty())
          {
            event.Enable(false);
            return;
          }
        }
        }
        break;

      case ITEM_FILEPICKERCTRL:
        {
        wxFilePickerCtrl* pc = (wxFilePickerCtrl*)item.GetWindow();
        if (item.GetIsRequired())
        {
          if (pc->GetPath().empty())
          {
            event.Enable(false);
            return;
          }
        }
        }
        break;

      default: ; // do nothing
      }
    }
    event.Enable(m_ForceCheckBoxChecked ? one_checkbox_checked: true);};
private:
  void Click(const wxCommandEvent& event) const {
    wxExFrame* frame = wxDynamicCast(wxTheApp->GetTopWindow(), wxExFrame);
    if (frame != nullptr)
    {
      frame->OnCommandItemDialog(GetId(), event);
    }};
  
  void Layout(int rows, int cols, wxImageList* imageList) {
    wxFlexGridSizer* previous_item_sizer = nullptr;
    wxFlexGridSizer* sizer = new wxFlexGridSizer(1);
    int previous_type = -1;
    for (auto& item : m_Items)
    {
      if (item.GetType() == ITEM_EMPTY) continue; //skip

      // If this item has same type as previous type use previous sizer,
      // otherwise use no sizer (Layout will create a new one).
      wxFlexGridSizer* current_item_sizer = (item.GetType() == previous_type ? previous_item_sizer: nullptr);
      // Layout the item.
      previous_item_sizer = item.Layout(
        this, 
        sizer, 
        GetButtonFlags() == wxCANCEL,
        current_item_sizer);
      previous_type = item.GetType();
      if (sizer->GetEffectiveRowsCount() >= 1 &&
         !sizer->IsRowGrowable(sizer->GetEffectiveRowsCount() - 1) &&
          item.IsRowGrowable())
      {
        sizer->AddGrowableRow(sizer->GetEffectiveRowsCount() - 1);
      }
      switch (item.GetType())
      {
        case ITEM_BUTTON:
        case ITEM_COMMAND_LINK_BUTTON:
          Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent& event) {
            Click(event);}, item.GetWindow()->GetId());
          break;
        case ITEM_COMBOBOXDIR:
          Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent& event) {
            wxComboBox* browse = (wxComboBox*)item.GetWindow();
            wxDirDialog dir_dlg(
              this,
              _(wxDirSelectorPromptStr),
              browse->GetValue(),
              wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
            if (dir_dlg.ShowModal() == wxID_OK)
            {
              const wxString value = dir_dlg.GetPath();
              const int item = browse->FindString(value);
              browse->SetSelection(item == wxNOT_FOUND ? browse->Append(value): item);
            }}, item.GetWindow()->GetId());
          break;
        case ITEM_TOGGLEBUTTON:
          Bind(wxEVT_TOGGLEBUTTON, [=](wxCommandEvent& event) {
            Click(event);}, item.GetWindow()->GetId());
          break;
      }
    }
    AddUserSizer(sizer);
    LayoutSizers();
  };

  std::vector< T > m_Items;
  bool m_ForceCheckBoxChecked;
  wxString m_Contains;
  wxString m_Page;
};
#endif // wxUSE_GUI
