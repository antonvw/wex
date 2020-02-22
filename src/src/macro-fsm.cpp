////////////////////////////////////////////////////////////////////////////////
// Name:      macro-fsm.cpp
// Purpose:   Implementation of class wex::macro_fsm
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <fstream>
#include <pugixml.hpp>
#include <boost/mpl/list.hpp>
#include <boost/statechart/custom_reaction.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <wex/ex.h>
#include <wex/config.h>
#include <wex/log.h>
#include <wex/macros.h>
#include <wex/macro-mode.h>
#include <wex/managedframe.h>
#include <wex/stc.h>
#include <wex/util.h>
#include <wex/variable.h>
#include "macro-fsm.h"

namespace mpl = boost::mpl;

namespace wex
{
  struct ssACTIVE : sc::simple_state < ssACTIVE, macro_fsm, ssIDLE > {;};

  struct ssIDLE : sc::state< ssIDLE, ssACTIVE >
  {
    typedef sc::transition< macro_fsm::evRECORD, ssRECORDING > reactions;

    ssIDLE(my_context ctx) : my_base ( ctx )
    {
      context< macro_fsm >().state(macro_fsm::IDLE);
    }
  };

  struct ssRECORDING : sc::state < ssRECORDING, ssACTIVE > 
  {
    typedef sc::custom_reaction< macro_fsm::evRECORD > reactions;

    ssRECORDING(my_context ctx) : my_base ( ctx )
    {
      context< macro_fsm >().state(macro_fsm::RECORDING);
    };

    sc::result react( const macro_fsm::evRECORD &) 
    {
      context< macro_fsm >().recorded();
      return transit< ssIDLE >();
    };
  };
}

wex::macro_fsm::macro_fsm(macro_mode* mode)
  : m_mode(mode)
{
  initiate();
}

bool wex::macro_fsm::expand_template(
  const variable& var, ex* ex, std::string& expanded)
{
  if (
    ex == nullptr ||
    var.get_name().empty() ||
    var.get_value().empty() ||
    !var.is_template())
  {
    return false;
  }
  
  // Read the file (file name is in variable value), expand
  // all macro variables in it, and set expanded to the result.
  const path filename(config::dir(), var.get_value());

  std::ifstream ifs(filename.data());
  
  if (!ifs.is_open())
  {
    log::verbose("could not open template file") << filename;
    return false;
  }

  set_ask_for_input();
    
  bool error = false;

  for (char c; ifs.get(c) && !error; )
  {
    if (c != '@')
    {
      expanded += c;
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
        error = true;
      }
      // Prevent recursion.
      else if (variable == var.get_name())
      {
        log() << "recursive variable:" << variable;
        error = true;
      }
      else
      {
        if (std::string value; !expanding_variable(ex, variable, &value))
        {
          error = true;
        }
        else
        {
          expanded += value;
        }
      }
    }
  }
  
  // Set back to normal value.  
  set_ask_for_input();
    
  m_macro = var.get_name();
  frame::statustext(m_macro, "PaneMacro");
    
  log::status(_("Macro expanded"));

  return !error;
}

bool wex::macro_fsm::expand_variable(
  const std::string& name, ex* ex) const
{
  return expanding_variable(ex, name, nullptr);
}

bool wex::macro_fsm::expanding_variable(
  ex* ex, const std::string& name, std::string* value) const
{
  pugi::xml_node node;
  variable* var;
    
  if (auto it = m_mode->get_macros()->m_variables.find(name);
    it == m_mode->get_macros()->m_variables.end())
  {
    const auto& itn = m_mode->get_macros()->m_variables.insert({name, variable(name)});
    var = &itn.first->second;
    node = m_mode->get_macros()->m_doc.document_element().append_child("variable");
  }
  else
  {
    var = &it->second;

    try
    {
      // node = m_mode->get_macros()->m_doc.document_element().child(name.c_str());
      const std::string query("//variable[@name='" + name + "']");

      if (auto xp = m_mode->get_macros()->m_doc.document_element().select_node(query.c_str());
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
    if (!var->expand(ex))
    {
      return false;
    }
  }
  else if (!var->expand(*value, ex))     
  {
    return false;
  }

  var->set_ask_for_input(false);
  m_mode->get_macros()->m_is_modified = true;
  var->save(node, value);
  log::status(_("Variable expanded"));

  return true;
}

void wex::macro_fsm::playback(const std::string& macro, ex* ex, int repeat)
{
  assert(ex != nullptr);

  if (repeat <= 0)
  {
    return;
  }
  
  if ((m_playback || m_state == RECORDING) && macro == m_macro)
  {
    log("recursive playback") << macro;
    return;
  }
  
  ex->get_stc()->BeginUndoAction();
  set_ask_for_input();
  m_playback = true;
  bool error = false;
  
  if (m_state == IDLE)
  {
    m_macro = macro;
  }
  
  const auto& commands(m_mode->get_macros()->get_macro_commands(macro));

  for (int i = 0; i < repeat && !error; i++)
  {
    for (const auto& it : commands)
    { 
      if (!ex->command(it))
      {
        error = true;
        log::status(_("Macro aborted at")) <<  it;
        break;
      }
    }
  }

  ex->get_stc()->EndUndoAction();

  if (!error)
  {
    log::status(_("Macro played back"));
  }
  
  m_playback = false;
}

void wex::macro_fsm::record(const std::string& macro, ex* ex)
{
  // an empty macro is used to stop recording
  if (!macro.empty())
  {
    m_macro = macro;
  }

  process_event(macro_fsm::evRECORD());

  frame::statustext(m_macro, "PaneMacro");
  frame::statustext(str(), "PaneMode");

  if (ex != nullptr)
  {
    ((statusbar *)ex->frame()->GetStatusBar())->show_pane(
      "PaneMode", 
      m_state != IDLE && config(_("stc.Show mode")).get(true));
  }
}

void wex::macro_fsm::recorded()
{
  if (m_macro.empty())
  {
    log("empty macro recorded");
  }
  else if (!m_mode->get_macros()->find(m_macro).empty())
  {
    m_mode->get_macros()->save_macro(m_macro);
    log::status("Macro") << m_macro << "is recorded";
    log::verbose("macro") << m_macro << "is recorded";
  }
  else
  {
    log::verbose("Macro") << m_macro << "not recorded";
    m_mode->get_macros()->erase();
    m_macro.clear();
    log::status(std::string());
  }
}
  
void wex::macro_fsm::set_ask_for_input() const
{
  for (auto& it : m_mode->get_macros()->m_variables)
  {
    it.second.set_ask_for_input();
  }
}

void wex::macro_fsm::state(state_t s)
{
  m_state = s;
  
  if (s == RECORDING)
  {
    log::verbose("vi macro") << "recording" << m_macro;
    log::status(_("Macro recording"));
      
    if (m_macro.size() == 1)
    {
      // Clear macro if it is lower case
      // (otherwise append to the macro).
      if (islower(m_macro[0]))
      {
        m_mode->get_macros()->m_macros[m_macro].clear();
      }

      // We only use lower case macro's, to be able to
      // append to them using.
      std::transform(
        m_macro.begin(), 
        m_macro.end(), 
        m_macro.begin(), ::tolower);
    }
    else
    {
      m_mode->get_macros()->m_macros[m_macro].clear();
    }
  }
}
