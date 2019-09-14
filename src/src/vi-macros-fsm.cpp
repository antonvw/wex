////////////////////////////////////////////////////////////////////////////////
// Name:      vi-macros-fsm.cpp
// Purpose:   Implementation of class wex::vi_macros_fsm
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
#include <wex/managedframe.h>
#include <wex/stc.h>
#include <wex/util.h>
#include <wex/variable.h>
#include <wex/vi-macros.h>
#include "vi-macros-fsm.h"

namespace mpl = boost::mpl;

namespace wex
{
  struct ssACTIVE : sc::simple_state < ssACTIVE, vi_macros_fsm, ssIDLE > {;};

  struct ssEXPANDING_TEMPLATE : sc::state < ssEXPANDING_TEMPLATE, ssACTIVE >
  {
    typedef sc::transition< vi_macros_fsm::evDONE, ssIDLE > reactions;

    ssEXPANDING_TEMPLATE(my_context ctx) : my_base ( ctx )
    {
      log::verbose("vi macro") << "expanding template";
      context< vi_macros_fsm >().state(vi_macros_fsm::EXPANDING_TEMPLATE);
      context< vi_macros_fsm >().expanding_template();
      post_event(vi_macros_fsm::evDONE());
    };
  };

  struct ssIDLE : sc::state< ssIDLE, ssACTIVE >
  {
    typedef mpl::list<
      sc::custom_reaction< vi_macros_fsm::evEXPAND_TEMPLATE >,
      sc::transition< vi_macros_fsm::evRECORD, ssRECORDING > > reactions;

    ssIDLE(my_context ctx) : my_base ( ctx )
    {
      context< vi_macros_fsm >().state(vi_macros_fsm::IDLE);
    }

    sc::result react( const vi_macros_fsm::evEXPAND_TEMPLATE &)
    {
      return
        !context< vi_macros_fsm >().get_variable().get_name().empty() &&
        !context< vi_macros_fsm >().get_variable().get_value().empty() &&
         context< vi_macros_fsm >().get_variable().is_template() ?
        transit< ssEXPANDING_TEMPLATE >(): forward_event();
     }
  };

  struct ssRECORDING : sc::state < ssRECORDING, ssACTIVE > 
  {
    typedef sc::custom_reaction< vi_macros_fsm::evRECORD > reactions;

    ssRECORDING(my_context ctx) : my_base ( ctx )
    {
      log::verbose("vi macro") << "recording" << vi_macros::get_macro();
      log::status(_("Macro recording"));
      
      context< vi_macros_fsm >().state(vi_macros_fsm::RECORDING);
      context< vi_macros_fsm >().start_recording();
    };

    sc::result react( const vi_macros_fsm::evRECORD &) 
    {
      if (!vi_macros::get(vi_macros::get_macro()).empty())
      {
        vi_macros::save_macro(vi_macros::get_macro());
        log::status("Macro") << vi_macros::get_macro() << "is recorded";
      }
      else
      {
        log::verbose("Macro") << vi_macros::get_macro() << "not recorded";
        vi_macros::erase();
        log::status(std::string());
      }

      return transit< ssIDLE >();
    };
  };
}

wex::vi_macros_fsm::vi_macros_fsm()
{
  initiate();
}

void wex::vi_macros_fsm::expand_variable(const std::string& name, ex* ex)
{
  m_ex = ex;
  
  if (expanding_variable(name, nullptr) && m_state != RECORDING)
  {
    vi_macros::set_macro(name);
  }
}

void wex::vi_macros_fsm::expanding_template()
{
  // Read the file (file name is in variable value), expand
  // all macro variables in it, and set expanded to the result.
  const path filename(config().dir(), m_variable.get_value());

  std::ifstream ifs(filename.data());
  
  if (!ifs.is_open())
  {
    log::verbose("could not open template file") << filename;
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
        if (std::string value; !expanding_variable(variable, &value))
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
    
  vi_macros::m_macro = m_variable.get_name();
  frame::statustext(vi_macros::m_macro, "PaneMacro");
    
  log::status(_("Macro expanded"));
}

bool wex::vi_macros_fsm::expanding_variable(
  const std::string& name, std::string* value) const
{
  pugi::xml_node node;
  variable* var;
    
  if (const auto& it = vi_macros::m_variables->find(name);
    it == vi_macros::m_variables->end())
  {
    const auto& itn = vi_macros::m_variables->insert({name, variable(name)});
    var = &itn.first->second;
    node = vi_macros::m_doc->document_element().append_child("variable");
  }
  else
  {
    var = &it->second;

    try
    {
      // node = vi_macros::m_doc.document_element().child(name.c_str());
      const std::string query("//variable[@name='" + name + "']");

      if (auto xp = vi_macros::m_doc->document_element().select_node(query.c_str());
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

void wex::vi_macros_fsm::playback(const std::string& macro, ex* ex, int repeat)
{
  if (repeat <= 0 || ex == nullptr)
  {
    return;
  }
  
  if (m_playback && macro == vi_macros::m_macro)
  {
    log("recursive playback") << macro;
    return;
  }
  
  wxBusyCursor wait;

  ex->get_stc()->BeginUndoAction();
  set_ask_for_input();
  m_playback = true;
  bool error = false;
  
  const auto& macro_commands((*vi_macros::m_macros)[macro]);

  for (int i = 0; i < repeat && !error; i++)
  {
    for (const auto& it : macro_commands)
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
    if (m_state != RECORDING)
    {
      vi_macros::m_macro = macro;
    }

    log::status(_("Macro played back"));
  }
  
  m_playback = false;
}

bool wex::vi_macros_fsm::process_expand(
  ex* ex, const variable& v, std::string& expanded)
{
  if (ex == nullptr)
  {
    return false;
  }
  
  m_error = false;
  m_ex = ex;
  m_expanded = &expanded;
  m_variable = v;

  process_event(evEXPAND_TEMPLATE());

  return !m_error;
}

bool wex::vi_macros_fsm::record(const std::string& macro, ex* ex)
{
  m_error = false;
  m_ex = ex;
  m_macro = macro;

  process_event(vi_macros_fsm::evRECORD());

  frame::statustext(vi_macros::m_macro, "PaneMacro");
  frame::statustext(state(), "PaneMode");

  if (m_ex != nullptr)
  {
    ((statusbar *)m_ex->frame()->GetStatusBar())->show_field(
      "PaneMode", 
      m_state != IDLE && config(_("Show mode")).get(false));
  }

  return !m_error;
}

void wex::vi_macros_fsm::set_ask_for_input() const
{
  for (auto& it : *vi_macros::m_variables)
  {
    it.second.set_ask_for_input();
  }
}

void wex::vi_macros_fsm::start_recording()
{
  vi_macros::m_is_modified = true;
  
  if (m_macro.size() == 1)
  {
    // We only use lower case macro's, to be able to
    // append to them using.
    vi_macros::m_macro = m_macro;
    std::transform(vi_macros::m_macro.begin(), vi_macros::m_macro.end(), vi_macros::m_macro.begin(), ::tolower);
  
    // Clear macro if it is lower case
    // (otherwise append to the macro).
    if (islower(m_macro[0]))
    {
      (*vi_macros::m_macros)[vi_macros::m_macro].clear();
    }
  }
  else
  {
    vi_macros::m_macro = m_macro;
    (*vi_macros::m_macros)[vi_macros::m_macro].clear();
  }
}
