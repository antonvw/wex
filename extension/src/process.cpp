////////////////////////////////////////////////////////////////////////////////
// Name:      process.cpp
// Purpose:   Implementation of class wxExProcess
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/timer.h>
#include <wx/txtstrm.h> // for wxTextInputStream
#include <wx/extension/process.h>
#include <wx/extension/itemdlg.h>
#include <wx/extension/defs.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/shell.h>
#include <wx/extension/util.h> // for wxExConfigFirstOf

#define GET_STREAM(SCOPE)                          \
{                                                  \
  if (Is##SCOPE##Available())                      \
  {                                                \
    wxTextInputStream tis(*Get##SCOPE##Stream());  \
                                                   \
    while (Is##SCOPE##Available())                 \
    {                                              \
      const wxChar c = tis.GetChar();              \
                                                   \
      if (c != 0)                                  \
      {                                            \
        output << c;                               \
      }                                            \
    }                                              \
  }                                                \
};                                                 \

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
  : wxProcess(wxPROCESS_REDIRECT)
  , m_Timer(new wxTimer(this))
  , m_Error(false)
  , m_HasStdError(false)
  , m_Sync(false)
{
  m_Command = wxExConfigFirstOf(_("Process"));
  
  Bind(wxEVT_TIMER, [=](wxTimerEvent& event) {CheckInput();});
}

wxExProcess::~wxExProcess()
{
  delete m_Timer;
}

wxExProcess::wxExProcess(const wxExProcess& process)
  : m_Timer(nullptr)
{
  *this = process;
}

wxExProcess& wxExProcess::operator=(const wxExProcess& p)
{
  if (this != &p)
  {
    delete m_Timer;
    
    m_Timer = new wxTimer(this);
    
    m_Command = p.m_Command;
    m_Error = p.m_Error;
    m_HasStdError = p.m_HasStdError;
    m_Input = p.m_Input;
    m_Output = p.m_Output;
    m_Sync = p.m_Sync;
  }

  return *this;
}

void wxExProcess::CheckInput()
{
  wxCriticalSectionLocker lock(m_Critical);
  
  if (m_Shell == nullptr)
  {
    return;
  }
  
  wxString output;
  GET_STREAM(Input);
  GET_STREAM(Error);
  
  if (!output.empty())
  {
    if (!m_Input.empty() && output.StartsWith(m_Input))
    {
      // prevent echo of last input
      m_Shell->AppendText(output.substr(m_Input.length()));
    }
    else
    {
      m_Shell->AppendText(output);
    }
  }
    
  if (!m_Input.empty())
  {
    m_Input.clear();
    m_Shell->Prompt(wxEmptyString, false);
  }
}

bool wxExProcess::Command(const wxString& command)
{
  if (!IsRunning()) 
  {
    wxLogStatus("Process is not running");
    return false;
  }
  
  m_Timer->Stop();
  
  if (
     !m_Command.StartsWith("cmd") && 
     !m_Command.StartsWith("powershell"))
  {
    if (command.empty())
    {
      m_Shell->Prompt(wxEmptyString, true);
    }
    else
    {
      m_Shell->AppendText(m_Shell->GetEOL());
    }
  }
  else
  {
    m_Shell->DocumentEnd();
  }
    
  // Send command to process and restart timer.
  wxOutputStream* os = GetOutputStream();

  if (os != nullptr)
  {
    HandleCommand(command);
    wxTextOutputStream(*os).WriteString(command + "\n");
    m_Input = command;
    wxMilliSleep(10);
    CheckInput();
    m_Timer->Start();
  }

  if (!IsRunning())
  {
    ShowProcess(false, m_Timer);
  }

  return true;
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
    wxExItem(m_WorkingDirKey, ITEM_COMBOBOX_DIR, wxAny(), true, wxWindow::NewControlId())};

  if (modal)
  {
    return wxExItemDialog(parent,
      v,
      title).ShowModal();
  }
  else
  {
    wxExItemDialog* dlg = new wxExItemDialog(
      parent,
      v,
      title);
      
    return dlg->Show();
  }
}

bool wxExProcess::Execute(
  const wxString& command_to_execute,
  int flags,
  const wxString& wd)
{
  // We need a shell for output.
  if (m_Shell == nullptr)
  {
    return false;
  }
  
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
  
  m_Sync = (flags & wxEXEC_SYNC);

  m_Shell->EnableShell(!m_Sync);
  m_Shell->SetProcess(this);
  
  m_HasStdError = false;
  
  if (!m_Sync)
  { 
    m_Shell->SetName(m_Command);
    m_Shell->SetPrompt(
      // a unix shell itself has no prompt, s put one here
      m_Command.StartsWith("bash") ||
      m_Command.StartsWith("csh") ||
      m_Command.StartsWith("ksh") ||
      m_Command.StartsWith("tcsh") ||
      m_Command.StartsWith("sh") ? ">" : "");
    
    // For asynchronous execution the return value is the process id and zero 
    // value indicates that the command could not be executed.
    if (wxExecute(m_Command, flags, this, &env) > 0)
    {
      if (!env.cwd.empty())
      {
        wxFileName fn(env.cwd);
        fn.Normalize();
        wxSetWorkingDirectory(fn.GetFullPath());
      }
      
      if (IsRunning())
      {
        m_Timer->Start(100); // milliseconds
        m_Shell->SetFocus();
      }
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
      m_Output.clear();
      m_Error = true;
    }
    else
    {
      m_HasStdError = !errors.empty();
      
      // Set output by converting array strings into normal strings.
      m_Output = wxJoin(errors, '\n', '\n') + wxJoin(output, '\n', '\n');
    }
  }
  
  return !m_Error;
}

bool wxExProcess::IsRunning() const
{
  if (
    // If we executed a sync process, then it always ended,
    // so it is not running.
    m_Sync || 
    // If we have not yet run Execute, process is not running
    m_Shell == nullptr ||
    GetPid() <= 0)
  {
    return false;
  }

  return Exists(GetPid());
}

wxKillError wxExProcess::Kill(wxSignal sig)
{
  if (!IsRunning())
  {
    return wxKILL_NO_PROCESS;
  }
  
  const wxKillError result = wxProcess::Kill(GetPid(), sig);
  
  DeletePendingEvents();
  ShowProcess(false, m_Timer);
  
  switch (result)
  {
    case wxKILL_OK: break;
    case wxKILL_BAD_SIGNAL:    wxLogStatus("no such signal"); break;
    case wxKILL_ACCESS_DENIED: wxLogStatus("permission denied"); break;
    case wxKILL_NO_PROCESS:    wxLogStatus("no such process"); break;
    case wxKILL_ERROR:         wxLogStatus("another, unspecified error"); break;
    default: wxFAIL;
  }
  
  return result;
}

void wxExProcess::OnTerminate(int pid, int status)
{
  m_Timer->Stop();
  CheckInput();
  wxLogStatus(_("Ready"));

  if (m_Shell != nullptr)
  {
    m_Shell->EnableShell(false);
  }
}

void wxExProcess::PrepareOutput(wxWindow* parent)
{
  if (m_Shell == nullptr)
  {
    m_Shell = new wxExShell(parent, wxEmptyString, wxEmptyString, true, 25, wxEmptyString,
      wxExSTC::STC_MENU_DEFAULT | wxExSTC::STC_MENU_OPEN_LINK);
  }
}

#if wxUSE_GUI
void wxExProcess::ShowOutput(const wxString& caption) const
{
  if (!m_Sync)
  {
    wxLogStatus("Output only available for sync processes");
  }  
  else if (!m_Error)
  {
    if (m_Shell != nullptr && ShowProcess(true))
    {
      m_Shell->AppendText(m_Output);
    }
    else if (!m_Output.empty())
    {
      wxLogMessage(m_Output);
    }
    else
    {
      wxLogStatus("No output available");
    }
  }
  else
  {
    // Executing command failed, so no output,
    // show failing command.
    wxLogError("Could not execute: " + m_Command);
  }
}
#endif
