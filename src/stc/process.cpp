////////////////////////////////////////////////////////////////////////////////
// Name:      process.cpp
// Purpose:   Implementation of class wex::process
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/algorithm/string.hpp>
#include <wex/core/log.h>
#include <wex/stc/process.h>
#include <wex/stc/shell.h>
#include <wex/ui/debug-entry.h>
#include <wex/ui/frame.h>
#include <wex/ui/item-dialog.h>
#include <wex/vi/ex-stream.h>
#include <wx/valtext.h>

#include <algorithm>

/* NOLINTNEXTLINE */
std::string wex::process::m_working_dir_key = _("Process folder");

wex::process::process()
  : m_frame(dynamic_cast<frame*>(wxTheApp->GetTopWindow()))
{
}

wex::process::~process() {}

wex::process::process(const process& process)
{
  *this = process;
}

wex::process& wex::process::operator=(const process& p)
{
  if (this != &p)
  {
    m_frame = p.m_frame;

    if (m_frame == nullptr)
    {
      m_frame = dynamic_cast<frame*>(wxTheApp->GetTopWindow());
    }
  }

  return *this;
}

bool wex::process::async_system(
  const std::string& exe_in,
  const std::string& start_dir)
{
  auto cwd(start_dir);
  auto exe(exe_in);

  if (exe.empty())
  {
    if (config(_("Process")).get_first_of().empty())
    {
      if (config_dialog() == wxID_CANCEL)
      {
        return false;
      }
    }

    cwd = config(m_working_dir_key).get_first_of();
    exe = config(_("Process")).get_first_of();
  }
  else
  {
    if (auto* stc = dynamic_cast<wex::stc*>(m_frame->get_stc()); stc != nullptr)
    {
      if (exe.find("%LINES") != std::string::npos)
      {
        if (!stc->is_visual())
        {
          boost::algorithm::replace_all(
            exe,
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
            exe,
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
            exe,
            "%LINES",
            std::to_string(stc->get_current_line() + 1) + "," +
              std::to_string(stc->get_current_line() + 1));
        }
      }
    }
  }

  // We need a shell for output.
  if (m_shell == nullptr)
  {
    log("execute") << "no shell";
    return false;
  }

  m_shell->set_process(this);
  path::current(path(cwd));

  if (
    m_frame->debug_entry() != nullptr &&
    !m_frame->debug_entry()->name().empty() &&
    m_frame->debug_entry()->name() == before(get_exe(), ' '))
  {
    set_handler_dbg(m_frame->debug_handler());
    m_shell->get_lexer().set(m_frame->debug_entry()->name());
  }
  else
  {
    m_shell->SetFocus();
    m_frame->show_process(true);
  }

  return factory::process::async_system(exe, cwd);
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
  if ((!get_stdout().empty() || !get_stderr().empty()) && m_shell != nullptr)
  {
    m_frame->show_process(true);
    m_shell->AppendText(!get_stdout().empty() ? get_stdout() : get_stderr());
  }
}

bool wex::process::write(const std::string& text)
{
  if (!is_debug())
  {
    m_frame->show_process(true);
  }

  return factory::process::write(text);
}
