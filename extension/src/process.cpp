////////////////////////////////////////////////////////////////////////////////
// Name:      process.cpp
// Purpose:   Implementation of class wex::process
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <vector>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/process.h>
#include <wx/timer.h>
#include <wx/txtstrm.h> // for wxTextInputStream
#include <wex/process.h>
#include <wex/config.h>
#include <wex/debug.h>
#include <wex/itemdlg.h>
#include <wex/log.h>
#include <wex/managedframe.h>
#include <wex/shell.h>
#include <wex/util.h>
#include <process.hpp>

#define GET_STREAM(SCOPE)                         \
{                                                 \
  if (Is##SCOPE##Available())                     \
  {                                               \
    wxTextInputStream tis(*Get##SCOPE##Stream()); \
                                                  \
    while (Is##SCOPE##Available())                \
    {                                             \
      const char c = tis.GetChar();               \
                                                  \
      if (c != 0)                                 \
      {                                           \
        text += c;                                \
      }                                           \
    }                                             \
  }                                               \
};                                                \

namespace wex
{
  // an async process
  class process_imp : public wxProcess
  {
  public:
    process_imp(process* process)
      : wxProcess(wxPROCESS_REDIRECT) 
      , m_process(process)
      , m_debug(process->get_command_executed() == "gdb")
      , m_timer(std::make_unique<wxTimer>(this)) {
      Bind(wxEVT_TIMER, [=](wxTimerEvent& event) {read();});};
    virtual ~process_imp() {
      m_timer.reset();};

    bool execute(const std::string& path);
    bool kill(int sig);
    void read();
    void write(const std::string& text);
  private:
    virtual void OnTerminate(int pid, int status) override {
      m_timer->Stop();
      read();
      m_process->is_finished(pid);};
    
    const bool m_debug;
    std::string m_stdin;
    process* m_process;
    std::unique_ptr<wxTimer> m_timer;
    wxCriticalSection m_critical;
  };

  bool process_run_and_collect_output(
    const std::string& command, 
    const std::string& cwd,
    std::string* out,
    std::string* err)
  {
    log::verbose("process exec wait:", 1) << command;

    out->clear();
    err->clear();

#ifdef __WXGTK__
    TinyProcessLib::Process process(command, cwd,
      [&](const char *bytes, size_t n) {out->append(bytes, n);},
      [&](const char *bytes, size_t n) {err->append(bytes, n);});
    
    auto exit_status = process.get_exit_status();
    
    return exit_status != 0;
#else
    wxArrayString output;
    wxArrayString errors;
    struct wxExecuteEnv env;
    env.cwd = cwd;

    if (wxExecute(command, output, errors, wxEXEC_SYNC, &env) == -1)
    {
      return false;
    }
    else
    {
      // Set output by converting array strings into normal strings.
      *out = wxJoin(output, '\n', '\n');
      *err = wxJoin(errors, '\n', '\n');
      return true;
    }
#endif
  }
      
  auto show_process(wex::managed_frame* frame, bool show)
  {
    if (frame != nullptr)
    {
      frame->show_pane("PROCESS", show);
      return true;
    }

    return false;  
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
    case EXEC_WAIT:
      error = process_run_and_collect_output(m_command, cwd, &m_stdout, &m_stderr);
    break;

    case EXEC_NO_WAIT:
      // We need a shell for output.
      if (m_shell == nullptr) return false;
    
      m_shell->enable(true);
      m_shell->set_process(this);
      m_shell->SetName(m_command);
      m_shell->set_prompt(
        // a unix shell itself has no prompt, so put one here
        m_command.find("bash") == 0 ||
        m_command.find("csh") == 0 ||
        m_command.find("ksh") == 0 ||
        m_command.find("tcsh") == 0 ||
        m_command.find("sh") == 0 ? ">" : "");
      
      m_process = std::make_unique<process_imp>(this);

      if (!m_process->execute(cwd))
      {
        m_process.reset();
        error = true;
      }
    break;
  }
  
  return !error;
}

void wex::process::is_finished(int pid)
{
  log::verbose("process", 1) << pid << "exit";
  
  m_frame->get_debug()->breakpoints().clear();
  m_process.reset();
}
  
bool wex::process::is_running() const
{
  return m_process != nullptr && wxProcess::Exists(m_process->GetPid());
}

bool wex::process::kill(int sig)
{
  bool killed = false;

  if (m_process != nullptr)
  {
    killed = m_process->Kill(sig);

    if ((sig == wxSIGKILL || sig == wxSIGTERM) && killed)
    {
      m_process.reset();
      show_process(m_frame, false);
    }
  }

  return killed;
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

bool wex::process::write(const std::string& text)
{
  if (!is_running()) 
  {
    log::status("Process is not running");
    return false;
  }
  
  m_process->write(text);
  
  if (!is_running())
  {
    show_process(m_frame, false);
  }

  return true;
}

// Implementation.

bool wex::process_imp::execute(const std::string& path)
{
  struct wxExecuteEnv env;
  env.cwd = path;
  
  if (wxExecute(m_process->get_command_executed(), wxEXEC_ASYNC, this, &env) <= 0) 
  {
    return false;
  }
  
  log::verbose("process", 1) << GetPid() << "exec no wait:" << m_process->get_command_executed();

  show_process(m_process->get_frame(), true);
  m_timer->Start(100); // milliseconds
  m_process->get_shell()->SetFocus();
  
  return true;
}

bool wex::process_imp::kill(int sig)
{
  if (const auto pid = GetPid(); wxProcess::Kill(pid, (wxSignal)sig) != wxKILL_OK)
  {
    return false;
  }

  return true;
}

void wex::process_imp::read()
{
  wxCriticalSectionLocker lock(m_critical);
  
  std::string text;
  GET_STREAM(Input);
  GET_STREAM(Error);
  
  if (!text.empty())
  {
    m_process->get_shell()->AppendText(
      // prevent echo of last input
      !m_stdin.empty() && text.find(m_stdin) == 0 ?
        text.substr(m_stdin.length()):
        text);
    
    if (m_debug && m_process->get_frame() != nullptr)
    {
      m_process->get_frame()->get_debug()->process_stdout(text);
    }
  }
    
  if (!m_stdin.empty())
  {
    m_stdin.clear();
    m_process->get_shell()->prompt(std::string(), false);
  }
}

void handle_command(const std::string& command)
{
  const std::string cd = 
#ifdef __WXMSW__
      "chdir";
#else
      "cd";
#endif        

  if (command.find(cd) == 0)
  {
    wxLogNull logNo;
    if (const auto rest (wex::skip_white_space(wex::after(command, cd.back())));
      rest.empty() || rest == "~")
    {
#ifdef __WXMSW__
#else        
      wxSetWorkingDirectory(wxGetHomeDir());
#endif        
    }
    else
    {
      wxSetWorkingDirectory(rest);
    }
  }
}

void wex::process_imp::write(const std::string& text)
{
  m_timer->Stop();
  
  if (m_process->get_command_executed().find("cmd") == 0 ||
      m_process->get_command_executed().find("powershell") == 0)
  {
    m_process->get_shell()->DocumentEnd();
  }
    
  // Write text to process and restart timer.
  if (wxOutputStream* os = GetOutputStream(); os != nullptr)
  {
    handle_command(text);

    if (m_debug && m_process->get_frame() != nullptr)
    {
      m_process->get_frame()->get_debug()->process_stdin(text);
    }

    const auto el = (text.size() == 1 && text[0] == 3 ? 
      std::string(): std::string("\n"));

    wxTextOutputStream(*os).WriteString(text + el);
    
    log::verbose("process") << GetPid() << "write:" << text;
    
    m_stdin = text;
    wxMilliSleep(10);

    read();
    m_timer->Start();
  }
}
