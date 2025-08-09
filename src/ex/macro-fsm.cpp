////////////////////////////////////////////////////////////////////////////////
// Name:      macro-fsm.cpp
// Purpose:   Implementation of class wex::macro_fsm
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/mpl/list.hpp>
#include <boost/statechart/custom_reaction.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <pugixml.hpp>
#include <wex/core/config.h>
#include <wex/core/log.h>
#include <wex/ex/ex.h>
#include <wex/ex/macro-mode.h>
#include <wex/ex/macros.h>
#include <wex/ex/variable.h>
#include <wex/factory/stc-undo.h>
#include <wex/syntax/stc.h>
#include <wex/ui/frame.h>
#include <wex/ui/statusbar.h>

#include "macro-fsm.h"

#include <fstream>
#include <utility>

namespace mpl = boost::mpl;

namespace wex
{
struct ssmACTIVE : sc::simple_state<ssmACTIVE, macro_fsm, ssmIDLE>
{
  ;
};

struct ssmIDLE : sc::state<ssmIDLE, ssmACTIVE>
{
  using reactions = sc::transition<macro_fsm::evRECORD, ssmRECORDING>;

  explicit ssmIDLE(my_context ctx)
    : my_base(std::move(ctx))
  {
    context<macro_fsm>().state(macro_fsm::IDLE);
  }
};

struct ssmRECORDING : sc::state<ssmRECORDING, ssmACTIVE>
{
  using reactions = sc::custom_reaction<macro_fsm::evRECORD>;

  explicit ssmRECORDING(my_context ctx)
    : my_base(std::move(ctx))
  {
    context<macro_fsm>().state(macro_fsm::RECORDING);
  };

  sc::result react(const macro_fsm::evRECORD&)
  {
    context<macro_fsm>().recorded();
    return transit<ssmIDLE>();
  };
};
} // namespace wex

wex::macro_fsm::macro_fsm(macro_mode* mode)
  : m_mode(mode)
{
  initiate();
}

bool wex::macro_fsm::expand_template(
  const variable& var,
  ex*             ex,
  std::string&    expanded)
{
  if (
    ex == nullptr || var.get_name().empty() || var.get_value().empty() ||
    !var.is_template())
  {
    if (!var.is_template() && !var.get_name().empty())
    {
      log("variable") << var.get_name() << "is not a template";
    }

    return false;
  }

  // Read the file (file name is in variable value), expand
  // all macro variables in it, and set expanded to the result.
  const path filename(
    m_mode->get_macros()->path().data().parent_path(),
    var.get_value());

  std::ifstream ifs(filename.data());

  if (!ifs.is_open())
  {
    if (std::filesystem::is_symlink(filename.data()))
    {
      ifs.open(std::filesystem::read_symlink(filename.data()));
    }

    if (!ifs.is_open())
    {
      log("could not open template file") << filename.data().string();
      return false;
    }
  }

  set_ask_for_input();

  bool error = false;

  for (char c; ifs.get(c) && !error;)
  {
    if (c != '@')
    {
      expanded += c;
    }
    else
    {
      if (const auto& exp(read_variable(ifs, '@', ex, var)); !exp.empty())
      {
        expanded += exp;
      }
      else
      {
        error = true;
      }
    }
  }

  // Set back to normal value.
  set_ask_for_input();

  m_macro = var.get_name();
  ex->frame()->statustext(m_macro, "PaneMacro");

  log::status(_("Macro expanded"));

  return !error;
}

bool wex::macro_fsm::expand_variable(const std::string& name, ex* ex) const
{
  return expanding_variable(ex, name, nullptr);
}

bool wex::macro_fsm::expanding_variable(
  ex*                ex,
  const std::string& name,
  std::string*       value) const
{
  pugi::xml_node node;
  variable*      var;

  if (auto it = m_mode->get_macros()->m_variables.find(name);
      it == m_mode->get_macros()->m_variables.end())
  {
    const auto& itn =
      m_mode->get_macros()->m_variables.insert({name, variable(name)});
    var = &itn.first->second;
    node =
      m_mode->get_macros()->m_doc.document_element().append_child("variable");
  }
  else
  {
    var = &it->second;

    try
    {
      const std::string query("//variable[@name='" + name + "']");

      if (auto xp = m_mode->get_macros()->m_doc.document_element().select_node(
            query.c_str());
          xp && xp.node())
      {
        node = xp.node();
      }
      else
      {
        log("xml query failed") << query;
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

void wex::macro_fsm::playback(const std::string& macro, ex* ex, size_t repeat)
{
  if (ex == nullptr)
  {
    log("playback requires ex") << macro;
    return;
  }

  if ((m_playback || m_state == RECORDING) && macro == m_macro)
  {
    log("recursive playback") << macro;
    return;
  }

  stc_undo* undo = nullptr;

  if (!m_playback)
  {
    undo = new stc_undo(ex->get_stc());
  }

  set_ask_for_input();
  m_playback = true;
  bool error = false;

  if (m_state == IDLE)
  {
    m_macro = macro;
  }

  // E.g. otherwise Coverage macro not ok.
  ex->reset_search_flags();

  const auto& commands(m_mode->get_macros()->get_macro_commands(macro));

  for (size_t i = 0; i < repeat && !error; i++)
  {
    if (!std::ranges::all_of(
          commands,
          [ex](const auto& i)
          {
            if (!ex->command(i))
            {
              log::status(_("Macro aborted at")) << i;
              return false;
            }
            return true;
          }))
    {
      error = true;
    }
  }

  if (!error)
  {
    log::status(_("Macro played back"));
  }

  if (undo)
  {
    delete undo;
  }

  m_playback = false;
}

std::string wex::macro_fsm::read_variable(
  std::ifstream&  ifs,
  const char      separator,
  ex*             ex,
  const variable& current)
{
  char        c;
  std::string variable;
  bool        completed = false;

  while (!completed && ifs.get(c))
  {
    switch (c)
    {
      case '\n':
      case '\r':
        break;

      default:
        if (c == separator)
        {
          completed = true;
        }
        // other separator
        else if (c == '\'')
        {
          if (const auto& exp(read_variable(ifs, c, ex, current)); !exp.empty())
          {
            variable::set_argument(exp);
          }
          else
          {
            return std::string();
          }
        }
        else
        {
          variable += c;
        }
    }
  }

  if (!completed)
  {
    log("variable syntax error") << variable;
  }
  // Prevent recursion.
  else if (variable == current.get_name())
  {
    log("recursive variable") << variable;
  }
  else
  {
    if (std::string value; expanding_variable(ex, variable, &value))
    {
      if (value.empty())
      {
        log::trace(variable) << "is empty";
        value = "?";
      }

      return value;
    }
  }

  return std::string();
}

void wex::macro_fsm::record(const std::string& macro, ex* ex)
{
  // an empty macro is used to stop recording
  if (!macro.empty())
  {
    m_macro = macro;
  }

  process_event(macro_fsm::evRECORD());

  if (ex != nullptr)
  {
    ex->frame()->statustext(m_macro, "PaneMacro");
    ex->frame()->statustext(str(), "PaneMode");
    ex->frame()->get_statusbar()->pane_show(
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
    log::trace("macro") << m_macro << "is recorded";
  }
  else
  {
    log::debug("macro") << m_macro << "not recorded";
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
    log::trace("vi macro") << "recording" << m_macro;
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
      std::ranges::transform(m_macro, m_macro.begin(), ::tolower);
    }
    else
    {
      m_mode->get_macros()->m_macros[m_macro].clear();
    }
  }
}
