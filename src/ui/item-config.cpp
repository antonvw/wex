////////////////////////////////////////////////////////////////////////////////
// Name:      config_item.cpp
// Purpose:   Implementation of wex::item class config methods
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/common/tostring.h>
#include <wex/common/util.h>
#include <wex/core/config.h>
#include <wex/ui/frd.h>
#include <wx/settings.h>

#ifndef __WXMSW__
#define BOOST_PROCESS_V2_HEADER_ONLY ON
#include <boost/process/v2.hpp>
#endif

#define PERSISTENT(TYPE, DEFAULT)                                              \
  {                                                                            \
    if (save)                                                                  \
      config(m_label).set(std::any_cast<TYPE>(get_value()));                   \
    else                                                                       \
      set_value((TYPE)config(m_label).get(DEFAULT));                           \
  }

#define PERSISTENT_FOR(ITEM_TYPE, ITEM_CAST, METHOD)                           \
  case ITEM_TYPE:                                                              \
    PERSISTENT(ITEM_CAST, METHOD);                                             \
    break;

#include <filesystem>

#include "item.h"

namespace wex
{
bool persistent_checkbox_frd(wxCheckListBox* clb, int item, bool save)
{
  auto* frd = find_replace_data::get();

  if (const std::string field(clb->GetString(item));
      field == frd->text_match_word())
  {
    !save ? clb->Check(item, frd->match_word()) :
            frd->set_match_word(clb->IsChecked(item));
  }
  else if (field == frd->text_match_case())
  {
    !save ? clb->Check(item, frd->match_case()) :
            frd->set_match_case(clb->IsChecked(item));
  }
  else if (field == frd->text_regex())
  {
    !save ? clb->Check(item, frd->is_regex()) :
            frd->set_regex(clb->IsChecked(item));
  }
  else if (field == frd->text_search_down())
  {
    !save ? clb->Check(item, frd->search_down()) :
            frd->set_search_down(clb->IsChecked(item));
  }
  else
  {
    return false;
  }

  return true;
}

void persistent_checkbox(const wex::item* item, bool save)
{
  if (auto* clb = reinterpret_cast<wxCheckListBox*>(item->window());
      clb != nullptr)
  {
    for (size_t i = 0; const auto& c : std::any_cast<item::choices_bool_t>(
                         item->data().initial()))
    {
      if (!persistent_checkbox_frd(clb, i, save))
      {
        if (save)
        {
          config(find_before(c, ",")).set(clb->IsChecked(i));
        }
        else
        {
          clb->Check(i, config(find_before(c, ",")).get(clb->IsChecked(i)));
        }
      }

      i++;
    }
  }
}

void persistent_combobox(const wex::item* item, bool save)
{
  const int max_items{25};

  if (auto* cb = reinterpret_cast<wxComboBox*>(item->window()); save)
  {
    if (const auto l = to_list_string(cb, max_items).get();
        item->label() == find_replace_data::get()->text_find())
    {
      find_replace_data::get()->set_find_strings(l);
    }
    else if (item->label() == find_replace_data::get()->text_replace_with())
    {
      find_replace_data::get()->set_replace_strings(l);
    }
    else
    {
      config(item->label()).set(l);
    }
  }
  else
  {
    combobox_from_list(
      cb,
      config(item->label())
        .get(
          !item->data().initial().has_value() ?
            config::strings_t{} :
            std::any_cast<config::strings_t>(item->data().initial())));
  }
}

void persistent_filepicker(const wex::item* item, bool save)
{
  if (save)
  {
    config(item->label()).set(std::any_cast<std::string>(item->get_value()));
  }
  else
  {
#ifndef __WXMSW__
    if (const auto initial = boost::process::v2::environment::find_executable(
          item->label_window());
        !initial.empty())
    {
      item->set_value(config(item->label()).get(initial.string()));
    }
#else
    item->set_value(config(item->label()).get(std::string()));
#endif
  }
}

void persistent_radiobox(const wex::item* item, bool save)
{
  if (auto* rb = reinterpret_cast<wxRadioBox*>(item->window()); save)
  {
    for (const auto& b : std::any_cast<item::choices_t>(item->data().initial()))
    {
      if (find_before(b.second, ",") == rb->GetStringSelection())
      {
        config(item->label()).set(b.first);
      }
    }
  }
  else
  {
    const auto& choices(std::any_cast<item::choices_t>(item->data().initial()));

    if (const auto c =
          choices.find(config(item->label()).get(rb->GetSelection()));
        c != choices.end())
    {
      rb->SetStringSelection(find_before(c->second, ","));
    }
  }
}

} // namespace wex

bool wex::item::persist(bool save) const
{
  switch (m_type)
  {
    PERSISTENT_FOR(
      CHECKBOX,
      bool,
      (reinterpret_cast<wxCheckBox*>(m_window))->GetValue());
    PERSISTENT_FOR(CHECKLISTBOX_BIT, long, std::any_cast<long>(get_value()));
    PERSISTENT_FOR(
      COLOURPICKERWIDGET,
      wxColour,
      (reinterpret_cast<wxColourPickerWidget*>(m_window))->GetColour());
    PERSISTENT_FOR(DIRPICKERCTRL, std::string, m_label);
    PERSISTENT_FOR(
      FONTPICKERCTRL,
      wxFont,
      wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));
    PERSISTENT_FOR(
      SLIDER,
      int,
      (reinterpret_cast<wxSlider*>(m_window))->GetValue());
    PERSISTENT_FOR(
      SPINCTRL,
      int,
      (reinterpret_cast<wxSpinCtrl*>(m_window))->GetValue());
    PERSISTENT_FOR(
      SPINCTRLDOUBLE,
      double,
      (reinterpret_cast<wxSpinCtrlDouble*>(m_window))->GetValue());
    PERSISTENT_FOR(
      TEXTCTRL,
      std::string,
      std::any_cast<std::string>(get_value()));
    PERSISTENT_FOR(TEXTCTRL_FLOAT, double, std::any_cast<double>(get_value()));
    PERSISTENT_FOR(TEXTCTRL_INT, long, std::any_cast<long>(get_value()));
    PERSISTENT_FOR(TOGGLEBUTTON, bool, false);

    default:
      return false;
  }

  return true;
}

bool wex::item::to_config(bool save) const
{
  if (!m_use_config || m_window == nullptr)
  {
    return false;
  }

  if (!persist(save))
  {
    switch (m_type)
    {
      case CHECKLISTBOX_BOOL:
        persistent_checkbox(this, save);
        break;

      case COMBOBOX:
      case COMBOBOX_DIR:
      case COMBOBOX_FILE:
        persistent_combobox(this, save);
        break;

      case FILEPICKERCTRL:
        persistent_filepicker(this, save);
        break;

      case GRID:
        if (save)
        {
          config(m_label).set(std::any_cast<std::string>(get_value()));
        }
        else
        {
          set_value(config(m_label).get());
        }
        break;

      case LISTVIEW:
        if (save)
        {
          config(m_label).set(std::any_cast<config::strings_t>(get_value()));
        }
        else
        {
          set_value(config(m_label).get(
            !m_data.initial().has_value() ?
              config::strings_t{} :
              std::any_cast<config::strings_t>(m_data.initial())));
        }
        break;

      case RADIOBOX:
        persistent_radiobox(this, save);
        break;

      case USER:
        if (
          m_data.user_window_to_config() != nullptr &&
          !(m_data.user_window_to_config())(m_window, save))
        {
          return false;
        }
        break;

      default:
        // the other types (including STC) have no persistent info
        return false;
    }
  }

  if (m_data.apply() != nullptr)
  {
    (m_data.apply())(m_window, get_value(), save);
  }

  return true;
}
