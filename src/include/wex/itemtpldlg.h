////////////////////////////////////////////////////////////////////////////////
// Name:      itemtpldlg.h
// Purpose:   Declaration of wex::item_template_dialog class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <vector>
#include <wx/app.h>
#include <wx/checkbox.h>
#include <wx/checklst.h>
#include <wx/combobox.h>
#include <wx/filepicker.h>
#include <wx/tglbtn.h> // for wxEVT_TOGGLEBUTTON
#include <wx/textctrl.h>
#include <wex/dialog.h>
#include <wex/frame.h>
#include <wex/item.h> 
#include <wex/path.h>

namespace wex
{
  /// Offers a dialog template to set several items.
  /// If you only specify a wxCANCEL button, the dialog is readonly.
  /// When pressing the:
  /// - wxAPPLY button
  /// - wxOK, wxCANCEL button for a modeless dialog
  /// - a item::BUTTON
  /// - a item::COMMANDLINKBUTTON
  /// - a item::TOGGLEBUTTON
  /// the method frame::on_command_item_dialog is invoked.
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
    , m_force_checkbox_checked(false)
    , m_Items(v) {
      layout(rows, cols);
      Bind(wxEVT_BUTTON, &item_template_dialog::OnCommand, this, wxID_APPLY);
      Bind(wxEVT_BUTTON, &item_template_dialog::OnCommand, this, wxID_CANCEL);
      Bind(wxEVT_BUTTON, &item_template_dialog::OnCommand, this, wxID_CLOSE);
      Bind(wxEVT_BUTTON, &item_template_dialog::OnCommand, this, wxID_OK);
      Bind(wxEVT_UPDATE_UI, &item_template_dialog::OnUpdateUI, this, wxID_APPLY);
      Bind(wxEVT_UPDATE_UI, &item_template_dialog::OnUpdateUI, this, wxID_OK);};
    
    /// Adds an item to the temp vector.
    void add(const T & item) {m_ItemsTemp.emplace_back(item);};

    /// If this item is related to a button, bind to event handler.
    /// Returns true if bind was done.
    bool bind_button(const T & item) {
      if (item.window() == nullptr) return false;
      switch (item.type())
      {
        case item::BUTTON:
        case item::COMMANDLINKBUTTON:
          Bind(wxEVT_BUTTON, [=](wxCommandEvent& event) {
            if (!item.apply()) Click(event);}, item.window()->GetId());
          break;
        case item::COMBOBOX_DIR:
          Bind(wxEVT_BUTTON, [=](wxCommandEvent& event) {
            wxComboBox* browse = (wxComboBox*)item.window();
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
            }}, item.window()->GetId());
          break;
        case item::COMBOBOX_FILE:
          Bind(wxEVT_BUTTON, [=](wxCommandEvent& event) {
            wxComboBox* browse = (wxComboBox*)item.window();
            const path path(browse->GetValue());
            wxFileDialog dlg(
              this,
              _(wxFileSelectorPromptStr),
              path.get_path(),
              path.fullname(),
              wxFileSelectorDefaultWildcardStr,
              wxFD_DEFAULT_STYLE | wxFD_FILE_MUST_EXIST);
            if (dlg.ShowModal() == wxID_OK)
            {
              const wxString value = dlg.GetPath();
              const int item = browse->FindString(value);
              browse->SetSelection(item == wxNOT_FOUND ? browse->Append(value): item);
            }}, item.window()->GetId());
          break;
        case item::TOGGLEBUTTON:
          Bind(wxEVT_TOGGLEBUTTON, [=](wxCommandEvent& event) {
            if (!item.apply()) Click(event);}, item.window()->GetId());
          break;
        default: return false;
      }
      return true;};

    /// If you specified some checkboxes, calling this method
    /// requires that one of them should be checked for the OK button
    /// to be enabled.
    void force_checkbox_checked(
      /// specify the (part of) the name of the checkbox
      const wxString& contains = wxEmptyString,
      /// specify on which page
      const wxString& page = wxEmptyString) {
      m_force_checkbox_checked = true;
      m_Contains = contains;
      m_Page = page;};
    
    /// Returns the (first) item that has specified label,
    /// or empty item if item does not exist.
    const T get_item(const std::string& label) const {
      for (const auto& item : m_Items)
      {
        if (item.label() == label)
        {
          return item;
        }
      };
      return T();};

    /// Returns all items.
    const auto & get_items() const {return m_Items;};

    /// Returns the item actual value for specified label, or 
    /// empty object if item does not exist.
    const auto get_item_value(const std::string& label) const {
      return get_item(label).get_value();};
   
    /// Sets the item actual value for specified label.
    bool set_item_value(const std::string& label, const std::any& value) const {
      for (auto& item : m_Items)
      {
        if (item.label() == label)
        {
          return item.set_value(value);
        }
      };
      return false;};
  private:
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
        switch (item.type())
        {
        case item::CHECKBOX:
          if (m_force_checkbox_checked)
          {
            if (auto* cb = (wxCheckBox*)item.window(); 
              wxString(item.label()).Lower().Contains(m_Contains.Lower()) && 
                cb->IsChecked() &&
                item.page() == m_Page)
            {
              one_checkbox_checked = true;
            }
          }
          break;

        case item::CHECKLISTBOX_BOOL:
          if (m_force_checkbox_checked)
          {
            auto* clb = (wxCheckListBox*)item.window();
            for (
              size_t i = 0;
              i < clb->GetCount();
              i++)
            {
              if (clb->GetString(i).Lower().Contains(m_Contains.Lower()) && 
                  clb->IsChecked(i) &&
                  item.page() == m_Page)
              {
                one_checkbox_checked = true;
              }
            }
          }
          break;

        case item::COMBOBOX:
        case item::COMBOBOX_DIR:
        case item::COMBOBOX_FILE:
          if (wxComboBox* cb = (wxComboBox*)item.window(); item.data().is_required())
          {
            if (cb->GetValue().empty())
            {
              event.Enable(false);
              return;
            }
          }
          break;

        case item::TEXTCTRL_INT:
        case item::TEXTCTRL:
          if (wxTextCtrl* tc = (wxTextCtrl*)item.window(); item.data().is_required())
          {
            if (tc->GetValue().empty())
            {
              event.Enable(false);
              return;
            }
          }
          break;

        case item::DIRPICKERCTRL:
          if (wxDirPickerCtrl* pc = (wxDirPickerCtrl*)item.window(); 
            item.data().is_required())
          {
            if (pc->GetPath().empty())
            {
              event.Enable(false);
              return;
            }
          }
          break;

        case item::FILEPICKERCTRL:
          if (wxFilePickerCtrl* pc = (wxFilePickerCtrl*)item.window(); 
            item.data().is_required())
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
      event.Enable(m_force_checkbox_checked ? one_checkbox_checked: true);};

    void Click(const wxCommandEvent& event) const {
      if (frame* frame = wxDynamicCast(wxTheApp->GetTopWindow(), wex::frame);
        frame != nullptr)
      {
        frame->on_command_item_dialog(GetId(), event);
      }};
    
    void layout(int rows, int cols) {
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
        if (item.type() == item::EMPTY) continue; //skip

        item.set_dialog(this);
        
        // If this item has same type as previous type use previous sizer,
        // otherwise use no sizer (layout will create a new one).
        wxFlexGridSizer* current_item_sizer = (item.type() == previous_type && cols == 1 ? 
          previous_item_sizer: 
          nullptr);

        // layout the item.
        previous_item_sizer = item.layout(
          this, 
          sizer, 
          data().button() == wxCANCEL,
          current_item_sizer);
        previous_type = item.type();
        
        if (sizer->GetEffectiveRowsCount() >= 1 &&
           !sizer->IsRowGrowable(sizer->GetEffectiveRowsCount() - 1) &&
            item.is_row_growable())
        {
          sizer->AddGrowableRow(sizer->GetEffectiveRowsCount() - 1);
        }
        bind_button(item);
      }
      add_user_sizer(sizer);
      layout_sizers();
      m_Items.insert(m_Items.end(), m_ItemsTemp.begin(), m_ItemsTemp.end());
      m_ItemsTemp.clear();
      };

    std::vector< T > m_Items;
    std::vector< T > m_ItemsTemp;
    bool m_force_checkbox_checked;
    wxString m_Contains;
    wxString m_Page;
  };
};
