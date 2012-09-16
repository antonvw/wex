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
    ReportAdd(output);
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

long wxExProcess::Execute(
  const wxString& command_to_execute,
  int flags,
  const wxString& wd)
{
  struct wxExecuteEnv env;
    
  if (command_to_execute.empty())
  {
    if (wxExConfigFirstOf(_("Process")).empty())
    {
      if (ConfigDialog(wxTheApp->GetTopWindow()) == wxID_CANCEL)
      {
        return -2;
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

  if (m_Dialog == NULL)
  {
    m_Dialog = new wxExSTCEntryDialog(
      wxTheApp->GetTopWindow(),
      m_Command,
      wxEmptyString,
      wxEmptyString,
      wxOK,
      true);
      
    m_Dialog->SetEventHandler(this);
    m_Dialog->GetSTCShell()->SetEventHandler(this);
  }
      
  m_Error = false;
    
  if (!(flags & wxEXEC_SYNC))
  { 
    if (!ReportCreate())
    {
      return -1; 
    }

    // For asynchronous execution, however, the return value is the process id and zero 
    // value indicates that the command could not be executed
    const long pid = wxExecute(m_Command, flags, this, &env);

    if (pid > 0)
    {
      wxLogVerbose("Execute: " + m_Command);
    
      CheckInput();
      
      m_Timer->Start(100); // each 100 milliseconds
    }
    else
    {
      m_Error = true;
      m_Dialog->Hide();
      
      wxLogStatus(_("Could not execute") + ": " + m_Command);
    }
    
    return pid;
  }
  else
  {
    wxArrayString output;
    wxArrayString errors;
    long retValue;
    m_Output.clear();
    
    m_Dialog->GetSTCShell()->EnableShell(false);
    
    // Call wxExecute to execute the command and
    // collect the output and the errors.
    if ((retValue = wxExecute(
      m_Command,
      output,
      errors,
      flags,
      &env)) != -1)
    {
      wxLogVerbose("Execute: " + m_Command);
    }
    else
    {
      m_Error = true;
      wxLogStatus(_("Could not execute") + ": " + m_Command);
    }

    // Set output by converting array strings into normal strings.
    m_Output = wxJoin(errors, '\n', '\n') + wxJoin(output, '\n', '\n');
  
    return retValue;
  }
}

void wxExProcess::HideDialog()
{
  if (m_Dialog != NULL)
  {
    m_Dialog->Hide();
  }
}

bool wxExProcess::IsRunning() const
{
  if (GetPid() <= 0)
  {
    return false;
  }

  return Exists(GetPid());
}

wxKillError wxExProcess::Kill(wxSignal sig)
{
  // This seems necessary.
  if (!IsRunning())
  {
    wxLogStatus("no such process");
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

bool wxExProcess::ReportAdd(const wxString& line) const
{
  m_Dialog->Show();
  m_Dialog->GetSTCShell()->AddText(line);
  m_Dialog->GetSTCShell()->Prompt();
  return true;
}

bool wxExProcess::ReportCreate()
{
  m_Dialog->SetTitle(m_Command);
  m_Dialog->GetSTCShell()->ClearAll();
  m_Dialog->GetSTCShell()->Prompt();
  m_Dialog->GetSTCShell()->EnableShell(true);
  return true;
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
    wxLogError(m_Command);
  }
}
#endif
