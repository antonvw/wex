////////////////////////////////////////////////////////////////////////////////
// Name:      vi-macros-fsm.cpp
// Purpose:   Implementation of class wex::vi_macros_fsm
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <fstream>
#include <pugixml.hpp>
#include <wex/ex.h>
#include <wex/config.h>
#include <wex/log.h>
#include <wex/managedframe.h>
#include <wex/stc.h>
#include <wex/util.h>
#include <wex/variable.h>
#include <wex/vi-macros.h>
#include "vi-macros-fsm.h"

wex::vi_macros_fsm::vi_macros_fsm()
{
  m_fsm.add_transitions({
     // from-state,              to-state,            trigger,    guard,         action
    // expanding
    {EXPANDING_TEMPLATE,         IDLE,                DONE,       nullptr,       nullptr},
    {EXPANDING_VARIABLE,         IDLE,                DONE,       nullptr,       nullptr},
    // idle
    {IDLE,                       EXPANDING_TEMPLATE,  EXPAND_TEMPLATE, 
      [&]{return 
           !m_variable.get_name().empty() &&
            m_variable.is_template() &&
           !m_variable.get_value().empty();},
      [&]{ExpandingTemplate();}},
    {IDLE,                        EXPANDING_VARIABLE, EXPAND_VARIABLE, nullptr,
      [&]{ExpandingVariable();}},
    {IDLE,                        PLAYINGBACK,        PLAYBACK,   [&]{return m_count > 0;}, 
      [&]{Playback();}},
    {IDLE,                        RECORDING,          RECORD,     nullptr,
      [&]{StartRecording();}},
    // playingback
    {PLAYINGBACK,                 IDLE,               DONE,       nullptr,       nullptr},
    {PLAYINGBACK,                 PLAYINGBACK,        PLAYBACK, 
      [&]{return m_count > 0 && m_macro != vi_macros::m_Macro;}, 
      [&]{Playback();}},
    // recording
    {PLAYINGBACK_WHILE_RECORDING, RECORDING,          DONE,       nullptr,       nullptr},
    {RECORDING,                   IDLE,               RECORD,     nullptr,       [&]{StopRecording();}},
    {RECORDING,                   PLAYINGBACK_WHILE_RECORDING, PLAYBACK, 
      [&]{return m_count > 0 && m_macro != vi_macros::m_Macro;}, 
      [&]{Playback();}}});

  m_fsm.add_debug_fn(verbose);
}

bool wex::vi_macros_fsm::execute(
  trigger_t trigger, const std::string& macro, ex* ex, int repeat) 
{
  m_count = repeat;
  m_error = false;
  m_ex = ex;
  m_macro = macro;

  if (m_fsm.execute(trigger) != FSM::Fsm_Success)
  {
    return false;
  }

  if (m_fsm.state() == PLAYINGBACK || 
      m_fsm.state() == PLAYINGBACK_WHILE_RECORDING ||
      m_fsm.state() == EXPANDING_VARIABLE)
  {
    m_fsm.execute(DONE);
  }

  frame::statustext(vi_macros::m_Macro, "PaneMacro");
  frame::statustext(state(), "PaneMode");

  if (m_ex != nullptr)
  {
    ((statusbar *)m_ex->frame()->GetStatusBar())->show_field(
      "PaneMode", 
      m_fsm.state() != IDLE && config(_("Show mode")).get(false));
  }

  return !m_error;
}

bool wex::vi_macros_fsm::expand(
  ex* ex, const variable& v, std::string& expanded)
{
  m_error = false;
  m_ex = ex;
  m_expanded = &expanded;
  m_variable = v;

  FSM::Fsm_Errors r1 = m_fsm.execute(EXPAND_TEMPLATE);
  FSM::Fsm_Errors r2 = m_fsm.execute(DONE);

  return r1 == FSM::Fsm_Success && r2 == FSM::Fsm_Success && !m_error;
}

void wex::vi_macros_fsm::ExpandingTemplate()
{
  // Read the file (file name is in variable value), expand
  // all macro variables in it, and set expanded to the result.
  const path filename(config().dir(), m_variable.get_value());

  std::ifstream ifs(filename.data());
  
  if (!ifs.is_open())
  {
    log::verbose("could not open template file:") << filename.data().string();
    m_error = true;
    return;
  }

  set_ask_for_input();

  for (char c; ifs.get(c) && !m_error; )
  {
    if (c != '@')
    {
      *m_expanded += c;
    }
    else
    {
      std::string variable;
      bool completed = false;
      
      while (!completed && ifs.get(c)) 
      {
        if (c != '@')
        {
          variable += c;
        }
        else
        {
          completed = true;
        }
      }
      
      if (!completed)
      {
        log() << "variable syntax error:" << variable;
        m_error = true;
      }
      // Prevent recursion.
      else if (variable == m_variable.get_name())
      {
        log() << "recursive variable:" << variable;
        m_error = true;
      }
      else
      {
        if (std::string value; !ExpandingVariable(variable, &value))
        {
          m_error = true;
        }
        else
        {
          *m_expanded += value;
        }
      }
    }
  }
  
  // Set back to normal value.  
  set_ask_for_input();
    
  vi_macros::m_Macro = m_variable.get_name();
  frame::statustext(vi_macros::m_Macro, "PaneMacro");
    
  log::status(_("Macro expanded"));
}

void wex::vi_macros_fsm::ExpandingVariable()
{
  if (!ExpandingVariable(m_macro, nullptr))
  {
    m_error = true;
  }
  else
  {
    vi_macros::m_Macro = m_macro;
  }
}

bool wex::vi_macros_fsm::ExpandingVariable(
  const std::string& name, std::string* value) const
{
  pugi::xml_node node;
  variable* var;
    
  if (const auto& it = vi_macros::m_Variables.find(name);
    it == vi_macros::m_Variables.end())
  {
    const auto& itn = vi_macros::m_Variables.insert({name, variable(name)});
    var = &itn.first->second;
    node = vi_macros::m_doc.document_element().append_child("variable");
  }
  else
  {
    var = &it->second;

    try
    {
      // node = vi_macros::m_doc.document_element().child(name.c_str());
      const std::string query("//variable[@name='" + name + "']");

      if (auto xp = vi_macros::m_doc.document_element().select_node(query.c_str());
        xp && xp.node())
      {
        node = xp.node();
      }
      else
      {
        log() << "xml query failed:" << query;
        return false;
      }
    }
    catch (pugi::xpath_exception& e)
    {
      log(e) << name;
      return false;
    }
  }

  if (value == nullptr)
  {
    if (!var->expand(m_ex))
    {
      return false;
    }
  }
  else if (!var->expand(*value, m_ex))     
  {
    return false;
  }

  var->set_ask_for_input(false);
  vi_macros::m_is_modified = true;
  var->save(node, value);
  log::status(_("Variable expanded"));

  return true;
}

bool wex::vi_macros_fsm::is_playback() const
{
  return m_playback;
}

void wex::vi_macros_fsm::Playback()
{
  wxBusyCursor wait;
  m_ex->get_stc()->BeginUndoAction();
  m_playback = true;
    
  set_ask_for_input();
  
  const auto& macro_commands(vi_macros::m_Macros[m_macro]);

  for (int i = 0; i < m_count && !m_error; i++)
  {
    for (const auto& it : macro_commands)
    { 
      if (!m_ex->command(it))
      {
        m_error = true;
        log::status(_("Macro aborted at")) <<  it;
        break;
      }
    }
  }

  m_ex->get_stc()->EndUndoAction();
  m_playback = false;

  if (!m_error)
  {
    vi_macros::m_Macro = m_macro;
    log::status(_("Macro played back"));
  }
}

void wex::vi_macros_fsm::set_ask_for_input() const
{
  for (auto& it : vi_macros::m_Variables)
  {
    it.second.set_ask_for_input();
  }
}

void wex::vi_macros_fsm::StartRecording()
{
  vi_macros::m_is_modified = true;
  
  if (m_macro.size() == 1)
  {
    // We only use lower case macro's, to be able to
    // append to them using.
    vi_macros::m_Macro = m_macro;
    std::transform(vi_macros::m_Macro.begin(), vi_macros::m_Macro.end(), vi_macros::m_Macro.begin(), ::tolower);
  
    // Clear macro if it is lower case
    // (otherwise append to the macro).
    if (islower(m_macro[0]))
    {
      vi_macros::m_Macros[vi_macros::m_Macro].clear();
    }
  }
  else
  {
    vi_macros::m_Macro = m_macro;
    vi_macros::m_Macros[vi_macros::m_Macro].clear();
  }

  log::status(_("Macro recording"));
}

void wex::vi_macros_fsm::StopRecording()
{
  if (!vi_macros::get(vi_macros::m_Macro).empty())
  {
    vi_macros::save_macro(vi_macros::m_Macro);
    log::status("Macro") << vi_macros::m_Macro << "is recorded";
  }
  else
  {
    vi_macros::m_Macros.erase(vi_macros::m_Macro);
    vi_macros::m_Macro.clear();
    log::status(std::string());
  }
}

void wex::vi_macros_fsm::verbose(state_t from, state_t to, trigger_t t)
{
  log::verbose(2) << 
    "vi macro" << m_macro <<
    "trigger" << trigger(t) <<
    "state from" << state(from) << 
    "to" << state(to);
}
