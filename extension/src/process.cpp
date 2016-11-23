////////////////////////////////////////////////////////////////////////////////
// Name:      process.cpp
// Purpose:   Implementation of class wxExProcess
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

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

#define DEBUG(SCOPE)                                       \
{                                                          \
  wxExManagedFrame* frame = dynamic_cast<                  \
    wxExManagedFrame*>(wxTheApp->GetTopWindow());          \
  if (frame != nullptr &&                                  \
      frame->GetDebug() != nullptr &&                      \
      frame->GetDebug()->GetProcess() == this)             \
  {                                                        \
    frame->GetDebug()->Process##SCOPE(text);               \
  }                                                        \
};

#define GET_STREAM(SCOPE)                                    \
{                                                            \
  if (m_Process->Is##SCOPE##Available())                     \
  {                                                          \
    wxTextInputStream tis(*m_Process->Get##SCOPE##Stream()); \
                                                             \
    while (m_Process->Is##SCOPE##Available())                \
    {                                                        \
      const char c = tis.GetChar();                          \
                                                             \
      if (c != 0)                                            \
      {                                                      \
        text += c;                                           \
      }                                                      \
    }                                                        \
  }                                                          \
};                                                           \

class wxExProcessImp : public wxProcess
{
public:
  wxExProcessImp() : 
    wxProcess(wxPROCESS_REDIRECT) {;};
protected:
  virtual void OnTerminate(int pid, int status) override {;};
};

void HandleCommand(const wxString& command)
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

bool ShowProcess(bool show, wxTimer* timer = nullptr)
{
  if (!show && timer != nullptr)
  {
    timer->Stop();
  }

  wxExManagedFrame* frame = (dynamic_cast<wxExManagedFrame*>(wxTheApp->GetTopWindow()));

  if (frame != nullptr)
  {
    frame->ShowPane("PROCESS", show);
    return true;
  }

  return false;  
}
      
wxExShell* wxExProcess::m_Shell = nullptr;
wxString wxExProcess::m_WorkingDirKey = _("Process folder");

wxExProcess::wxExProcess()
  : m_Process(new wxExProcessImp())
  , m_Timer(std::make_unique<wxTimer>(m_Process))
  , m_Error(false)
  , m_Command(wxExConfigFirstOf(_("Process")))
{
  m_Process->Bind(wxEVT_TIMER, [=](wxTimerEvent& event) {CheckInput();});
}

wxExProcess::~wxExProcess()
{
  delete m_Process;
}
  
wxExProcess::wxExProcess(const wxExProcess& process)
{
  *this = process;
}

wxExProcess& wxExProcess::operator=(const wxExProcess& p)
{
  if (this != &p)
  {
    m_Process = new wxExProcessImp();
    m_Timer = std::make_unique<wxTimer>(m_Process);
    m_Command = p.m_Command;
    m_Error = p.m_Error;
    m_StdIn = p.m_StdIn;
    m_StdErr = p.m_StdErr;
    m_StdOut = p.m_StdOut;
  }

  return *this;
}

void wxExProcess::CheckInput()
{
  if (m_Shell == nullptr) return;
  
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
    
    DEBUG(StdOut);
  }
    
  if (!m_StdIn.empty())
  {
    m_StdIn.clear();
    m_Shell->Prompt(std::string(), false);
  }
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
  const std::string& command_to_execute,
  int flags,
  const std::string& wd)
{
  m_Error = false;
    
  struct wxExecuteEnv env;
    
  if (command_to_execute.empty())
  {
    if (wxExConfigFirstOf(_("Process")).empty())
    {
      if (!wd.empty())
      {
        wxLogStatus("Ignored specified working directory");
      }
      
      if (ConfigDialog(wxTheApp->GetTopWindow()) == wxID_CANCEL)
      {
        return false;
      }
    }
    
    m_Command = wxExConfigFirstOf(_("Process"));
    env.cwd = wxExConfigFirstOf(m_WorkingDirKey);
  }
  else
  {
    m_Command = command_to_execute;
    env.cwd = wd;
  }
  
  const bool sync = (flags & wxEXEC_SYNC);
  
  if (!sync)
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
    
    // For asynchronous execution the return value is the process id and zero 
    // value indicates that the command could not be executed.
    if (wxExecute(m_Command, flags, m_Process, &env) > 0)
    {
      if (!env.cwd.empty())
      {
        wxFileName fn(env.cwd);
        fn.Normalize();
        wxSetWorkingDirectory(fn.GetFullPath());
      }
      
      m_Timer->Start(100); // milliseconds
      m_Shell->SetFocus();
    }
    else
    {
      m_Error = true;
    }
  }
  else
  {
    wxArrayString output;
    wxArrayString errors;
    
    // Call wxExecute to execute the command and
    // collect the output and the errors.
    if (wxExecute(
      m_Command,
      output,
      errors,
      flags,
      &env) == -1)
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
  return 
    // If we have not yet run Execute, process is not running
    m_Shell != nullptr &&
    m_Process->GetPid() > 0 && wxProcess::Exists(m_Process->GetPid());
}

bool wxExProcess::Kill()
{
  if (!IsRunning())
  {
    return false;
  }

  wxSignal sig = wxSIGKILL;
  
  const wxKillError result = m_Process->Kill(m_Process->GetPid(), sig);
  
  m_Process->DeletePendingEvents();
  ShowProcess(false, m_Timer.get());
  
  switch (result)
  {
    case wxKILL_OK: break;
    case wxKILL_BAD_SIGNAL:    wxLogStatus("no such signal"); break;
    case wxKILL_ACCESS_DENIED: wxLogStatus("permission denied"); break;
    case wxKILL_NO_PROCESS:    wxLogStatus("no such process"); break;
    case wxKILL_ERROR:         wxLogStatus("another, unspecified error"); break;
    default: wxFAIL;
  }
  
  return result == wxKILL_OK;
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
    if (m_Shell != nullptr && ShowProcess(true))
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
  
  m_Timer->Stop();
  
  if (m_Command.find("cmd") == 0 ||
      m_Command.find("powershell") == 0)
  {
    m_Shell->DocumentEnd();
  }
    
  // Send text to process and restart timer.
  wxOutputStream* os = m_Process->GetOutputStream();

  if (os != nullptr)
  {
    HandleCommand(text);
    DEBUG(StdIn);
    const std::string el = (text.size() == 1 && text[0] == 3 ? std::string(): std::string("\n"));
    wxTextOutputStream(*os).WriteString(text + el);
    m_StdIn = text;
    wxMilliSleep(10);
    CheckInput();
    m_Timer->Start();
  }

  if (!IsRunning())
  {
    ShowProcess(false, m_Timer.get());
  }

  return true;
}
