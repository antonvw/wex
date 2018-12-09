////////////////////////////////////////////////////////////////////////////////
// Name:      process.cpp
// Purpose:   Implementation of class wex::process
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
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
#include <easylogging++.h>

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
  class process_imp : public wxProcess
  {
  public:
    process_imp(managed_frame* frame, shell* shell, bool debug)
      : wxProcess(wxPROCESS_REDIRECT) 
      , m_Debug(debug)
      , m_Frame(frame)
      , m_Shell(shell)
      , m_Timer(std::make_unique<wxTimer>(this)) {
      Bind(wxEVT_TIMER, [=](wxTimerEvent& event) {read();});};
    virtual ~process_imp() {;};

    bool execute(const std::string& command, const std::string& path);
    bool kill(int sig);
    static int kill_all(int sig);
    void read();
    void write(const std::string& text);
  private:
    void HandleCommand(const std::string& command);
    virtual void OnTerminate(int pid, int status) override {
      if (const auto& it = std::find(m_pids.begin(), m_pids.end(), pid);
        it != m_pids.end())
      {
        m_pids.erase(it);
      }
      m_Timer->Stop();
      m_Frame->get_debug()->breakpoints().clear();
      read();};
    
    const bool m_Debug;
    std::string m_Command, m_StdIn;
    managed_frame* m_Frame;
    shell* m_Shell;
    std::unique_ptr<wxTimer> m_Timer;
    wxCriticalSection m_Critical;
    static inline std::vector<int> m_pids;
  };
};

auto ShowProcess(wex::managed_frame* frame, bool show)
{
  if (frame != nullptr)
  {
    frame->show_pane("PROCESS", show);
    return true;
  }

  return false;  
}
      
std::string wex::process::m_WorkingDirKey = _("Process folder");

wex::process::process() 
  : m_Command(config(_("Process")).firstof())
  , m_Frame(dynamic_cast<managed_frame*>(wxTheApp->GetTopWindow()))
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
    m_Command = p.m_Command;
    m_Error = p.m_Error;
    m_StdErr = p.m_StdErr;
    m_StdOut = p.m_StdOut;
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
    {m_WorkingDirKey, item::COMBOBOX_DIR, std::any(), 
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
  int type,
  const std::string& wd)
{
  m_Error = false;
    
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
    
    m_Command = config(_("Process")).firstof();
    cwd = config(m_WorkingDirKey).firstof();
  }
  else
  {
    m_Command = command;
  }

  switch (type)
  {
    case EXEC_WAIT:
    {
      wxArrayString output;
      wxArrayString errors;
      struct wxExecuteEnv env;
      env.cwd = cwd;

      VLOG(1) << "exec: " << m_Command;

      if (wxExecute(m_Command, output, errors, wxEXEC_SYNC, &env) == -1)
      {
        m_StdErr.clear();
        m_StdOut.clear();
        m_Error = true;
      }
      else
      {
        // Set output by converting array strings into normal strings.
        m_StdOut = wxJoin(output, '\n', '\n');
        m_StdErr = wxJoin(errors, '\n', '\n');
      }

      if (m_Shell != nullptr)
      {
        m_Shell->enable(false);
      }
    }
    break;

    case EXEC_DEFAULT:
      // We need a shell for output.
      if (m_Shell == nullptr) return false;
    
      m_Shell->enable(true);
      m_Shell->set_process(this);
      m_Shell->SetName(m_Command);
      m_Shell->set_prompt(
        // a unix shell itself has no prompt, so put one here
        m_Command.find("bash") == 0 ||
        m_Command.find("csh") == 0 ||
        m_Command.find("ksh") == 0 ||
        m_Command.find("tcsh") == 0 ||
        m_Command.find("sh") == 0 ? ">" : "");
      
      m_Process = std::make_unique<process_imp>(m_Frame, m_Shell, command == "gdb");

      if (!m_Process->execute(m_Command, cwd))
      {
        m_Process.release();
        m_Error = true;
      }
    break;
  }
  
  return !m_Error;
}

bool wex::process::is_running() const
{
  return m_Process != nullptr && wxProcess::Exists(m_Process->GetPid());
}

bool wex::process::kill(int sig)
{
  bool killed = false;

  if (m_Process != nullptr)
  {
    killed = m_Process->Kill(sig);

    if ((sig == wxSIGKILL || sig == wxSIGTERM) && killed)
    {
      m_Process.release();

      ShowProcess(m_Frame, false);
    }
  }

  return killed;
}

int wex::process::kill_all(int sig)
{
  return process_imp::kill_all(sig);
}

void wex::process::prepare_output(wxWindow* parent)
{
  if (m_Shell == nullptr)
  {
    m_Shell = new shell(
      stc_data().window(window_data().parent(parent)),
      std::string()); // empty prompt
  }
}

void wex::process::show_output(const std::string& caption) const
{
  if (!m_Error)
  {
    if (m_Shell != nullptr && ShowProcess(m_Frame, true))
    {
      m_Shell->AppendText(!m_StdOut.empty() ? m_StdOut: m_StdErr);
    }

    VLOG(2) << m_StdOut;
  }
  else
  {
    log() << "could not execute:" <<  m_Command;
  }
}

bool wex::process::write(const std::string& text)
{
  if (!is_running()) 
  {
    log_status("Process is not running");
    return false;
  }
  
  m_Process->write(text);
  
  if (!is_running())
  {
    ShowProcess(m_Frame, false);
  }

  return true;
}

// Implementation.

bool wex::process_imp::execute(
  const std::string& command, const std::string& path)
{
  struct wxExecuteEnv env;
  env.cwd = path;
  m_Command = command;
  
  VLOG(1) << "exec: " << command;

  if (wxExecute(command, wxEXEC_ASYNC, this, &env) <= 0) 
  {
    return false;
  }
  
  m_pids.push_back(GetPid());
  
  ShowProcess(m_Frame, true);
  m_Timer->Start(100); // milliseconds
  m_Shell->SetFocus();
  
  return true;
}

void wex::process_imp::HandleCommand(const std::string& command)
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
    if (const auto rest (skip_white_space(after(command, cd.back())));
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

bool wex::process_imp::kill(int sig)
{
  if (const auto pid = GetPid(); wxProcess::Kill(pid, (wxSignal)sig) != wxKILL_OK)
  {
    return false;
  }
  else if (sig == wxSIGKILL || sig == wxSIGTERM)
  {
    if (const auto& it = std::find(m_pids.begin(), m_pids.end(), pid);
      it != m_pids.end())
    {
      m_pids.erase(it);
    }
  }

  return true;
}

int wex::process_imp::kill_all(int sig)
{
  int killed = 0;
  
  for (auto pid : m_pids)
  {
    if (wxProcess::Kill(pid, (wxSignal)sig) == wxKILL_OK)
    {
      killed++;
    }
  }
  
  if (killed != m_pids.size())
  {
    log() << "could not kill all processes";
  }
 
  return killed;
}

void wex::process_imp::read()
{
  wxCriticalSectionLocker lock(m_Critical);
  
  std::string text;
  GET_STREAM(Input);
  GET_STREAM(Error);
  
  if (!text.empty())
  {
    m_Shell->AppendText(
      // prevent echo of last input
      !m_StdIn.empty() && text.find(m_StdIn) == 0 ?
        text.substr(m_StdIn.length()):
        text);
    
    if (m_Debug && m_Frame != nullptr)
    {
      m_Frame->get_debug()->process_stdout(text);
    }
  }
    
  if (!m_StdIn.empty())
  {
    m_StdIn.clear();
    m_Shell->prompt(std::string(), false);
  }
}

void wex::process_imp::write(const std::string& text)
{
  m_Timer->Stop();
  
  if (m_Command.find("cmd") == 0 ||
      m_Command.find("powershell") == 0)
  {
    m_Shell->DocumentEnd();
  }
    
  // Write text to process and restart timer.
  if (wxOutputStream* os = GetOutputStream(); os != nullptr)
  {
    HandleCommand(text);

    if (m_Debug && m_Frame != nullptr)
    {
      m_Frame->get_debug()->process_stdin(text);
    }

    const auto el = (text.size() == 1 && text[0] == 3 ? 
      std::string(): std::string("\n"));

    wxTextOutputStream(*os).WriteString(text + el);
    
    VLOG(9) << "process write: " << text;
    
    m_StdIn = text;
    wxMilliSleep(10);

    read();
    m_Timer->Start();
  }
}
