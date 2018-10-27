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

bool ShowDialog(wxWindow* parent, std::string& macro)
{
  if (const auto& v(wex::vi_macros::Get()); !v.empty())
  {
    wxArrayString macros;
    macros.resize(v.size());
    std::copy(v.begin(), v.end(), macros.begin());
  
    wxSingleChoiceDialog dialog(parent,
      _("Input") + ":", 
      _("Select Macro"),
      macros);

    if (const auto index = macros.Index(wex::vi_macros::GetMacro()); index != wxNOT_FOUND)
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

bool wex::vi_macros_mode::Expand(
  ex* ex, const variable& v, std::string& expanded)
{
  return m_FSM->Expand(ex, v, expanded);
}

bool wex::vi_macros_mode::IsPlayback() const
{
  return m_FSM->IsPlayback();
}

bool wex::vi_macros_mode::IsRecording() const 
{
  return m_FSM->Get() == vi_macros_fsm::state::RECORDING;
}

const std::string wex::vi_macros_mode::String() const
{
  return m_FSM->State();
}

int wex::vi_macros_mode::Transition(
  const std::string& command, ex* ex, bool complete, int repeat)
{
  if (command.empty() || repeat <= 0)
  {
    return 0;
  }

  vi_macros_fsm::trigger trigger = vi_macros_fsm::trigger::DONE;
  wxWindow* parent = (ex != nullptr ? ex->GetSTC(): wxTheApp->GetTopWindow());

  std::string macro(command);

  switch (macro[0])
  {
    case 'q': 
      trigger = vi_macros_fsm::trigger::RECORD;

      macro = macro.substr(1);

      if (complete)
      {
        if (macro.empty())
        {
          wxTextEntryDialog dlg(parent,
            _("Input") + ":",
            _("Enter Macro"),
            vi_macros::GetMacro());
        
          wxTextValidator validator(wxFILTER_ALPHANUMERIC);
          dlg.SetTextValidator(validator);
        
          if (dlg.ShowModal() != wxID_OK)
          {
            return 0;
          }
          
          macro = dlg.GetValue();
        }
      }
      else if (m_FSM->Get() == vi_macros_fsm::state::IDLE && macro.empty())
      {
        return 0;
      }
    break;

    case '@': 
      if (macro == "@")
      {
        if (complete)
        {
          if (!ShowDialog(parent, macro))
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
        if ((macro = vi_macros::GetMacro()) == std::string() &&
            !ShowDialog(parent, macro))
        {
          return 2;
        }
      }
      else if (regafter("@", macro))
      {
        macro = std::string(1, macro.back());

        if (!vi_macros::IsRecorded(macro))
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
        else if (vi_macros::StartsWith(macro.substr(1)))
        {
          if (std::string s;
            autocomplete_text(macro.substr(1), vi_macros::Get(), s))
          {
            frame::StatusText(s, "PaneMacro");
            macro = s;
          }
          else
          {
            frame::StatusText(macro.substr(1), "PaneMacro");
            return 0;
          }
        }
        else
        {
          frame::StatusText(vi_macros::GetMacro(), "PaneMacro");
          return macro.size();
        }
      }

      trigger = vi_macros::IsRecordedMacro(macro) ? 
        vi_macros_fsm::trigger::PLAYBACK: vi_macros_fsm::trigger::EXPAND_VARIABLE; 
    break;

    default: return 0;
  }

  const ex_command cmd(ex != nullptr ? ex->GetCommand(): ex_command());
  m_FSM->Execute(trigger, macro, ex, repeat);
  if (ex != nullptr) ex->m_Command.Restore(cmd);

  return command.size();
}
