////////////////////////////////////////////////////////////////////////////////
// Name:      config_item.cpp
// Purpose:   Implementation of wex::item class config methods
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <filesystem>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/checklst.h>
#include <wx/spinctrl.h>
#include <wx/window.h>
#include <wex/config.h>
#include <wex/item.h>
#include <wex/frd.h>
#include <wex/log.h>
#include <wex/tostring.h>
#include <wex/util.h>

#define PERSISTENT(TYPE, DEFAULT)                          \
{                                                          \
  if (save)                                                \
    config(m_label).set(std::any_cast<TYPE>(get_value())); \
  else                                                     \
    set_value((TYPE)config(m_label).get(DEFAULT));         \
}                                                          \

bool update(
  wex::find_replace_data* frd, 
  wxCheckListBox* clb, 
  int item, 
  bool save, 
  bool value)
{
  if (const std::string field(clb->GetString(item));
    field == frd->text_match_word())
  {
    !save ? clb->Check(
       item, frd->match_word()): frd->set_match_word(value);
  }
  else if (field == frd->text_match_case())
  {
    !save ? clb->Check(
       item, frd->match_case()): frd->set_match_case(value);
  }
  else if (field == frd->text_regex())
  {
    !save ? clb->Check(
       item, frd->use_regex()): frd->set_use_regex(value);
  }
  else if (field == frd->text_search_down())
  {
    !save ? clb->Check(
       item, frd->search_down()): frd->set_search_down(value);
  }
  else
  {
    return false;
  }

  return true;
}

bool wex::item::to_config(bool save) const
{
  if (!m_use_config)
  {
    return false;
  }
  
  switch (type())
  {
    case CHECKBOX:           
      PERSISTENT(bool, false); 
      break;

    case CHECKLISTBOX_BIT:   
      PERSISTENT(long, 0); 
      break;
    
    case CHECKLISTBOX_BOOL:
      if (auto* clb = (wxCheckListBox*)window();
        clb != nullptr)
      {
        for (size_t i = 0; i < clb->GetCount(); i++)
        {
          if (!update(find_replace_data::get(), clb, i, save, clb->IsChecked(i)))
          {
            if (save)
              config(clb->GetString(i)).set(clb->IsChecked(i));
            else
              clb->Check(i, config(clb->GetString(i)).get(false));
          }
        }
      }
      break;

    case COLOURPICKERWIDGET: 
      PERSISTENT(wxColour, *wxWHITE); 
      break;
    
    case COMBOBOX:
    case COMBOBOX_DIR:
    case COMBOBOX_FILE:
      if (auto* cb = (wxComboBox*)window(); save)
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
        combobox_from_list(cb, config(m_label).get(std::list<std::string>{}));
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
      PERSISTENT(
        wxFont, 
        wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT)); 
      break;
    
    case GRID:
    case LISTVIEW:
      if (save)
        config(m_label).set(std::any_cast<std::string>(get_value()));
      else
        set_value(config(m_label).get());
      break;

    case RADIOBOX:
      if (auto* rb = (wxRadioBox*)window(); save)
      {
        for (const auto& b : std::any_cast<item::choices_t>(initial()))
        {
          if (b.second == rb->GetStringSelection())
          {
            config(m_label).set(b.first);
          }
        }
      }
      else
      {
        const auto& choices(std::any_cast<item::choices_t>(initial()));
        
        if (const auto c = choices.find(config(m_label).get((int)0));
          c != choices.end())
        {
          rb->SetStringSelection(c->second);
        }
      }
      break;

    case SLIDER:             
      PERSISTENT(int, ((wxSlider* )window())->GetMin()); 
      break;
    
    case SPINCTRL:           
      PERSISTENT(int, ((wxSpinCtrl* )window())->GetMin()); 
      break;
    
    case SPINCTRLDOUBLE:     
      PERSISTENT(double, ((wxSpinCtrlDouble* )window())->GetMin()); 
      break;
    
    case STC:                
      PERSISTENT(std::string, std::string()); 
      break;
    
    case TEXTCTRL_FLOAT:
      if (m_initial.has_value())
      {
        PERSISTENT(double, std::stod(std::any_cast<std::string>(m_initial))); 
      }
      else
      {
        PERSISTENT(double, 0.0);
      }
      break;
    
    case TEXTCTRL_INT:       
      if (m_initial.has_value())
      {
        PERSISTENT(long, std::stol(std::any_cast<std::string>(m_initial))); 
      }
      else
      {
        PERSISTENT(long, 0l);
      }
      break;
    
    case TEXTCTRL:           
      if (m_initial.has_value())
      {
        PERSISTENT(std::string, std::any_cast<std::string>(m_initial));
      }
      else
      {
        PERSISTENT(std::string, std::string());
      }
      break;
    
    case TOGGLEBUTTON:       
      PERSISTENT(bool, false); 
      break;

    case USER:
      if (m_user_window_to_config_t != nullptr &&
        !(m_user_window_to_config_t)(window(), save))
      { 
        return false;
      }
      break;
      
    default:
      // the other types have no persistent info
      return false;
      break;
  }

  if (m_apply != nullptr)
  {
    (m_apply)(m_window, get_value(), save);
  }

  return true;
}

wex::config_defaults::config_defaults(const std::vector<default_t> & items)
{
  if (config cfg;
    !cfg.item(std::get<0>(items.front())).exists())
  {
    for (const auto& it : items)
    {
      try
      {
        switch (std::get<1>(it))
        {
          case item::CHECKBOX:
            cfg.item(std::get<0>(it)).set(std::any_cast<bool>(std::get<2>(it)));
            break;
          case item::COLOURPICKERWIDGET:
            cfg.item(std::get<0>(it)).set(std::any_cast<wxColour>(std::get<2>(it)));
            break;
          case item::COMBOBOX:
            cfg.item(std::get<0>(it)).set(std::any_cast<std::list<std::string>>(std::get<2>(it)));
            break;
          case item::FONTPICKERCTRL:
            cfg.item(std::get<0>(it)).set(std::any_cast<wxFont>(std::get<2>(it)));
            break;
          case item::SPINCTRL:
            cfg.item(std::get<0>(it)).set(std::any_cast<long>(std::get<2>(it)));
            break;
          case item::TEXTCTRL:
            cfg.item(std::get<0>(it)).set(std::any_cast<std::string>(std::get<2>(it)));
            break;
          case item::TEXTCTRL_FLOAT:
            cfg.item(std::get<0>(it)).set(std::any_cast<double>(std::get<2>(it)));
            break;
          case item::TEXTCTRL_INT:
            cfg.item(std::get<0>(it)).set(std::any_cast<long>(std::get<2>(it)));
            break;
          default:
            log("unsupported default type for") << std::get<0>(it);
        }
      }
      catch (std::bad_cast& e)
      {
        log(e) << std::get<0>(it);
      }
    }
  }
}
