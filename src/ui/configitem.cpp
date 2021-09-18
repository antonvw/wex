////////////////////////////////////////////////////////////////////////////////
// Name:      config_item.cpp
// Purpose:   Implementation of wex::item class config methods
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/config.h>
#include <wex/core.h>
#include <wex/frd.h>
#include <wex/item.h>
#include <wex/tostring.h>
#include <wex/util.h>
#include <wx/checkbox.h>
#include <wx/checklst.h>
#include <wx/clrpicker.h>
#include <wx/radiobox.h>
#include <wx/settings.h>
#include <wx/spinctrl.h>
#include <wx/window.h>

#define PERSISTENT(TYPE, DEFAULT)                            \
  {                                                          \
    if (save)                                                \
      config(m_label).set(std::any_cast<TYPE>(get_value())); \
    else                                                     \
      set_value((TYPE)config(m_label).get(DEFAULT));         \
  }

import<filesystem>;

namespace wex
{
bool update(wxCheckListBox* clb, int item, bool save)
{
  find_replace_data* frd = find_replace_data::get();

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
} // namespace wex

bool wex::item::to_config(bool save) const
{
  if (!m_use_config || m_window == nullptr)
  {
    return false;
  }

  switch (type())
  {
    case CHECKBOX:
      PERSISTENT(bool, (reinterpret_cast<wxCheckBox*>(m_window))->GetValue());
      break;

    case CHECKLISTBOX_BIT:
      PERSISTENT(long, std::any_cast<long>(get_value()));
      break;

    case CHECKLISTBOX_BOOL:
      if (auto* clb = reinterpret_cast<wxCheckListBox*>(m_window);
          clb != nullptr)
      {
        size_t i = 0;

        for (const auto& c : std::any_cast<choices_bool_t>(m_data.initial()))
        {
          if (!update(clb, i, save))
          {
            if (save)
              config(before(c, ',')).set(clb->IsChecked(i));
            else
              clb->Check(i, config(before(c, ',')).get(clb->IsChecked(i)));
          }

          i++;
        }
      }
      break;

    case COLOURPICKERWIDGET:
      PERSISTENT(
        wxColour,
        (reinterpret_cast<wxColourPickerWidget*>(m_window))->GetColour());
      break;

    case COMBOBOX:
    case COMBOBOX_DIR:
    case COMBOBOX_FILE:
      if (auto* cb = reinterpret_cast<wxComboBox*>(m_window); save)
      {
        if (const auto l = to_list_string(cb, m_max_items).get();
            m_label == find_replace_data::get()->text_find())
        {
          find_replace_data::get()->set_find_strings(l);
        }
        else if (m_label == find_replace_data::get()->text_replace_with())
        {
          find_replace_data::get()->set_replace_strings(l);
        }
        else
        {
          config(m_label).set(l);
        }
      }
      else
      {
        combobox_from_list(
          cb,
          config(m_label).get(
            !m_data.initial().has_value() ?
              config::strings_t{} :
              std::any_cast<config::strings_t>(m_data.initial())));
      }
      break;

    case DIRPICKERCTRL:
      PERSISTENT(std::string, m_label);
      break;

    case FILEPICKERCTRL:
      if (save)
      {
        config(m_label).set(std::any_cast<std::string>(get_value()));
      }
      else
      {
        std::string initial;

#ifdef __WXGTK__
        initial = "/usr/bin/" + m_label;

        if (!std::filesystem::is_regular_file(initial))
        {
          initial.clear();
        }
#endif
        set_value(config(m_label).get(initial));
      }
      break;

    case FONTPICKERCTRL:
      PERSISTENT(wxFont, wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));
      break;

    case GRID:
      if (save)
        config(m_label).set(std::any_cast<std::string>(get_value()));
      else
        set_value(config(m_label).get());
      break;

    case LISTVIEW:
      if (save)
        config(m_label).set(std::any_cast<config::strings_t>(get_value()));
      else
        set_value(config(m_label).get(
          !m_data.initial().has_value() ?
            config::strings_t{} :
            std::any_cast<config::strings_t>(m_data.initial())));
      break;

    case RADIOBOX:
      if (auto* rb = reinterpret_cast<wxRadioBox*>(m_window); save)
      {
        for (const auto& b : std::any_cast<choices_t>(m_data.initial()))
        {
          if (before(b.second, ',') == rb->GetStringSelection())
          {
            config(m_label).set(b.first);
          }
        }
      }
      else
      {
        const auto& choices(std::any_cast<choices_t>(m_data.initial()));

        if (const auto c =
              choices.find(config(m_label).get(rb->GetSelection()));
            c != choices.end())
        {
          rb->SetStringSelection(before(c->second, ','));
        }
      }
      break;

    case SLIDER:
      PERSISTENT(int, (reinterpret_cast<wxSlider*>(m_window))->GetValue());
      break;

    case SPINCTRL:
      PERSISTENT(int, (reinterpret_cast<wxSpinCtrl*>(m_window))->GetValue());
      break;

    case SPINCTRLDOUBLE:
      PERSISTENT(
        double,
        (reinterpret_cast<wxSpinCtrlDouble*>(m_window))->GetValue());
      break;

    case TEXTCTRL:
      PERSISTENT(std::string, std::any_cast<std::string>(get_value()));
      break;

    case TEXTCTRL_FLOAT:
      PERSISTENT(double, std::any_cast<double>(get_value()));
      break;

    case TEXTCTRL_INT:
      PERSISTENT(long, std::any_cast<long>(get_value()));
      break;

    case TOGGLEBUTTON:
      PERSISTENT(bool, false);
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

  if (m_data.apply() != nullptr)
  {
    (m_data.apply())(m_window, get_value(), save);
  }

  return true;
}
