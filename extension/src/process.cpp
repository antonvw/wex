////////////////////////////////////////////////////////////////////////////////
// Name:      process.cpp
// Purpose:   Implementation of class wxExProcess
// Author:    Anton van Wezenbeek
// Copyright: (c) 2012 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/txtstrm.h> // for wxTextInputStream
#include <wx/extension/process.h>
#include <wx/extension/configdlg.h>
#include <wx/extension/defs.h>
#include <wx/extension/shell.h>
#include <wx/extension/util.h> // for wxExConfigFirstOf

BEGIN_EVENT_TABLE(wxExProcess, wxProcess)
  EVT_MENU(ID_SHELL_COMMAND, wxExProcess::OnCommand)
  EVT_MENU(ID_SHELL_COMMAND_STOP, wxExProcess::OnCommand)
  EVT_TIMER(-1, wxExProcess::OnTimer)
END_EVENT_TABLE()

wxExSTCEntryDialog* wxExProcess::m_Dialog = NULL;
wxString wxExProcess::m_WorkingDirKey = _("Process folder");

wxExProcess::wxExProcess()
  : wxProcess(wxPROCESS_REDIRECT)
  , m_Timer(new wxTimer(this))
  , m_Busy(false)
  , m_Error(false)
  , m_Sync(false)
{
  m_Command = wxExConfigFirstOf(_("Process"));
}

wxExProcess::~wxExProcess()
{
  delete m_Timer;
}

wxExProcess::wxExProcess(const wxExProcess& process)
{
  *this = process;
}

wxExProcess& wxExProcess::operator=(const wxExProcess& p)
{
  m_Error = p.m_Error;
  m_Output = p.m_Output;
  m_Timer = new wxTimer(this);
  
  return *this;
}

void wxExProcess::CheckInput()
{
  if (m_Busy)
  {
    return;
  }
  
  m_Busy = true;
  
  wxString output;
  
  if (IsInputAvailable())
  {
    wxTextInputStream tis(*GetInputStream());
    
    while (IsInputAvailable())
    {
      const wxChar c = tis.GetChar();
      
      if (c != 0)
      {
        output << c;
      }
    }
  }
  
  if (IsErrorAvailable())
  {
    wxTextInputStream tis(*GetErrorStream());
    
    while (IsErrorAvailable())
    {
      const wxChar c = tis.GetChar();
      
      if (c != 0)
      {
        output << c;
      }
    }
  }

  m_Busy = false;
  
  if (!output.empty())
  {
    m_Dialog->GetSTCShell()->AddText(output);
    m_Dialog->GetSTCShell()->Prompt();
  }
}

int wxExProcess::ConfigDialog(
  wxWindow* parent,
  const wxString& title,
  bool modal)
{
  std::vector<wxExConfigItem> v;

  v.push_back(wxExConfigItem(
    _("Process"), 
    CONFIG_COMBOBOX, 
    wxEmptyString,
    true));

  v.push_back(wxExConfigItem(
    m_WorkingDirKey, 
    CONFIG_COMBOBOXDIR, 
    wxEmptyString,
    true,
    1000));

  if (modal)
  {
    return wxExConfigDialog(parent,
      v,
      title).ShowModal();
  }
  else
  {
    wxExConfigDialog* dlg = new wxExConfigDialog(
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
  m_Error = false;
    
  struct wxExecuteEnv env;
    
  if (command_to_execute.empty())
  {
    if (wxExConfigFirstOf(_("Process")).empty())
    {
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
  
  if (m_Dialog == NULL)
  {
    m_Dialog = new wxExSTCEntryDialog(
      wxTheApp->GetTopWindow(),
      m_Command,
      wxEmptyString,
      wxEmptyString,
      wxOK,
      true);
      
    m_Dialog->SetProcess(this);
    m_Dialog->GetSTCShell()->SetEventHandler(this);
  }
      
  m_Dialog->GetSTCShell()->EnableShell(!m_Sync);
    
  if (!m_Sync)
  { 
    m_Dialog->SetTitle(m_Command);
    m_Dialog->GetSTCShell()->ClearAll();
    m_Dialog->GetSTCShell()->Prompt();

    // For asynchronous execution the return value is the process id and zero 
    // value indicates that the command could not be executed.
    if (wxExecute(m_Command, flags, this, &env) > 0)
    {
      m_Dialog->Show();
      
      CheckInput();
      
      m_Timer->Start(100); // each 100 milliseconds
    }
    else
    {
      m_Dialog->Hide();
      
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
  
  switch (result)
  {
    case wxKILL_OK:
      wxLogStatus(_("Stopped"));
      DeletePendingEvents();
      m_Timer->Stop();
      m_Dialog->Hide();
      break;

    case wxKILL_BAD_SIGNAL:
      wxLogStatus("no such signal");
      break;

    case wxKILL_ACCESS_DENIED:
    	wxLogStatus("permission denied");
      break;

    case wxKILL_NO_PROCESS: 
      wxLogStatus("no such process");
      break;

    case wxKILL_ERROR:
      wxLogStatus("another, unspecified error");
      break;
    
    default: wxFAIL;
  }
  
  return result;
}

void  wxExProcess::OnCommand(wxCommandEvent& event)
{
  switch (event.GetId())
  {
  case ID_SHELL_COMMAND:
    m_Dialog->GetSTCShell()->LineEnd();
    m_Dialog->GetSTCShell()->AddText(m_Dialog->GetSTCShell()->GetEOL());
    m_Dialog->GetSTCShell()->Prompt();
    
    if (IsRunning()) 
    {
      // send command to process
      wxOutputStream* os = GetOutputStream();
    
      if (os != NULL)
      {
        wxTextOutputStream os(*GetOutputStream());
        os.WriteString(event.GetString() + "\n");
      } 
    }
    break;

  case ID_SHELL_COMMAND_STOP:
    if (IsRunning())
    {
      Kill();
    }
    break;
    
  default: wxFAIL; break;
  }
}
  
void wxExProcess::OnTerminate(int pid, int status)
{
  m_Timer->Stop();

  wxLogStatus(_("Ready"));
  wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, ID_TERMINATED_PROCESS);
  wxPostEvent(wxTheApp->GetTopWindow(), event);
  
  // Collect remaining input.
  CheckInput();
}

void wxExProcess::OnTimer(wxTimerEvent& event)
{
  CheckInput();
}

#if wxUSE_GUI
void wxExProcess::ShowOutput(const wxString& caption) const
{
  if (!m_Error)
  {
    if (m_Dialog != NULL)
    {
      m_Dialog->GetSTC()->SetText(m_Output);
      m_Dialog->SetTitle(caption.empty() ? m_Command: caption);
      m_Dialog->Show();
    }
    else if (!m_Output.empty())
    {
      wxLogMessage(m_Output);
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
