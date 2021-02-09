////////////////////////////////////////////////////////////////////////////////
// Name:      process.cpp
// Purpose:   Implementation of class wex::process
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "process-imp.h"

#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <vector>
#include <wex/config.h>
#include <wex/ex-stream.h>
#include <wex/item-dialog.h>
#include <wex/log.h>
#include <wex/managed-frame.h>
#include <wex/process.h>
#include <wex/shell.h>
#include <wx/valtext.h>

std::string wex::process::m_working_dir_key = _("Process folder");

wex::process::process()
  : m_command(config(_("Process")).get_first_of())
  , m_frame(dynamic_cast<managed_frame*>(wxTheApp->GetTopWindow()))
{
}

wex::process::~process()
{
  if (m_process != nullptr)
  {
    m_process->stop();
  }
}

wex::process::process(const process& process)
{
  *this = process;
}

wex::process& wex::process::operator=(const process& p)
{
  if (this != &p)
  {
    m_command = p.m_command;
    m_stderr  = p.m_stderr;
    m_stdout  = p.m_stdout;
  }

  return *this;
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
  else
  {
    return item_dialog(v, data).ShowModal();
  }
}

bool wex::process::execute(
  const std::string& command,
  exec_t             type,
  const std::string& wd)
{
  auto cwd(wd);

  if (command.empty())
  {
    if (config(_("Process")).get_first_of().empty())
    {
      if (config_dialog() == wxID_CANCEL)
      {
        return false;
      }
    }

    m_command = config(_("Process")).get_first_of();
    cwd       = config(m_working_dir_key).get_first_of();
  }
  else
  {
    m_command = command;

    if (auto* stc = m_frame->get_stc(); stc != nullptr)
    {
      if (command.find("%LINES") != std::string::npos)
      {
        if (!stc->is_visual())
        {
          boost::algorithm::replace_all(
            m_command,
            "%LINES",
            std::to_string(std::max(
              (size_t)1,
              (size_t)stc->get_current_line() + 1 -
                std::min(
                  (size_t)stc->GetLineCount(),
                  stc->get_file().ex_stream()->get_context_lines()))) +
              "," + std::to_string(stc->get_current_line() + 1));
        }
        else if (const std::string sel(stc->GetSelectedText()); !sel.empty())
        {
          boost::algorithm::replace_all(
            m_command,
            "%LINES",
            std::to_string(
              stc->LineFromPosition(stc->GetSelectionStart()) + 1) +
              "," +
              std::to_string(
                stc->LineFromPosition(stc->GetSelectionEnd()) + 1));
        }
        else
        {
          boost::algorithm::replace_all(
            m_command,
            "%LINES",
            std::to_string(stc->get_current_line() + 1) + "," +
              std::to_string(stc->get_current_line() + 1));
        }
      }
    }
  }

  bool error = false;

  switch (type)
  {
    case EXEC_NO_WAIT:
      // We need a shell for output.
      if (m_shell == nullptr)
      {
        log("execute") << "no shell";
        return false;
      }

      m_shell->enable(true);
      m_shell->set_process(this);
      m_shell->SetName(m_command);
      path::current(cwd);

      m_process = std::make_unique<process_imp>(this);

      if (!m_process->async(cwd))
      {
        m_process.reset();
        error = true;
      }
      break;

    case EXEC_WAIT:
      error =
        (process_run_and_collect_output(m_command, cwd, m_stdout, m_stderr) !=
         0);
      break;
  }

  return !error;
}

bool wex::process::is_debug() const
{
  return m_process != nullptr && m_process->is_debug();
}

bool wex::process::is_running() const
{
  return m_process != nullptr && m_process->is_running();
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

void wex::process::show_output(const std::string& caption) const
{
  if ((!m_stdout.empty() || !m_stderr.empty()) && m_shell != nullptr)
  {
    m_frame->show_process(true);
    m_shell->AppendText(!m_stdout.empty() ? m_stdout : m_stderr);
  }
}

bool wex::process::stop()
{
  return m_process != nullptr && m_process->stop();
}

bool wex::process::write(const std::string& text)
{
  return m_process != nullptr && m_process->write(text);
}
