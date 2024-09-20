////////////////////////////////////////////////////////////////////////////////
// Name:      process.cpp
// Purpose:   Implementation of class wex::process
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/config.h>
#include <wex/core/core.h>
#include <wex/core/log.h>
#include <wex/stc/shell.h>
#include <wex/ui/debug-entry.h>
#include <wex/ui/frame.h>
#include <wex/ui/item-dialog.h>
#include <wex/vcs/process.h>
#include <wx/valtext.h>

#include <algorithm>

#include "util.h"

/* NOLINTNEXTLINE */
std::string wex::process::m_working_dir_key = _("Process folder");

wex::process::process()
  : m_frame(dynamic_cast<frame*>(wxTheApp->GetTopWindow()))
{
}

bool wex::process::async_system(const process_data& data_in)
{
  set_frame();
  process_data data(data_in);

  if (data.exe().empty())
  {
    if (config(_("Process")).get_first_of().empty())
    {
      if (config_dialog() == wxID_CANCEL)
      {
        return false;
      }
    }

    data.start_dir(config(m_working_dir_key).get_first_of())
      .exe(config(_("Process")).get_first_of());
  }
  else
  {
    if (auto* stc = dynamic_cast<wex::stc*>(m_frame->get_stc()); stc != nullptr)
    {
      expand_macro(data, stc);
    }
  }

  // We need a shell for output.
  if (m_shell == nullptr)
  {
    log("async_system") << "no shell";
    return false;
  }

  m_shell->set_process(this);

  if (
    m_frame->debug_entry() != nullptr &&
    !m_frame->debug_entry()->name().empty() &&
    m_frame->debug_entry()->name() == find_before(data.exe(), " "))
  {
    log::debug("async_system debug handler") << m_frame->debug_entry()->name();
    set_handler_dbg(m_frame->debug_handler());
    m_shell->get_lexer().set(m_frame->debug_entry()->name());
  }
  else
  {
    m_shell->SetFocus();
    m_frame->show_process(true);
  }

  return factory::process::async_system(data);
}

int wex::process::config_dialog(const data::window& par)
{
  wxTextValidator validator(wxFILTER_EXCLUDE_CHAR_LIST);
  validator.SetCharExcludes("?%*\"");
  const data::window      data(data::window(par).title(_("Select Process")));
  const std::vector<item> v{
    {_("Process"),
     item::COMBOBOX,
     std::any(),
     data::control().validator(&validator).is_required(true)},
    {m_working_dir_key,
     item::COMBOBOX_DIR,
     std::any(),
     data::control().is_required(true)}};

  if (data.button() & wxAPPLY)
  {
    auto* dlg = new item_dialog(v, data);
    return dlg->Show();
  }

  return item_dialog(v, data).ShowModal();
}

wex::shell* wex::process::prepare_output(wxWindow* parent)
{
  if (m_shell == nullptr)
  {
    m_shell = new shell(
      data::stc().window(data::window().parent(parent)),
      std::string()); // empty prompt
  }

  return m_shell;
}

void wex::process::set_frame()
{
  if (m_frame == nullptr)
  {
    m_frame = dynamic_cast<frame*>(wxTheApp->GetTopWindow());
  }
}

void wex::process::show_output(const std::string& caption) const
{
  if (
    (!std_out().empty() || !std_err().empty()) && m_shell != nullptr &&
    m_frame != nullptr)
  {
    m_frame->show_process(true);
    m_shell->AppendText(!std_out().empty() ? std_out() : std_err());
  }
}

int wex::process::system(const process_data& data_in)
{
  process_data data(data_in);
  set_frame();

  if (auto* stc = dynamic_cast<wex::stc*>(m_frame->get_stc()); stc != nullptr)
  {
    expand_macro(data, stc);
  }

  return factory::process::system(data);
}

bool wex::process::write(const std::string& text)
{
  if (!is_debug() && m_frame != nullptr)
  {
    m_frame->show_process(true);
  }

  return factory::process::write(text);
}
