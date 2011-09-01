////////////////////////////////////////////////////////////////////////////////
// Name:      process.cpp
// Purpose:   Implementation of class wxExProcess
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/regex.h>
#include <wx/tokenzr.h>
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
  m_Command = p.m_Command;
  m_Error = p.m_Error;
  m_Output = p.m_Output;
  m_Timer = new wxTimer(this);
  
  return *this;
}

bool wxExProcess::CheckInput()
{
  // This assumes that the output is always line buffered.
  wxString line;

  if (IsInputAvailable())
  {
    wxTextInputStream tis(*GetInputStream());
    line << tis.ReadLine();
  }
  else if (IsErrorAvailable())
  {
    wxTextInputStream tis(*GetErrorStream());
    line << tis.ReadLine();
  }

  if (line.empty())
  {
    return false;
  }
  
  wxString lineno;
  wxString path;

  // Check on error in php script output.
  const wxRegEx regex(".*in \\(.*\\) on line \\(.*\\)", wxRE_ADVANCED);

  if (regex.Matches(line))
  {
    size_t start, len;

    if (regex.GetMatch(&start, &len, 1))
    {
      path = line.substr(start, len);
    }

    if (regex.GetMatch(&start, &len, 2))
    {
      lineno = line.substr(start, len);
    }
  }
  else
  {
    // Check on error in gcc output (and some others).
    wxStringTokenizer tkz(line, ':');
    path = tkz.GetNextToken();

    if (tkz.HasMoreTokens())
    {
      lineno = tkz.GetNextToken();
    }
  }

  if (atoi(lineno.c_str()) == 0)
  {
    lineno.clear();
  }
    
  if (!wxFileExists(path))
  {
    lineno.clear();
    path.clear();
  }
    
  ReportAdd(line, path, lineno);

  return true;
}

int wxExProcess::ConfigDialog(
  wxWindow* parent,
  const wxString& title)
{
  std::vector<wxExConfigItem> v;

  v.push_back(wxExConfigItem(
    _("Process"), 
    CONFIG_COMBOBOX, 
    wxEmptyString));

  v.push_back(wxExConfigItem(
    m_WorkingDirKey, 
    CONFIG_COMBOBOXDIR, 
    wxEmptyString,
    true,
    1000));

  return wxExConfigDialog(parent,
    v,
    title).ShowModal();
}

long wxExProcess::Execute(
  const wxString& command,
  int flags,
  const wxString& wd)
{
  if (command.empty())
  {
    if (m_Command.empty())
    {
      if (ConfigDialog(wxTheApp->GetTopWindow()) == wxID_CANCEL)
      {
        return -1;
      }
    }
    
    m_Command = wxExConfigFirstOf(_("Process"));
  }
  else
  {
    m_Command = command;
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
      
    m_Dialog->GetSTCShell()->SetEventHandler(this);
  }
      
  const struct wxExecuteEnv env = {
    (wd.empty() ? wxExConfigFirstOf(m_WorkingDirKey): wd), 
    wxEnvVariableHashMap()};
    
  if (!(flags & wxEXEC_SYNC))
  {
    // For asynchronous execution, however, the return value is the process id and zero 
    // value indicates that the command could not be executed
    const long pid = wxExecute(m_Command, flags, this, &env);

    if (pid > 0)
    {
      wxLogVerbose(_("Execute") + ": " + m_Command);
      
      m_Dialog->GetSTCShell()->EnableShell(true);
    
      ReportCreate();

      m_Timer->Start(1000); // each 1000 milliseconds
    }

    return pid;
  }
  else
  {
    wxArrayString output;
    wxArrayString errors;
    long retValue;
    
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
      wxLogVerbose(_("Execute") + ": " + m_Command);
    }

    // We have an error if the command could not be executed.  
    m_Error = (retValue == -1);
    m_Output = wxJoin(errors, '\n') + wxJoin(output, '\n');
  
    return retValue;
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

bool wxExProcess::IsSelected() const
{
  return 
    !m_Command.empty() ||
    !wxExConfigFirstOf(_("Process")).empty();
}

wxKillError wxExProcess::Kill(wxSignal sig)
{
  if (!IsRunning())
  {
    return wxKILL_NO_PROCESS;
  }

  m_Timer->Stop();
  
  wxLogStatus(_("Stopped"));

  DeletePendingEvents();

  if (m_Dialog != NULL)
  {
    m_Dialog->GetSTCShell()->Prompt();
  }
  
  return wxProcess::Kill(GetPid(), sig);
}

void  wxExProcess::OnCommand(wxCommandEvent& event)
{
  switch (event.GetId())
  {
  case ID_SHELL_COMMAND:
    {
    // send command to process
    wxTextOutputStream os(*GetOutputStream());
    os.WriteString(event.GetString());
    m_Dialog->GetSTCShell()->Prompt();
    }
    break;

  case ID_SHELL_COMMAND_STOP:
    Kill();
    break;
    
  default: wxFAIL; break;
  }
}
  
void wxExProcess::OnTerminate(int pid, int status)
{
  m_Timer->Stop();

  // Collect remaining input.
  while (CheckInput())
  {
    // Do nothing.
  }

  wxLogStatus(_("Ready"));
  wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, ID_TERMINATED_PROCESS);
  wxPostEvent(wxTheApp->GetTopWindow(), event);
  m_Dialog->GetSTCShell()->Prompt(wxEmptyString, false); // no eol
}

void wxExProcess::OnTimer(wxTimerEvent& event)
{
  while (CheckInput())
  {
    // Do nothing.
  }
}

void wxExProcess::ReportAdd(
  const wxString& line, 
  const wxString& path,
  const wxString& lineno)
{
  m_Dialog->GetSTCShell()->AddText(line + m_Dialog->GetSTCShell()->GetEOL());
}

void wxExProcess::ReportCreate()
{
  m_Dialog->SetTitle(m_Command);
  m_Dialog->GetSTCShell()->ClearAll();
  m_Dialog->GetSTCShell()->Prompt();
  m_Dialog->Show();
}

#if wxUSE_GUI
void wxExProcess::ShowOutput(const wxString& caption) const
{
  if (!m_Error && m_Dialog != NULL)
  {
    m_Dialog->GetSTC()->SetText(m_Output);
    m_Dialog->SetTitle(caption.empty() ? m_Command: caption);
    m_Dialog->Show();
  }
}
#endif
