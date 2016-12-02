////////////////////////////////////////////////////////////////////////////////
// Name:      process.cpp
// Purpose:   Implementation of class wxExProcess
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

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
#include <wx/extension/defs.h>
#include <wx/extension/itemdlg.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/shell.h>
#include <wx/extension/util.h> // for wxExConfigFirstOf

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
  bool Kill();
  static int KillAll();
  void Read();
  bool Write(const std::string& text);
private:
  void HandleCommand(const wxString& command);
  virtual void OnTerminate(int pid, int status) override {
    const auto it = find (m_pids.begin(), m_pids.end(), pid);
    if (it != m_pids.end())
    {
      m_pids.erase(it);
    }
    m_Timer->Stop();
    Read();};
  
  const bool m_Debug;
  std::string m_Command, m_StdIn;
  wxExManagedFrame* m_Frame;
  wxExShell* m_Shell;
  std::unique_ptr<wxTimer> m_Timer;
  wxCriticalSection m_Critical;
  static std::vector<long> m_pids;
};

bool ShowProcess(wxExManagedFrame* frame, bool show)
{
  if (frame != nullptr)
  {
    frame->ShowPane("PROCESS", show);
    return true;
  }

  return false;  
}
      
std::vector<long> wxExProcessImp::m_pids;

wxExShell* wxExProcess::m_Shell = nullptr;
wxString wxExProcess::m_WorkingDirKey = _("Process folder");

wxExProcess::wxExProcess() 
  : m_Command(wxExConfigFirstOf(_("Process")))
  , m_Frame(dynamic_cast<wxExManagedFrame*>(wxTheApp->GetTopWindow()))
{
  wxASSERT(m_Frame != nullptr);
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

int wxExProcess::ConfigDialog(
  wxWindow* parent,
  const wxString& title,
  bool modal)
{
  wxExItem ci(_("Process"), ITEM_COMBOBOX, wxAny(), true);
    
  wxTextValidator validator(wxFILTER_EXCLUDE_CHAR_LIST);
  validator.SetCharExcludes("?%*\"");
  ci.SetValidator(&validator);
  
  const std::vector<wxExItem> v {
    ci,
    {m_WorkingDirKey, ITEM_COMBOBOX_DIR, wxAny(), true, wxWindow::NewControlId()}};

  if (modal)
  {
    return wxExItemDialog(parent, v, title).ShowModal();
  }
  else
  {
    wxExItemDialog* dlg = new wxExItemDialog(parent, v, title);
    return dlg->Show();
  }
}

bool wxExProcess::Execute(
  const std::string& command,
  bool wait,
  const std::string& wd)
{
  m_Error = false;
    
  std::string cwd(wd);
    
  if (command.empty())
  {
    if (wxExConfigFirstOf(_("Process")).empty())
    {
      if (ConfigDialog(wxTheApp->GetTopWindow()) == wxID_CANCEL)
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

  if (!wait)
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

    if (!m_Process->Execute(m_Command, wd))
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
    env.cwd = wd;
    
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

bool wxExProcess::Kill()
{
  bool killed = false;

  if (m_Process != nullptr)
  {
    killed = m_Process->Kill();
    m_Process.release();
  }
  
  ShowProcess(m_Frame, false);

  return killed;
}

int wxExProcess::KillAll()
{
  return wxExProcessImp::KillAll();
}

void wxExProcess::PrepareOutput(wxWindow* parent)
{
  if (m_Shell == nullptr)
  {
    m_Shell = new wxExShell(parent, 
      std::string(), 
      std::string(), 
      true, 
      100);
  }
}

#if wxUSE_GUI
void wxExProcess::ShowOutput(const wxString& caption) const
{
  if (!m_Error)
  {
    if (m_Shell != nullptr && ShowProcess(m_Frame, true))
    {
      m_Shell->AppendText(m_StdOut);
    }
    else
    {
      std::cout << m_StdOut << "\n";
    }
  }
  else
  {
    // Executing command failed, so no output,
    // show failing command.
    wxLogError("Could not execute: %s", m_Command.c_str());
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
  
  if (wxExecute(command, wxEXEC_ASYNC, this, &env) <= 0) return false;
  
  m_pids.push_back(GetPid());

  if (!env.cwd.empty())
  {
    wxFileName fn(env.cwd);
    fn.Normalize();
    wxSetWorkingDirectory(fn.GetFullPath());
  }
  
  ShowProcess(m_Frame, true);
  m_Timer->Start(100); // milliseconds
  m_Shell->SetFocus();
  
  return true;
}

void wxExProcessImp::HandleCommand(const wxString& command)
{
  wxString rest;
  
  if (
         command.StartsWith("cd", &rest)
#ifdef __WXMSW__
      || command.StartsWith("chdir", &rest)
#endif        
     )
  {
    wxLogNull logNo;
    rest.Trim(false);
    rest.Trim(true);
    
    if (rest.empty() || rest == "~")
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

bool wxExProcessImp::Kill()
{
  long pid = GetPid();

  if (wxProcess::Kill(pid) != wxKILL_OK)
  {
    return false;
  }
  
  const auto it = find (m_pids.begin(), m_pids.end(), pid);

  if (it != m_pids.end())
  {
    m_pids.erase(it);
  }

  return true;
}

int wxExProcessImp::KillAll()
{
  int killed = 0;
  
  for (auto pid : m_pids)
  {
    if (wxProcess::Kill(pid, wxSIGKILL) == wxKILL_OK)
    {
      killed++;
    }
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
    
    if (m_Debug)
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
  wxOutputStream* os = GetOutputStream();

  if (os != nullptr)
  {
    HandleCommand(text);

    if (m_Debug)
    {
      m_Frame->GetDebug()->ProcessStdIn(text);
    }

    const std::string el = (text.size() == 1 && text[0] == 3 ? std::string(): std::string("\n"));
    wxTextOutputStream(*os).WriteString(text + el);
    m_StdIn = text;
    wxMilliSleep(10);
    Read();
    m_Timer->Start();
  }

  return true;
}
