////////////////////////////////////////////////////////////////////////////////
// Name:      config_item.cpp
// Purpose:   Implementation of wex::item class config methods
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
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

#define PERSISTENT(TYPE, DEFAULT)                         \
{                                                         \
  if (save)                                               \
    config(m_Label).set(std::any_cast<TYPE>(GetValue())); \
  else                                                    \
    SetValue((TYPE)config(m_Label).get(DEFAULT));         \
}                                                         \

bool Update(wex::find_replace_data* frd, wxCheckListBox* clb, int item, bool save, bool value)
{
  if (const wxString field(clb->GetString(item));
    field == frd->GetTextMatchWholeWord())
  {
    !save ? clb->Check(item, frd->MatchWord()): frd->SetMatchWord(value);
  }
  else if (field == frd->GetTextMatchCase())
  {
    !save ? clb->Check(item, frd->MatchCase()): frd->SetMatchCase(value);
  }
  else if (field == frd->GetTextRegEx())
  {
    !save ? clb->Check(item, frd->UseRegEx()): frd->SetUseRegEx(value);
  }
  else if (field == frd->GetTextSearchDown())
  {
    frd->SetFlags(value ? wxFR_DOWN: ~wxFR_DOWN);
  }
  else
  {
    return false;
  }

  return true;
}

bool wex::item::ToConfig(bool save) const
{
  if (!m_UseConfig)
  {
    return false;
  }
  
  switch (GetType())
  {
    case CHECKBOX:           PERSISTENT(bool, false); break;
    case CHECKLISTBOX_BIT:   PERSISTENT(long, 0); break;
    case COLOURPICKERWIDGET: PERSISTENT(wxColour, *wxWHITE); break;
    case DIRPICKERCTRL:      PERSISTENT(std::string, m_Label); break;
    case TEXTCTRL_FLOAT:     PERSISTENT(double, 0); break;
    case FONTPICKERCTRL:     PERSISTENT(wxFont, wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT)); break;
    case TEXTCTRL_INT:       PERSISTENT(long, 0); break;
    case SLIDER:             PERSISTENT(int, ((wxSlider* )GetWindow())->GetMin()); break;
    case SPINCTRL:           PERSISTENT(int, ((wxSpinCtrl* )GetWindow())->GetMin()); break;
    case SPINCTRLDOUBLE:     PERSISTENT(double, ((wxSpinCtrlDouble* )GetWindow())->GetMin()); break;
    case STC:                PERSISTENT(std::string, std::string()); break;
    case TEXTCTRL:           PERSISTENT(std::string, std::string()); break;
    case TOGGLEBUTTON:       PERSISTENT(bool, false); break;

    case CHECKLISTBOX_BOOL:
      if (auto* clb = (wxCheckListBox*)GetWindow();
        clb != nullptr)
      {
        for (size_t i = 0; i < clb->GetCount(); i++)
        {
          if (!Update(find_replace_data::Get(), clb, i, save, clb->IsChecked(i)))
          {
            if (save)
              config(clb->GetString(i)).set(clb->IsChecked(i));
            else
              clb->Check(i, config(clb->GetString(i)).get(false));
          }
        }
      }
      break;

    case COMBOBOX:
    case COMBOBOX_DIR:
    case COMBOBOX_FILE:
      if (auto* cb = (wxComboBox*)GetWindow(); save)
      {
        if (const auto l = to_list_string(cb, m_MaxItems).Get();
          m_Label == find_replace_data::Get()->GetTextFindWhat())
        {
          find_replace_data::Get()->SetFindStrings(l);
        }
        else if (m_Label == find_replace_data::Get()->GetTextReplaceWith())
        {
          find_replace_data::Get()->SetReplaceStrings(l);
        }
        else
        {
          config(m_Label).set(l);
        }
      }
      else
      {
        combobox_from_list(cb, config(m_Label).get_list());
      }
      break;

    case FILEPICKERCTRL:
      if (save)
      {
        config(m_Label).set(std::any_cast<std::string>(GetValue()));
      }
      else
      {
        std::string initial;

#ifdef __WXGTK__
        initial = "/usr/bin/" + m_Label;

        if (!std::filesystem::is_regular_file(initial))
        {
          initial.clear();
        }
#endif
        SetValue(config(m_Label).get(initial));
      }
      break;

    case LISTVIEW:
      if (save)
        config(m_Label).set(std::any_cast<std::string>(GetValue()));
      else
        SetValue(config(m_Label).get());
      break;

    case RADIOBOX:
      if (auto* rb = (wxRadioBox*)GetWindow(); save)
      {
        for (const auto& b : std::any_cast<item::Choices>(GetInitial()))
        {
          if (b.second == rb->GetStringSelection())
          {
            config(m_Label).set(b.first);
          }
        }
      }
      else
      {
        const item::Choices & choices(std::any_cast<item::Choices>(GetInitial()));
        
        if (const auto c = choices.find(config(m_Label).get((int)0));
          c != choices.end())
        {
          rb->SetStringSelection(c->second);
        }
      }
      break;

    case USER:
      if (m_UserWindowToConfig != nullptr &&
        !(m_UserWindowToConfig)(GetWindow(), save))
      { 
        return false;
      }
      break;
      
    default:
      // the other types have no persistent info
      return false;
      break;
  }

  if (m_Apply != nullptr)
  {
    (m_Apply)(m_Window, GetValue(), save);
  }

  return true;
}

wex::config_defaults::config_defaults(const std::vector<Defaults> & items)
{
  config cfg(config::DATA_NO_STORE);
  
  if (!cfg.item(std::get<0>(items.front())).exists())
  {
    config::set_record_defaults(true);
    
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
            cfg.item(std::get<0>(it)).set(std::any_cast<std::string>(std::get<2>(it)));
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

wex::config_defaults::~config_defaults()
{
  config::set_record_defaults(false);
}
