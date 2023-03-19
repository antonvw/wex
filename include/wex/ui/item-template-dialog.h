////////////////////////////////////////////////////////////////////////////////
// Name:      item-template-dialog.h
// Purpose:   Declaration of wex::item_template_dialog class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/core/log.h>
#include <wex/core/path.h>
#include <wex/factory/frame.h>
#include <wex/ui/dialog.h>
#include <wex/ui/item.h>
#include <wx/app.h>
#include <wx/checkbox.h>
#include <wx/checklst.h>
#include <wx/combobox.h>
#include <wx/filepicker.h>
#include <wx/textctrl.h>
#include <wx/tglbtn.h> // for wxEVT_TOGGLEBUTTON

#define DO_DIALOG                                            \
  {                                                          \
    /* NOLINTNEXTLINE */                                     \
    if (dlg.ShowModal() == wxID_OK)                          \
    {                                                        \
      const auto value = dlg.GetPath();                      \
      const int  item  = browse->FindString(value);          \
      browse->SetSelection(                                  \
        item == wxNOT_FOUND ? browse->Append(value) : item); \
    }                                                        \
  }

#define PICKER_HANDLE(COMPONENT)                              \
  if (auto* pc = reinterpret_cast<COMPONENT*>(item.window()); \
      item.data().control().is_required())                    \
  {                                                           \
    if (pc->GetPath().empty())                                \
    {                                                         \
      event.Enable(false);                                    \
      return true;                                            \
    }                                                         \
  }                                                           \
  return false;

#include <vector>

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
template <class T> class item_template_dialog : public dialog
{
public:
  /// Constructor.
  item_template_dialog(
    /// vector with items
    const std::vector<T>& v,
    /// data
    const data::window& data = wex::data::window(),
    /// number of rows (if 0 add similar items on next row)
    int rows = 0,
    /// number of columns
    int cols = 1);

  /// Adds an item to the temp vector.
  void add(const T& item) { m_items_tmp.emplace_back(item); }

  /// If this item is related to a button, bind to event handler.
  /// Returns true if bind was done.
  bool bind_button(const T& item);

  /// Returns the (first) item that has specified label,
  /// or empty item if item does not exist.
  const T find(const std::string& label) const;

  /// If you specified some checkboxes, calling this method
  /// requires that one of them should be checked for the OK button
  /// to be enabled.
  void force_checkbox_checked(
    /// specify the (part of) the name of the checkbox
    const std::string& contains = std::string(),
    /// specify on which page
    const std::string& page = std::string());

  /// Returns all items.
  const auto& get_items() const { return m_items; }

  /// Returns the item actual value for specified label, or
  /// empty object if item does not exist.
  const auto get_item_value(const std::string& label) const
  {
    return find(label).get_value();
  };

  /// Sets the item actual value for specified label.
  bool set_item_value(const std::string& label, const std::any& value) const;

private:
  void click(const wxCommandEvent& event) const;
  void layout(int rows, int cols);
  void on_command(wxCommandEvent& event);
  void on_update_ui(wxUpdateUIEvent& event);
  void process_checkbox(const T& item);
  void process_checklistbox(const T& item);
  bool process_combobox(const T& item, wxUpdateUIEvent& event);
  bool process_dirpickerctrl(const T& item, wxUpdateUIEvent& event);
  bool process_filepickerctrl(const T& item, wxUpdateUIEvent& event);
  bool process_textctrl(const T& item, wxUpdateUIEvent& event);

  std::vector<T> m_items, m_items_tmp;

  bool m_force_checkbox_checked{false}, m_one_checkbox_checked{false};

  wxString m_contains, m_page;
};

// implementation

template <class T>
wex::item_template_dialog<T>::item_template_dialog(
  const std::vector<T>& v,
  const data::window&   data,
  int                   rows,
  int                   cols)
  : dialog(data)
  , m_items(v)
{
  T::set_dialog(this);
  layout(rows, cols);

  Bind(wxEVT_BUTTON, &item_template_dialog::on_command, this, wxID_APPLY);
  Bind(wxEVT_BUTTON, &item_template_dialog::on_command, this, wxID_CANCEL);
  Bind(wxEVT_BUTTON, &item_template_dialog::on_command, this, wxID_CLOSE);
  Bind(wxEVT_BUTTON, &item_template_dialog::on_command, this, wxID_OK);
  Bind(wxEVT_UPDATE_UI, &item_template_dialog::on_update_ui, this, wxID_APPLY);
  Bind(wxEVT_UPDATE_UI, &item_template_dialog::on_update_ui, this, wxID_OK);
};

template <class T> bool wex::item_template_dialog<T>::bind_button(const T& item)
{
  if (item.window() == nullptr)
    return false;

  switch (item.type())
  {
    case item::BUTTON:
    case item::COMMANDLINKBUTTON:
      Bind(
        wxEVT_BUTTON,
        [&, this](const wxCommandEvent& event)
        {
          if (!item.apply())
            click(event);
        },
        item.window()->GetId());
      break;

    case item::COMBOBOX_DIR:
      Bind(
        wxEVT_BUTTON,
        [&, this](const wxCommandEvent& event)
        {
          if (auto* browse = reinterpret_cast<wxComboBox*>(item.window());
              browse == nullptr)
          {
            log("browse dir failed") << item.label();
          }
          else
          {
            wxDirDialog dlg(
              this,
              _(wxDirSelectorPromptStr),
              browse->GetValue(),
              wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
            DO_DIALOG;
          }
        },
        item.window()->GetId());
      break;

    case item::COMBOBOX_FILE:
      Bind(
        wxEVT_BUTTON,
        [&, this](const wxCommandEvent& event)
        {
          if (auto* browse = reinterpret_cast<wxComboBox*>(item.window());
              browse != nullptr)
          {
            const path   path(browse->GetValue());
            wxFileDialog dlg(
              this,
              _(wxFileSelectorPromptStr),
              path.parent_path(),
              path.filename(),
              wxFileSelectorDefaultWildcardStr,
              wxFD_DEFAULT_STYLE | wxFD_FILE_MUST_EXIST);
            DO_DIALOG;
          }
          else
          {
            log("browse file failed") << item.label();
          }
        },
        item.window()->GetId());
      break;

    case item::TOGGLEBUTTON:
      Bind(
        wxEVT_TOGGLEBUTTON,
        [&, this](const wxCommandEvent& event)
        {
          if (!item.apply())
            click(event);
        },
        item.window()->GetId());
      break;

    default:
      return false;
  }

  return true;
};

template <class T>
void wex::item_template_dialog<T>::click(const wxCommandEvent& event) const
{
  if (auto* frame =
        dynamic_cast<wex::factory::frame*>(wxTheApp->GetTopWindow());
      frame != nullptr)
  {
    frame->on_command_item_dialog(GetId(), event);
  }
};

template <class T>
const T wex::item_template_dialog<T>::find(const std::string& label) const
{
  if (const auto& it = std::find_if(
        m_items.begin(),
        m_items.end(),
        [label](const auto& p)
        {
          return label == p.label();
        });
      it != m_items.end())
  {
    return *it;
  }

  return T();
};

template <class T>
void wex::item_template_dialog<T>::force_checkbox_checked(
  const std::string& contains,
  const std::string& page)
{
  m_force_checkbox_checked = true;
  m_contains               = contains;
  m_page                   = page;
};

template <class T> void wex::item_template_dialog<T>::layout(int rows, int cols)
{
  wxFlexGridSizer* previous_item_sizer = nullptr;
  wxFlexGridSizer* sizer =
    (rows > 0 ? new wxFlexGridSizer(rows, cols, 0, 0) :
                new wxFlexGridSizer(cols));

  for (int i = 0; i < cols; i++)
  {
    sizer->AddGrowableCol(i);
  }

  int previous_type = -1;

  for (auto& item : m_items)
  {
    if (item.empty())
      continue;

    // If this item has same type as previous type use previous sizer,
    // otherwise use no sizer (layout will create a new one).
    wxFlexGridSizer* current_item_sizer =
      (item.type() == previous_type && cols == 1 ? previous_item_sizer :
                                                   nullptr);

    // layout the item.
    previous_item_sizer =
      item.layout(this, sizer, data().button() == wxCANCEL, current_item_sizer);
    previous_type = item.type();

    if (
      sizer->GetEffectiveRowsCount() >= 1 &&
      !sizer->IsRowGrowable(sizer->GetEffectiveRowsCount() - 1) &&
      item.is_row_growable())
    {
      sizer->AddGrowableRow(sizer->GetEffectiveRowsCount() - 1);
    }
    bind_button(item);
  }

  add_user_sizer(sizer);
  layout_sizers();

  m_items.insert(m_items.end(), m_items_tmp.begin(), m_items_tmp.end());
  m_items_tmp.clear();
};

template <class T>
void wex::item_template_dialog<T>::on_command(wxCommandEvent& event)
{
  if (
    event.GetId() == wxID_APPLY ||
    ((event.GetId() == wxID_OK || event.GetId() == wxID_CANCEL) && !IsModal()))
  {
    click(event);
  }

  event.Skip();
};

template <class T>
void wex::item_template_dialog<T>::on_update_ui(wxUpdateUIEvent& event)
{
  m_one_checkbox_checked = false;

  for (const auto& item : m_items)
  {
    switch (item.type())
    {
      case item::CHECKBOX:
        process_checkbox(item);
        break;

      case item::CHECKLISTBOX_BOOL:
        process_checklistbox(item);
        break;

      case item::COMBOBOX:
      case item::COMBOBOX_DIR:
      case item::COMBOBOX_FILE:
        if (process_combobox(item, event))
        {
          return;
        }
        break;

      case item::TEXTCTRL:
      case item::TEXTCTRL_FLOAT:
      case item::TEXTCTRL_INT:
        if (process_textctrl(item, event))
        {
          return;
        }
        break;

      case item::DIRPICKERCTRL:
        if (process_dirpickerctrl(item, event))
        {
          return;
        }
        break;

      case item::FILEPICKERCTRL:
        if (process_filepickerctrl(item, event))
        {
          return;
        }
        break;

      default:; // do nothing
    }
  }

  event.Enable(m_force_checkbox_checked ? m_one_checkbox_checked : true);
};

template <class T>
void wex::item_template_dialog<T>::process_checkbox(const T& item)
{
  if (m_force_checkbox_checked)
  {
    if (auto* cb = reinterpret_cast<wxCheckBox*>(item.window());
        wxString(item.label()).Lower().Contains(m_contains.Lower()) &&
        cb->IsChecked() && item.page() == m_page)
    {
      m_one_checkbox_checked = true;
    }
  }
}

template <class T>
void wex::item_template_dialog<T>::process_checklistbox(const T& item)
{
  if (m_force_checkbox_checked)
  {
    auto* clb = reinterpret_cast<wxCheckListBox*>(item.window());
    for (size_t i = 0; i < clb->GetCount(); i++)
    {
      if (
        clb->GetString(i).Lower().Contains(m_contains.Lower()) &&
        clb->IsChecked(i) && item.page() == m_page)
      {
        m_one_checkbox_checked = true;
      }
    }
  }
}

template <class T>
bool wex::item_template_dialog<T>::process_combobox(
  const T&         item,
  wxUpdateUIEvent& event)
{
  if (auto* cb = reinterpret_cast<wxComboBox*>(item.window()); cb != nullptr)
  {
    if (
      !item.validate() ||
      (!item.data().validate_re().empty() &&
       !item.validate(item.data().validate_re())) ||
      (item.data().control().is_required() && cb->GetValue().empty()))
    {
      event.Enable(false);
      return true;
    }
  }

  return false;
}

template <class T>
bool wex::item_template_dialog<T>::process_dirpickerctrl(
  const T&         item,
  wxUpdateUIEvent& event)
{
  PICKER_HANDLE(wxDirPickerCtrl)
}

template <class T>
bool wex::item_template_dialog<T>::process_filepickerctrl(
  const T&         item,
  wxUpdateUIEvent& event)
{
  PICKER_HANDLE(wxFilePickerCtrl)
}

template <class T>
bool wex::item_template_dialog<T>::process_textctrl(
  const T&         item,
  wxUpdateUIEvent& event)
{
  if (auto* tc = reinterpret_cast<wxTextCtrl*>(item.window()); tc != nullptr)
  {
    if (
      !item.validate() ||
      (!item.data().validate_re().empty() &&
       !item.validate(item.data().validate_re())) ||
      (item.data().control().is_required() && tc->GetValue().empty()))
    {
      event.Enable(false);
      return true;
    }
  }

  return false;
}

template <class T>
bool wex::item_template_dialog<T>::set_item_value(
  const std::string& label,
  const std::any&    value) const
{
  for (auto& item : m_items)
  {
    if (item.label() == label)
    {
      return item.set_value(value);
    }
  };

  return false;
};
}; // namespace wex
