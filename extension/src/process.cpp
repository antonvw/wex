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
#include <wx/extension/shell.h>
#include <wx/extension/util.h> // for wxExConfigFirstOf

BEGIN_EVENT_TABLE(wxExProcess, wxProcess)
  EVT_TIMER(-1, wxExProcess::OnTimer)
END_EVENT_TABLE()

wxString wxExProcess::m_Command;
wxString wxExProcess::m_WorkingDirKey = _("Process folder");

wxExProcess::wxExProcess(
  const wxString& command)
  : wxProcess(wxPROCESS_REDIRECT)
  , m_Timer(this)
  , m_Shell(NULL)
{
  if (!command.empty())
  {
    m_Command = command;
  }
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

  if (!line.empty())
  {
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
      path.clear();
    }
    
    ReportAdd(line, path, lineno);
  }

  return !line.empty();
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

  const auto result = wxExConfigDialog(parent,
    v,
    title).ShowModal();

  if (result == wxID_OK)
  {
    m_Command = wxExConfigFirstOf(_("Process"));
  }

  return result;
}

long wxExProcess::Execute(const wxString& wd)
{
  if (m_Command.empty())
  {
    if (ConfigDialog(wxTheApp->GetTopWindow()) == wxID_CANCEL)
    {
      return -1;
    }
  }

  const struct wxExecuteEnv env = {
    (wd.empty() ? wxExConfigFirstOf(m_WorkingDirKey): wd), 
    wxEnvVariableHashMap()};
  
  // For asynchronous execution, however, the return value is the process id and zero 
  // value indicates that the command could not be executed
  const long pid = wxExecute(m_Command, wxEXEC_ASYNC, this, &env);

  if (pid > 0)
  {
    wxLogVerbose(_("Execute") + ": " + m_Command);
    
    ReportCreate();

    m_Timer.Start(1000); // each 1000 milliseconds
  }

  return pid;
}

bool wxExProcess::IsRunning() const
{
  if (GetPid() < 0)
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

  m_Timer.Stop();
  
  wxLogStatus(_("Stopped"));

  DeletePendingEvents();

  if (m_Shell != NULL)
  {
    m_Shell->Prompt();
  }
  
  return wxProcess::Kill(GetPid(), sig);
}

void wxExProcess::OnTerminate(int pid, int status)
{
  m_Timer.Stop();

  // Collect remaining input.
  while (CheckInput())
  {
    // Do nothing.
  }

  wxLogStatus(_("Ready"));
  wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, ID_TERMINATED_PROCESS);
  wxPostEvent(wxTheApp->GetTopWindow(), event);
  
  if (m_Shell != NULL)
  {
    m_Shell->Prompt();
  }
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
  m_Shell->AddText(line);
}

void wxExProcess::ReportCreate()
{
  if (m_Shell == NULL)
  {
    m_Shell = new wxExSTCEntryDialog(wxTheApp->GetTopWindow());
    m_Shell->SetEventHandler(this);
  }
  else
  {
    m_Shell->Clear();
    m_Shell->Prompt();
  }
}
