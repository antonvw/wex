////////////////////////////////////////////////////////////////////////////////
// Name:      macro-mode.cpp
// Purpose:   Implementation of class wex::macro_mode
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/algorithm/string.hpp>
#include <wex/core/core.h>
#include <wex/core/regex.h>
#include <wex/factory/stc.h>
#include <wex/ui/frame.h>
#include <wex/ui/statusbar.h>
#include <wex/vi/ex.h>
#include <wex/vi/macro-mode.h>
#include <wex/vi/macros.h>
#include <wx/app.h>
#include <wx/choicdlg.h>

#include "macro-fsm.h"
#include "util.h"

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

  auto* frame = dynamic_cast<wex::frame*>(wxTheApp->GetTopWindow());

  std::string      macro(boost::algorithm::trim_copy(command));
  const ex_command cmd(ex != nullptr ? ex->get_command() : ex_command());

  frame->get_statusbar()->pane_show("PaneMacro", true);

  switch (macro[0])
  {
    case 'q':
      if (int x = transition_q(macro, ex, complete); x >= 0)
      {
        return x;
      }
      break;

    case '@':
      if (int x = transition_at(macro, ex, complete, repeat); x >= 0)
      {
        return x;
      }
      break;

    default:
      return 0;
  }

  if (ex != nullptr)
    ex->m_command.restore(cmd);

  return command.size();
}

int wex::macro_mode::transition_q(std::string& macro, ex* ex, bool complete)
{
  macro.erase(0, 1);

  if (complete)
  {
    if (macro.empty())
    {
      auto* frame = dynamic_cast<wex::frame*>(wxTheApp->GetTopWindow());
      frame->stc_entry_dialog_component()->set_text(get_macro());
      frame->stc_entry_dialog_title(_("Enter Macro"));
      frame->stc_entry_dialog_validator("[A-Za-z0-9][a-z0-9]*");

      if (
        frame->show_stc_entry_dialog(true) != wxID_OK ||
        (macro = frame->stc_entry_dialog_component()->get_text()) ==
          std::string())
      {
        return 0;
      }
    }
  }
  else if (m_fsm->get() == macro_fsm::state_t::IDLE && macro.empty())
  {
    return 0;
  }

  m_fsm->record(macro, ex);

  return -1;
}

int wex::macro_mode::transition_at(
  std::string& macro,
  ex*          ex,
  bool         complete,
  int          repeat)
{
  wxWindow* parent = (ex != nullptr ? ex->get_stc() : wxTheApp->GetTopWindow());

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
  else if (register_after("@", macro))
  {
    macro = macro.back();

    if (!ex::get_macros().is_recorded(macro))
    {
      return 2;
    }
  }
  else
  {
    auto* frame = dynamic_cast<wex::frame*>(wxTheApp->GetTopWindow());

    if (regex v("@([a-zA-Z].+)@"); v.match(macro) > 0)
    {
      macro = v[0];
    }
    else if (ex::get_macros().starts_with(macro.substr(1)))
    {
      if (std::string s;
          auto_complete_text(macro.substr(1), ex::get_macros().get(), s))
      {
        frame->statustext(s, "PaneMacro");
        macro = s;
      }
      else
      {
        frame->statustext(macro.substr(1), "PaneMacro");
        return 0;
      }
    }
    else
    {
      if (ex != nullptr)
      {
        frame->statustext(get_macro(), "PaneMacro");
      }
      return macro.size();
    }
  }

  if (ex::get_macros().is_recorded_macro(macro))
    m_fsm->playback(macro, ex, repeat);
  else
    m_fsm->expand_variable(macro, ex);

  return -1;
}
