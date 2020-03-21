////////////////////////////////////////////////////////////////////////////////
// Name:      macro-mode.cpp
// Purpose:   Implementation of class wex::macro_mode
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include "macro-fsm.h"
#include <wex/ex.h>
#include <wex/macro-mode.h>
#include <wex/macros.h>
#include <wex/managedframe.h>
#include <wex/stc.h>
#include <wex/util.h>

bool show_dialog(
  wxWindow*          parent,
  const std::string& current,
  std::string&       macro)
{
  if (const auto& v(wex::ex::get_macros().get()); !v.empty())
  {
    wxArrayString macros;
    macros.resize(v.size());
    std::copy(v.begin(), v.end(), macros.begin());

    wxSingleChoiceDialog dialog(
      parent,
      _("Input") + ":",
      _("Select Macro"),
      macros);

    if (const auto index = macros.Index(current); index != wxNOT_FOUND)
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

wex::macro_mode::macro_mode(macros* macros)
  : m_fsm(new macro_fsm(this))
  , m_macros(macros)
{
}

wex::macro_mode::~macro_mode()
{
  delete m_fsm;
}

bool wex::macro_mode::expand(ex* ex, const variable& v, std::string& expanded)
  const
{
  return m_fsm->expand_template(v, ex, expanded);
}

const std::string wex::macro_mode::get_macro() const
{
  return m_fsm->get_macro();
}

bool wex::macro_mode::is_playback() const
{
  return m_fsm->is_playback();
}

bool wex::macro_mode::is_recording() const
{
  return m_fsm->get() == macro_fsm::state_t::RECORDING;
}

const std::string wex::macro_mode::str() const
{
  return m_fsm->str();
}

int wex::macro_mode::transition(
  const std::string& command,
  ex*                ex,
  bool               complete,
  int                repeat)
{
  if (command.empty() || repeat <= 0)
  {
    return 0;
  }

  wxWindow* parent = (ex != nullptr ? ex->get_stc() : wxTheApp->GetTopWindow());

  std::string      macro(trim(command));
  const ex_command cmd(ex != nullptr ? ex->get_command() : ex_command());

  if (ex != nullptr)
  {
    ((wex::statusbar*)ex->frame()->GetStatusBar())
      ->pane_show("PaneMacro", true);
  }

  switch (macro[0])
  {
    case 'q':
      macro.erase(0, 1);

      if (complete)
      {
        if (macro.empty())
        {
          wxTextEntryDialog dlg(
            parent,
            _("Input") + ":",
            _("Enter Macro"),
            get_macro());

          wxTextValidator validator(wxFILTER_ALPHANUMERIC);
          dlg.SetTextValidator(validator);

          if (dlg.ShowModal() != wxID_OK)
          {
            return 0;
          }

          macro = dlg.GetValue();
        }
      }
      else if (m_fsm->get() == macro_fsm::state_t::IDLE && macro.empty())
      {
        return 0;
      }
      m_fsm->record(macro, ex);
      break;

    case '@':
      if (macro == "@")
      {
        if (complete)
        {
          if (!show_dialog(parent, get_macro(), macro))
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
        if (
          (macro = get_macro()) == std::string() &&
          !show_dialog(parent, get_macro(), macro))
        {
          return 2;
        }
      }
      else if (regafter("@", macro))
      {
        macro = std::string(1, macro.back());

        if (!ex::get_macros().is_recorded(macro))
        {
          return 2;
        }
      }
      else
      {
        if (std::vector<std::string> v; match("@([a-zA-Z].+)@", macro, v) > 0)
        {
          macro = v[0];
        }
        else if (ex::get_macros().starts_with(macro.substr(1)))
        {
          if (std::string s;
              autocomplete_text(macro.substr(1), ex::get_macros().get(), s))
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
          frame::statustext(get_macro(), "PaneMacro");
          return macro.size();
        }
      }

      if (ex::get_macros().is_recorded_macro(macro))
        m_fsm->playback(macro, ex, repeat);
      else
        m_fsm->expand_variable(macro, ex);
      break;

    default:
      return 0;
  }

  if (ex != nullptr)
    ex->m_command.restore(cmd);

  return command.size();
}
