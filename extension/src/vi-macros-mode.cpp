////////////////////////////////////////////////////////////////////////////////
// Name:      vi-macros-mode.cpp
// Purpose:   Implementation of class wex::vi_macros_mode
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/vi-macros-mode.h>
#include <wex/ex.h>
#include <wex/frame.h>
#include <wex/stc.h>
#include <wex/util.h>
#include <wex/vi-macros.h>
#include "vi-macros-fsm.h"

bool show_dialog(wxWindow* parent, std::string& macro)
{
  if (const auto& v(wex::vi_macros::get()); !v.empty())
  {
    wxArrayString macros;
    macros.resize(v.size());
    std::copy(v.begin(), v.end(), macros.begin());
  
    wxSingleChoiceDialog dialog(parent,
      _("Input") + ":", 
      _("Select Macro"),
      macros);

    if (const auto index = macros.Index(wex::vi_macros::get_macro()); 
      index != wxNOT_FOUND)
    {
      dialog.SetSelection(index);
    }

    if (dialog.ShowModal() == wxID_OK)
    {
      macro = dialog.GetStringSelection();
      return true;
    }
  }

  return false;
}

wex::vi_macros_mode::vi_macros_mode()
  : m_FSM(new vi_macros_fsm())
{
}

wex::vi_macros_mode::~vi_macros_mode()
{
  delete m_FSM;
}

bool wex::vi_macros_mode::expand(
  ex* ex, const variable& v, std::string& expanded)
{
  return m_FSM->expand(ex, v, expanded);
}

bool wex::vi_macros_mode::is_playback() const
{
  return m_FSM->is_playback();
}

bool wex::vi_macros_mode::is_recording() const 
{
  return m_FSM->get() == vi_macros_fsm::state_t::RECORDING;
}

const std::string wex::vi_macros_mode::string() const
{
  return m_FSM->state();
}

int wex::vi_macros_mode::transition(
  const std::string& command, ex* ex, bool complete, int repeat)
{
  if (command.empty() || repeat <= 0)
  {
    return 0;
  }

  vi_macros_fsm::trigger_t trigger = vi_macros_fsm::trigger_t::DONE;
  wxWindow* parent = (ex != nullptr ? ex->stc(): wxTheApp->GetTopWindow());

  std::string macro(command);

  switch (macro[0])
  {
    case 'q': 
      trigger = vi_macros_fsm::trigger_t::RECORD;

      macro = macro.substr(1);

      if (complete)
      {
        if (macro.empty())
        {
          wxTextEntryDialog dlg(parent,
            _("Input") + ":",
            _("Enter Macro"),
            vi_macros::get_macro());
        
          wxTextValidator validator(wxFILTER_ALPHANUMERIC);
          dlg.SetTextValidator(validator);
        
          if (dlg.ShowModal() != wxID_OK)
          {
            return 0;
          }
          
          macro = dlg.GetValue();
        }
      }
      else if (m_FSM->get() == vi_macros_fsm::state_t::IDLE && macro.empty())
      {
        return 0;
      }
    break;

    case '@': 
      if (macro == "@")
      {
        if (complete)
        {
          if (!show_dialog(parent, macro))
          {
            return 1;
          }
        }
        else
        {
          return 0;
        }
      }
      else if (macro == "@@") 
      {
        if ((macro = vi_macros::get_macro()) == std::string() &&
            !show_dialog(parent, macro))
        {
          return 2;
        }
      }
      else if (regafter("@", macro))
      {
        macro = std::string(1, macro.back());

        if (!vi_macros::is_recorded(macro))
        {
          return 2;
        }
      }
      else
      {
        if (std::vector <std::string> v;
          match("@([a-zA-Z].+)@", macro, v) > 0)
        {
          macro = v[0];
        }
        else if (vi_macros::starts_with(macro.substr(1)))
        {
          if (std::string s;
            autocomplete_text(macro.substr(1), vi_macros::get(), s))
          {
            frame::statustext(s, "PaneMacro");
            macro = s;
          }
          else
          {
            frame::statustext(macro.substr(1), "PaneMacro");
            return 0;
          }
        }
        else
        {
          frame::statustext(vi_macros::get_macro(), "PaneMacro");
          return macro.size();
        }
      }

      trigger = vi_macros::is_recorded_macro(macro) ? 
        vi_macros_fsm::trigger_t::PLAYBACK: vi_macros_fsm::trigger_t::EXPAND_VARIABLE; 
    break;

    default: return 0;
  }

  const ex_command cmd(ex != nullptr ? ex->get_command(): ex_command());
  m_FSM->execute(trigger, macro, ex, repeat);
  if (ex != nullptr) ex->m_Command.restore(cmd);

  return command.size();
}
