////////////////////////////////////////////////////////////////////////////////
// Name:      process.cpp
// Purpose:   Implementation of class wex::process
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <atomic>
#include <queue>
#include <thread>
#include <vector>
#include <boost/process.hpp>
#include <wx/valtext.h>
#include <wex/process.h>
#include <wex/config.h>
#include <wex/debug.h>
#include <wex/itemdlg.h>
#include <wex/log.h>
#include <wex/managedframe.h>
#include <wex/shell.h>
#include <wex/util.h>

namespace bp = boost::process;

namespace wex
{
  auto show_process(wex::managed_frame* frame, bool show)
  {
    if (frame != nullptr)
    {
      frame->show_pane("PROCESS", show);
      return true;
    }

    return false;  
  };

  class process_imp
  {
  public:
    process_imp(process* process)
      : m_process(process)
      , m_io(std::make_shared<boost::asio::io_context>())
      , m_queue(std::make_shared<std::queue
          <std::pair<std::string, std::string*>>>()) {;}

    // Starts the async process, collecting output
    // into the stc shell of the parent process.
    bool async(const std::string& path);
    
    // Stops the process.
    bool stop() {
      if (m_io->stopped()) return false;
      log::verbose("stop") << m_process->get_exec();
      m_io->stop();
      return true;};
    
    // Writes data to the input of the process.
    // TODO: out is not yet used.
    bool write(const std::string& text, std::string* out) {
      if (is_running())
      {
        m_queue->push({text, out});
      }
      return true;};

    bool is_debug() const {return m_debug.load();};
    bool is_running() const {return !m_io->stopped();};
  private:
    std::atomic_bool m_debug {false};
    std::shared_ptr<boost::asio::io_context> m_io;
    std::shared_ptr<std::queue<std::pair<std::string, std::string*>>> m_queue;
    process* m_process;
    bp::ipstream m_es, m_is;
    bp::opstream m_os;
  };

  bool process_run_and_collect_output(
    const std::string& command, 
    const std::string& cwd,
    std::string& output,
    std::string& error)
  {
    try
    {
      std::future<std::string> of, ef;
      const auto ec = bp::system(
        bp::start_dir = cwd, command, bp::std_out > of, bp::std_err > ef);

      log::verbose("process", 1) << command << "cwd:" << cwd << "ec:" << ec;
    
      output = of.get();
      error = ef.get();
    }
    catch (std::exception& e)
    {
      log(e) << command << "cwd:" << cwd;

      output.clear();
      error = e.what();
    }

    return error.empty();
  }
}
      
std::string wex::process::m_working_dir_key = _("Process folder");

wex::process::process() 
  : m_command(config(_("Process")).firstof())
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
    m_stderr = p.m_stderr;
    m_stdout = p.m_stdout;
  }

  return *this;
}

int wex::process::config_dialog(const window_data& par)
{
  wxTextValidator validator(wxFILTER_EXCLUDE_CHAR_LIST);
  validator.SetCharExcludes("?%*\"");
  const window_data data(
    window_data(par).title(_("Select Process").ToStdString()));
  const std::vector<item> v {
    {_("Process"), item::COMBOBOX, std::any(), 
      control_data().validator(&validator).is_required(true)},
    {m_working_dir_key, item::COMBOBOX_DIR, std::any(), 
      control_data().is_required(true)}};

  if (data.button() & wxAPPLY)
  {
    auto * dlg = new item_dialog(v, data);
    return dlg->Show();
  }
  else
  {
    return item_dialog(v, data).ShowModal();
  }
}

bool wex::process::execute(
  const std::string& command,
  exec_t type,
  const std::string& wd)
{
  auto cwd(wd);
    
  if (command.empty())
  {
    if (config(_("Process")).firstof().empty())
    {
      if (config_dialog() == wxID_CANCEL)
      {
        return false;
      }
    }
    
    m_command = config(_("Process")).firstof();
    cwd = config(m_working_dir_key).firstof();
  }
  else
  {
    m_command = command;
  }
  
  bool error = false;

  switch (type)
  {
    case EXEC_NO_WAIT:
      // We need a shell for output.
      if (m_shell == nullptr) return false;
    
      m_shell->enable(true);
      m_shell->set_process(this);
      m_shell->SetName(m_command);
      
      m_process = std::make_unique<process_imp>(this);

      if (!m_process->async(cwd))
      {
        m_process.reset();
        error = true;
      }
    break;

    case EXEC_WAIT:
      error = !process_run_and_collect_output(m_command, cwd, m_stdout, m_stderr);
    break;
  }
  
  return !error;
}

bool wex::process::is_debug() const
{
  return m_process != nullptr && m_process->is_debug();
}
  
void wex::process::is_finished(int pid)
{
  if (!m_frame->is_closing())
  {
    m_frame->get_debug()->breakpoints().clear();
  }
}

bool wex::process::is_running() const
{
  return m_process != nullptr && m_process->is_running();
}

bool wex::process::kill(kill_t type)
{
  return m_process != nullptr && m_process->stop();
}

void wex::process::prepare_output(wxWindow* parent)
{
  if (m_shell == nullptr)
  {
    m_shell = new shell(
      stc_data().window(window_data().parent(parent)),
      std::string()); // empty prompt
  }
}

void wex::process::show_output(const std::string& caption) const
{
  if (
    (!m_stdout.empty() || !m_stderr.empty()) &&
      m_shell != nullptr && show_process(m_frame, true))
  {
    m_shell->AppendText(!m_stdout.empty() ? m_stdout: m_stderr);
  }
}

bool wex::process::write(const std::string& text, std::string* out)
{
  return m_process != nullptr && m_process->write(text, out);
}

// Implementation.

bool wex::process_imp::async(const std::string& path)
{
  try
  {
    bp::async_system(
      *m_io.get(),
      [&](boost::system::error_code error, int i) {
        log::verbose("async", 1) << "exit";
        m_process->is_finished(i);},
      bp::start_dir = path,
      m_process->get_exec(),
      bp::std_out > m_is,
      bp::std_in < m_os,
      bp::std_err > m_es);
  }
  catch (std::exception& e)
  {
    log(e) << m_process->get_exec() << "path:" << path;
    return false;
  }

  log::verbose("async", 1) << m_process->get_exec();

  show_process(m_process->get_frame(), true);
  m_process->get_shell()->SetFocus();

  m_debug.store(m_process->get_frame()->get_debug()->debug_entry().name()
    == before(m_process->get_exec(), ' '));
  
  std::thread t([
    debug = m_debug.load() && m_process->get_frame() != nullptr,
    process = m_process, 
    &is = m_is] 
    {
      while (is.good())
      {
        const std::string data(1, is.get());

        if (!data.empty() && !process->get_frame()->is_closing())
        {
          wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, ID_SHELL_APPEND);
          event.SetString(data);
          wxPostEvent(process->get_shell(), event);

          if (debug)
          {
            process->get_frame()->get_debug()->process_stdout(data);
          }
        }
      }
    });
  t.detach();

  std::thread u([
    debug = m_debug.load(), 
    io = m_io, 
    &os = m_os, 
    process = m_process, 
    queue = m_queue]
    {
      while (!io->stopped())
      {
        io->run_one_for(std::chrono::milliseconds(10));

        if (!queue->empty())
        {
          const std::string text(queue->front().first);
          queue->pop();

          log::verbose("async") << "write:" << text;

          os << text << std::endl;

          if (debug && process->get_frame() != nullptr)
          {
            process->get_frame()->get_debug()->process_stdin(text);
          }
        }
      }
    });
  u.detach();

  std::thread v([process = m_process, &es = m_es] 
  {
    std::string data;

    while (es.good())
    {
      data.append(std::string(1, es.get()));
    }

    if (!data.empty() && !process->get_frame()->is_closing())
    {
      wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, ID_SHELL_APPEND);
      event.SetString(data);
      wxPostEvent(process->get_shell(), event);
    }
  });
  v.detach();

  return true;
}
