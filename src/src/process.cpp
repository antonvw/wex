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
#if BOOST_VERSION / 100 % 1000 <= 65
  #include <boost/asio.hpp>
#endif
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
#if BOOST_VERSION / 100 % 1000 <= 65
      , m_io(std::make_shared<boost::asio::io_service>())
#else    
      , m_io(std::make_shared<boost::asio::io_context>())
#endif    
      , m_queue(std::make_shared<std::queue<std::string>>()) {;}

    // Starts the async process, collecting output
    // into the stc shell of the parent process.
    bool async(const std::string& path);
    
    // Stops the process.
    bool stop() {
      if (m_io->stopped()) return false;
      log::verbose("stop") << m_process->get_exec();
      try
      {
        if (m_group.valid())
        {
          m_group.terminate();
        }
        m_io->stop();
      }
      catch (std::exception& e)
      {
        log(e) << "stop" << m_process->get_exec();
      }
      return true;};
    
    // Writes data to the input of the process.
    bool write(const std::string& text) {
      if (!is_debug())
      {
        show_process(m_process->get_frame(), true);
      }
    
      if (is_running())
      {
        m_queue->push(text);
      }
      return true;};

    bool is_debug() const {return m_debug.load();};
    bool is_running() const {return !m_io->stopped();};
  private:
    std::atomic_bool m_debug {false};
#if BOOST_VERSION / 100 % 1000 <= 65
    std::shared_ptr<boost::asio::io_service> m_io;
#else
    std::shared_ptr<boost::asio::io_context> m_io;
#endif
    std::shared_ptr<std::queue<std::string>> m_queue;
    process* m_process;
    bp::ipstream m_es, m_is;
    bp::opstream m_os;
    bp::group m_group;
  };

  int process_run_and_collect_output(
    const std::string& command, 
    const std::string& cwd,
    std::string& output,
    std::string& error)
  {
    try
    {
      std::future<std::string> of, ef;
      const int ec = bp::system(
        bp::start_dir = cwd, 
        command, 
        bp::std_out > of, 
        bp::std_err > ef);

      if (of.valid()) output = of.get();
      if (ef.valid()) error = ef.get();

      if (!ec)
      {
        log::verbose("process", 1) << command << "cwd:" << cwd;
      }
      else
      {
        const std::string text(!error.empty() ? ":" + error: std::string());
        log("bp::system") << command << "cwd:" << cwd << "ec:" << ec << text;
      }
      
      return ec;
    }
    catch (std::exception& e)
    {
      log(e) << command << "cwd:" << cwd;

      output.clear();
      error = e.what();
      return 1;
    }
  }
}
      
std::string wex::process::m_working_dir_key = _("Process folder");

wex::process::process() 
  : m_command(config(_("Process")).get_firstof())
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
    window_data(par).title(_("Select Process")));
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
    if (config(_("Process")).get_firstof().empty())
    {
      if (config_dialog() == wxID_CANCEL)
      {
        return false;
      }
    }
    
    m_command = config(_("Process")).get_firstof();
    cwd = config(m_working_dir_key).get_firstof();
  }
  else
  {
    m_command = command;

    if (auto* stc = m_frame->get_stc(); stc != nullptr)
    {
      if (command.find("%LINES") != std::string::npos)
      {
        if (const std::string sel(stc->GetSelectedText());
          !sel.empty())
        {
          replace_all(m_command, "%LINES", 
            std::to_string(
              stc->LineFromPosition(stc->GetSelectionStart()) + 1) + "," +
            std::to_string(
              stc->LineFromPosition(stc->GetSelectionEnd()) + 1));
        }
        else
        {
          replace_all(m_command, "%LINES", 
            std::to_string(stc->GetCurrentLine() + 1) + "," +
            std::to_string(stc->GetCurrentLine() + 1));
        }
      }
    }
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
      path::current(cwd);
      
      m_process = std::make_unique<process_imp>(this);

      if (!m_process->async(cwd))
      {
        m_process.reset();
        error = true;
      }
    break;

    case EXEC_WAIT:
      error = (process_run_and_collect_output(
        m_command, cwd, m_stdout, m_stderr) != 0);
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

bool wex::process::stop()
{
  return m_process != nullptr && m_process->stop();
}

bool wex::process::write(const std::string& text)
{
  return m_process != nullptr && m_process->write(text);
}

// Implementation.

#define WEX_POST(ID, TEXT, PROCESS)                         \
  if (PROCESS != nullptr)                                   \
  {                                                         \
    wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, ID);  \
    event.SetString(TEXT);                                  \
    wxPostEvent(PROCESS, event);                            \
  }

bool wex::process_imp::async(const std::string& path)
{
  try
  {
    bp::async_system(
      *m_io.get(),
      [&](boost::system::error_code error, int i) {
        log::verbose("async", 1) << "exit:" << error.message();
        if (m_debug.load())
        {
          WEX_POST(ID_DEBUG_EXIT, "", m_process->get_frame()->get_debug())
        }},
      bp::start_dir = path,
      m_process->get_exec(),
      bp::std_out > m_is,
      bp::std_in < m_os,
      bp::std_err > m_es,
      m_group);
  }
  catch (std::exception& e)
  {
    log(e) << m_process->get_exec() << "path:" << path;
    return false;
  }

  log::verbose("bp::async_system", 1) << m_process->get_exec();

  m_debug.store(m_process->get_frame()->get_debug()->debug_entry().name()
    == before(m_process->get_exec(), ' '));
  
  if (m_debug.load())
  {
    m_process->get_shell()->get_lexer().set(
      m_process->get_frame()->get_debug()->debug_entry().name());
  }
  else
  {
    m_process->get_shell()->SetFocus();
    show_process(m_process->get_frame(), true);
  }
  
  std::thread t([
    debug = m_debug.load() && m_process->get_frame() != nullptr,
    process = m_process, 
    &is = m_is] 
    {
      std::string text, line;
      line.reserve(1000000);
      text.reserve(1000000);
      int linesize = 0;
      bool error = false;

      while (is.good() && !error)
      {
        text.push_back(is.get());
        linesize++;
        
        if (linesize > 10000)
        {
          error = true;
          WEX_POST(ID_SHELL_APPEND, "\n*** LINE LIMIT ***\n", process->get_shell())
        }
        else if (isspace(text.back()) && !process->get_frame()->is_closing())
        {
          WEX_POST(ID_SHELL_APPEND, text, process->get_shell())

          if (text.back() == '\n')
          {
            linesize = 0;
          }
          
          if (debug)
          {
            line.append(text);

            if (line.back() == '\n')
            {
              WEX_POST(ID_DEBUG_STDOUT, line, process->get_frame()->get_debug())
              line.clear();
            }
          }

          text.clear();
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
      while (os.good() && !io->stopped())
      {
#if BOOST_VERSION / 100 % 1000 > 65
        io->run_one_for(std::chrono::milliseconds(10));
#else        
        io->run_one();
#endif        

        if (!queue->empty())
        {
          const std::string text(queue->front());
          queue->pop();
          
          log::verbose("async") << "write:" << text;

          os << text << std::endl;

          if (debug && process->get_frame() != nullptr)
          {
            WEX_POST(ID_DEBUG_STDIN, text, process->get_frame()->get_debug())
          }
        }
      }
    });
  u.detach();

  std::thread v([process = m_process, &es = m_es] 
  {
    std::string text;

    while (es.good())
    {
      text.push_back(es.get());
            
      if (text.back() == '\n')
      {
        WEX_POST(ID_SHELL_APPEND_ERROR, text, process->get_shell())
        text.clear();
      }
    }
  });
  v.detach();

  return true;
}
