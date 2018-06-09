////////////////////////////////////////////////////////////////////////////////
// Name:      process.cpp
// Purpose:   Implementation of class wxExProcess
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <vector>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/process.h>
#include <wx/timer.h>
#include <wx/txtstrm.h> // for wxTextInputStream
#include <wx/extension/process.h>
#include <wx/extension/debug.h>
#include <wx/extension/itemdlg.h>
#include <wx/extension/log.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/shell.h>
#include <wx/extension/util.h> // for wxExConfigFirstOf
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

class wxExProcessImp : public wxProcess
{
public:
  wxExProcessImp(wxExManagedFrame* frame, wxExShell* shell, bool debug)
    : wxProcess(wxPROCESS_REDIRECT) 
    , m_Debug(debug)
    , m_Frame(frame)
    , m_Shell(shell)
    , m_Timer(std::make_unique<wxTimer>(this)) {
    Bind(wxEVT_TIMER, [=](wxTimerEvent& event) {Read();});};
  virtual ~wxExProcessImp() {;};

  bool Execute(const std::string& command, const std::string& path);
  bool Kill(int sig);
  static int KillAll(int sig);
  void Read();
  bool Write(const std::string& text);
private:
  void HandleCommand(const std::string& command);
  virtual void OnTerminate(int pid, int status) override {
    if (const auto& it = std::find(m_pids.begin(), m_pids.end(), pid);
      it != m_pids.end())
    {
      m_pids.erase(it);
    }
    m_Timer->Stop();
    m_Frame->GetDebug()->GetBreakpoints().clear();
    Read();};
  
  const bool m_Debug;
  std::string m_Command, m_StdIn;
  wxExManagedFrame* m_Frame;
  wxExShell* m_Shell;
  std::unique_ptr<wxTimer> m_Timer;
  wxCriticalSection m_Critical;
  static inline std::vector<int> m_pids;
};

auto ShowProcess(wxExManagedFrame* frame, bool show)
{
  if (frame != nullptr)
  {
    frame->ShowPane("PROCESS", show);
    return true;
  }

  return false;  
}
      
wxString wxExProcess::m_WorkingDirKey = _("Process folder");

wxExProcess::wxExProcess() 
  : m_Command(wxExConfigFirstOf(_("Process")))
  , m_Frame(dynamic_cast<wxExManagedFrame*>(wxTheApp->GetTopWindow()))
{
}

wxExProcess::~wxExProcess()
{
}
  
wxExProcess::wxExProcess(const wxExProcess& process)
{
  *this = process;
}
  
wxExProcess& wxExProcess::operator=(const wxExProcess& p)
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

int wxExProcess::ConfigDialog(const wxExWindowData& par)
{
  wxTextValidator validator(wxFILTER_EXCLUDE_CHAR_LIST);
  validator.SetCharExcludes("?%*\"");
  const wxExWindowData data(wxExWindowData(par).Title(_("Select Process").ToStdString()));
  const std::vector<wxExItem> v {
    {_("Process"), ITEM_COMBOBOX, std::any(), wxExControlData().Validator(&validator).Required(true)},
    {m_WorkingDirKey, ITEM_COMBOBOX_DIR, std::any(), wxExControlData().Required(true)}};

  if (data.Button() & wxAPPLY)
  {
    wxExItemDialog* dlg = new wxExItemDialog(v, data);
    return dlg->Show();
  }
  else
  {
    return wxExItemDialog(v, data).ShowModal();
  }
}

bool wxExProcess::Execute(
  const std::string& command,
  long flags,
  const std::string& wd)
{
  m_Error = false;
    
  auto cwd(wd);
    
  if (command.empty())
  {
    if (wxExConfigFirstOf(_("Process")).empty())
    {
      if (ConfigDialog() == wxID_CANCEL)
      {
        return false;
      }
    }
    
    m_Command = wxExConfigFirstOf(_("Process"));
    cwd = wxExConfigFirstOf(m_WorkingDirKey);
  }
  else
  {
    m_Command = command;
  }

  if (!(flags & PROCESS_EXEC_WAIT))
  { 
    // We need a shell for output.
    if (m_Shell == nullptr) return false;
  
    m_Shell->EnableShell(true);
    m_Shell->SetProcess(this);
    m_Shell->SetName(m_Command);
    m_Shell->SetPrompt(
      // a unix shell itself has no prompt, so put one here
      m_Command.find("bash") == 0 ||
      m_Command.find("csh") == 0 ||
      m_Command.find("ksh") == 0 ||
      m_Command.find("tcsh") == 0 ||
      m_Command.find("sh") == 0 ? ">" : "");
    
    m_Process = std::make_unique<wxExProcessImp>(m_Frame, m_Shell, command == "gdb");

    if (!m_Process->Execute(m_Command, cwd))
    {
      m_Process.release();
      m_Error = true;
    }
  }
  else
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
      m_Shell->EnableShell(false);
    }
  }
  
  return !m_Error;
}

bool wxExProcess::IsRunning() const
{
  return m_Process != nullptr && wxProcess::Exists(m_Process->GetPid());
}

bool wxExProcess::Kill(int sig)
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

int wxExProcess::KillAll(int sig)
{
  return wxExProcessImp::KillAll(sig);
}

void wxExProcess::PrepareOutput(wxWindow* parent)
{
  if (m_Shell == nullptr)
  {
    m_Shell = new wxExShell(
      wxExSTCData().Window(wxExWindowData().Parent(parent)),
      std::string()); // empty prompt
  }
}

#if wxUSE_GUI
void wxExProcess::ShowOutput(const std::string& caption) const
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
    wxExLog() << "could not execute:" <<  m_Command;
  }
}
#endif

bool wxExProcess::Write(const std::string& text)
{
  if (!IsRunning()) 
  {
    wxLogStatus("Process is not running");
    return false;
  }
  
  m_Process->Write(text);
  
  if (!IsRunning())
  {
    ShowProcess(m_Frame, false);
  }

  return true;
}

// Implementation.

bool wxExProcessImp::Execute(
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

void wxExProcessImp::HandleCommand(const std::string& command)
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
    if (const auto rest (wxExSkipWhiteSpace(wxExAfter(command, cd.back())));
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

bool wxExProcessImp::Kill(int sig)
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

int wxExProcessImp::KillAll(int sig)
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
    wxExLog() << "could not kill all processes";
  }
 
  return killed;
}

void wxExProcessImp::Read()
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
      m_Frame->GetDebug()->ProcessStdOut(text);
    }
  }
    
  if (!m_StdIn.empty())
  {
    m_StdIn.clear();
    m_Shell->Prompt(std::string(), false);
  }
}

bool wxExProcessImp::Write(const std::string& text)
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
      m_Frame->GetDebug()->ProcessStdIn(text);
    }

    const auto el = (text.size() == 1 && text[0] == 3 ? 
      std::string(): std::string("\n"));
    wxTextOutputStream(*os).WriteString(text + el);
    m_StdIn = text;
    wxMilliSleep(10);
    Read();
    m_Timer->Start();
  }

  return true;
}
