////////////////////////////////////////////////////////////////////////////////
// Name:      itemtpldlg.h
// Purpose:   Declaration of wex::item_template_dialog class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <vector>
#include <wx/app.h>
#include <wx/checkbox.h>
#include <wx/checklst.h>
#include <wx/combobox.h>
#include <wx/filepicker.h>
#include <wx/tglbtn.h> // for wxEVT_TOGGLEBUTTON
#include <wx/extension/dialog.h>
#include <wx/extension/frame.h>
#include <wx/extension/item.h> 
#include <wx/extension/path.h>

namespace wex
{
  /// Offers a dialog template to set several items.
  /// If you only specify a wxCANCEL button, the dialog is readonly.
  /// When pressing the:
  /// - wxAPPLY button
  /// - wxOK, wxCANCEL button for a modeless dialog
  /// - a ITEM_BUTTON
  /// - a ITEM_COMMANDLINKBUTTON
  /// - a ITEM_TOGGLEBUTTON
  /// the method frame::OnCommandItemDialog is invoked.
  template <class T> class item_template_dialog: public dialog
  {
  public:
    /// Constructor.
    item_template_dialog(
      /// vector with items 
      const std::vector< T >& v,
      /// data
      const window_data& data = wex::window_data(),
      /// number of rows (if 0 add similar items on next row)
      int rows = 0,
      /// number of columns
      int cols = 1)
    : dialog(data)
    , m_ForceCheckBoxChecked(false)
    , m_Items(v) {
      Layout(rows, cols);
      Bind(wxEVT_BUTTON, &item_template_dialog::OnCommand, this, wxID_APPLY);
      Bind(wxEVT_BUTTON, &item_template_dialog::OnCommand, this, wxID_CANCEL);
      Bind(wxEVT_BUTTON, &item_template_dialog::OnCommand, this, wxID_CLOSE);
      Bind(wxEVT_BUTTON, &item_template_dialog::OnCommand, this, wxID_OK);
      Bind(wxEVT_UPDATE_UI, &item_template_dialog::OnUpdateUI, this, wxID_APPLY);
      Bind(wxEVT_UPDATE_UI, &item_template_dialog::OnUpdateUI, this, wxID_OK);};
    
    /// Adds an item to the temp vector.
    void Add(const T & item) {m_ItemsTemp.emplace_back(item);};

    /// If this item is related to a button, bind to event handler.
    /// Returns true if bind was done.
    bool BindButton(const T & item) {
      if (item.GetWindow() == nullptr) return false;
      switch (item.GetType())
      {
        case ITEM_BUTTON:
        case ITEM_COMMANDLINKBUTTON:
          Bind(wxEVT_BUTTON, [=](wxCommandEvent& event) {
            if (!item.Apply()) Click(event);}, item.GetWindow()->GetId());
          break;
        case ITEM_COMBOBOX_DIR:
          Bind(wxEVT_BUTTON, [=](wxCommandEvent& event) {
            wxComboBox* browse = (wxComboBox*)item.GetWindow();
            wxDirDialog dlg(
              this,
              _(wxDirSelectorPromptStr),
              browse->GetValue(),
              wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
            if (dlg.ShowModal() == wxID_OK)
            {
              const wxString value = dlg.GetPath();
              const int item = browse->FindString(value);
              browse->SetSelection(item == wxNOT_FOUND ? browse->Append(value): item);
            }}, item.GetWindow()->GetId());
          break;
        case ITEM_COMBOBOX_FILE:
          Bind(wxEVT_BUTTON, [=](wxCommandEvent& event) {
            wxComboBox* browse = (wxComboBox*)item.GetWindow();
            const path path(browse->GetValue());
            wxFileDialog dlg(
              this,
              _(wxFileSelectorPromptStr),
              path.GetPath(),
              path.GetFullName(),
              wxFileSelectorDefaultWildcardStr,
              wxFD_DEFAULT_STYLE | wxFD_FILE_MUST_EXIST);
            if (dlg.ShowModal() == wxID_OK)
            {
              const wxString value = dlg.GetPath();
              const int item = browse->FindString(value);
              browse->SetSelection(item == wxNOT_FOUND ? browse->Append(value): item);
            }}, item.GetWindow()->GetId());
          break;
        case ITEM_TOGGLEBUTTON:
          Bind(wxEVT_TOGGLEBUTTON, [=](wxCommandEvent& event) {
            if (!item.Apply()) Click(event);}, item.GetWindow()->GetId());
          break;
        default: return false;
      }
      return true;};

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
    
    /// Returns the (first) item that has specified label,
    /// or empty item if item does not exist.
    const T GetItem(const std::string& label) const {
      for (const auto& item : m_Items)
      {
        if (item.GetLabel() == label)
        {
          return item;
        }
      };
      return T();};

    /// Returns all items.
    const auto & GetItems() const {return m_Items;};

    /// Returns the item actual value for specified label, or 
    /// empty object if item does not exist.
    const auto GetItemValue(const std::string& label) const {
      return GetItem(label).GetValue();};
   
    /// Sets the item actual value for specified label.
    bool SetItemValue(const std::string& label, const std::any& value) const {
      for (auto& item : m_Items)
      {
        if (item.GetLabel() == label)
        {
          return item.SetValue(value);
        }
      };
      return false;};
  protected:
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
            if (auto* cb = (wxCheckBox*)item.GetWindow(); 
              wxString(item.GetLabel()).Lower().Contains(m_Contains.Lower()) && 
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
            auto* clb = (wxCheckListBox*)item.GetWindow();
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
        case ITEM_COMBOBOX_DIR:
        case ITEM_COMBOBOX_FILE:
          if (wxComboBox* cb = (wxComboBox*)item.GetWindow(); item.GetData().Required())
          {
            if (cb->GetValue().empty())
            {
              event.Enable(false);
              return;
            }
          }
          break;

        case ITEM_TEXTCTRL_INT:
        case ITEM_TEXTCTRL:
          if (wxTextCtrl* tc = (wxTextCtrl*)item.GetWindow(); item.GetData().Required())
          {
            if (tc->GetValue().empty())
            {
              event.Enable(false);
              return;
            }
          }
          break;

        case ITEM_DIRPICKERCTRL:
          if (wxDirPickerCtrl* pc = (wxDirPickerCtrl*)item.GetWindow(); 
            item.GetData().Required())
          {
            if (pc->GetPath().empty())
            {
              event.Enable(false);
              return;
            }
          }
          break;

        case ITEM_FILEPICKERCTRL:
          if (wxFilePickerCtrl* pc = (wxFilePickerCtrl*)item.GetWindow(); 
            item.GetData().Required())
          {
            if (pc->GetPath().empty())
            {
              event.Enable(false);
              return;
            }
          }
          break;

        default: ; // do nothing
        }
      }
      event.Enable(m_ForceCheckBoxChecked ? one_checkbox_checked: true);};
  private:
    void Click(const wxCommandEvent& event) const {
      if (frame* frame = wxDynamicCast(wxTheApp->GetTopWindow(), wex::frame);
        frame != nullptr)
      {
        frame->OnCommandItemDialog(GetId(), event);
      }};
    
    void Layout(int rows, int cols) {
      wxFlexGridSizer* previous_item_sizer = nullptr;
      wxFlexGridSizer* sizer = (rows > 0 ? 
        new wxFlexGridSizer(rows, cols, 0, 0): 
        new wxFlexGridSizer(cols));

      for (int i = 0; i < cols; i++)
      {
        sizer->AddGrowableCol(i);
      }
      int previous_type = -1;
      for (auto& item : m_Items)
      {
        if (item.GetType() == ITEM_EMPTY) continue; //skip

        item.SetDialog(this);
        
        // If this item has same type as previous type use previous sizer,
        // otherwise use no sizer (Layout will create a new one).
        wxFlexGridSizer* current_item_sizer = (item.GetType() == previous_type && cols == 1 ? 
          previous_item_sizer: 
          nullptr);

        // Layout the item.
        previous_item_sizer = item.Layout(
          this, 
          sizer, 
          GetData().Button() == wxCANCEL,
          current_item_sizer);
        previous_type = item.GetType();
        
        if (sizer->GetEffectiveRowsCount() >= 1 &&
           !sizer->IsRowGrowable(sizer->GetEffectiveRowsCount() - 1) &&
            item.IsRowGrowable())
        {
          sizer->AddGrowableRow(sizer->GetEffectiveRowsCount() - 1);
        }
        BindButton(item);
      }
      AddUserSizer(sizer);
      LayoutSizers();
      m_Items.insert(m_Items.end(), m_ItemsTemp.begin(), m_ItemsTemp.end());
      m_ItemsTemp.clear();
      };

    std::vector< T > m_Items;
    std::vector< T > m_ItemsTemp;
    bool m_ForceCheckBoxChecked;
    wxString m_Contains;
    wxString m_Page;
  };
};
