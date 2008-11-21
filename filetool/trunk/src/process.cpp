/******************************************************************************\
* File:          process.cpp
* Purpose:       Implementation of class 'ftProcess' and support classes
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2008 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/tokenzr.h>
#include <wx/txtstrm.h> // for wxTextInputStream
#include <wx/filetool/process.h>
#include <wx/filetool/filetool.h>

class ftThread : public wxThread
{
public:
  ftThread(ftProcess* process) 
    : wxThread(wxTHREAD_JOINABLE)
    , m_Process(process) {}
protected:
  virtual ExitCode Entry()
  {
    while (!TestDestroy())
    {
      m_Process->CheckInput();

      Yield();
      Sleep(20);
    }

    return NULL;
  };
private:
  ftProcess* m_Process;
};

wxString ftProcess::m_Command;

ftProcess::ftProcess(ftListView* listview, const wxString& command)
  : wxProcess(listview, -1)
  , m_Owner(listview)
  , m_Thread(NULL)
{
  if (!command.empty())
  {
    m_Command = command;
  }

  Redirect();
}

void ftProcess::CheckInput()
{
  bool hasInput = false;

  // This assumes that the output is always line buffered.
  wxString line;

  if (IsInputAvailable())
  {
    wxTextInputStream tis(*GetInputStream());
    line << tis.ReadLine();
    hasInput = true;
  }
  else if (IsErrorAvailable())
  {
    wxTextInputStream tis(*GetErrorStream());
    line << tis.ReadLine();
    hasInput = true;
  }

  if (hasInput && !line.empty())
  {
    wxString lineno;
    wxString path;

    // Check on error in php script output.
    wxRegEx regex(".*in \\(.*\\) on line \\(.*\\)");

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
        const wxString number = tkz.GetNextToken();
        long val;
        if (number.ToLong(&val)) lineno = number;
      }
    }

    wxFileName fn(path);
    fn.Normalize();

    if (fn.FileExists())
    {
      ftListItem item(m_Owner, fn.GetFullPath());
      item.Insert();
      item.SetColumnText(_("Line"), line);
      item.SetColumnText(_("Line No"), lineno);
    }
    else
    {
      ftListItem item(m_Owner, wxEmptyString); // exListItem gives incorrect image
      item.Insert();
      item.SetColumnText(_("Line"), line);
    }

    // If nothing selected, then ensure last line is visible.
    // Otherwise you are busy inspecting some line, and
    // it would be irritating to get it out of view.
    if (m_Owner->GetSelectedItemCount() == 0)
    {
      m_Owner->EnsureVisible(m_Owner->GetItemCount() - 1);
    }

    m_Owner->UpdateStatusBar();
  }
}

int ftProcess::ConfigDialog()
{
  // Better do not give it this as parent, as the process list usually is not yet created.
  std::vector<exConfigItem> v;
  v.push_back(exConfigItem(_("Process"), CONFIG_COMBOBOX, wxEmptyString, true));
  v.push_back(exConfigItem(_("In folder"), CONFIG_COMBOBOXDIR, wxEmptyString, true));

  const int result = exConfigDialog(NULL,
    v,
    _("Select Process")).ShowModal();

  if (result == wxID_OK)
  {
    wxSetWorkingDirectory(exApp::GetConfig(_("In folder")));
    m_Command = exApp::GetConfig(_("Process"));
  }

  return result;
}

bool ftProcess::IsRunning() const 
{
  return m_Thread != NULL && m_Thread->IsRunning();
}

void ftProcess::OnTerminate(int WXUNUSED(pid), int WXUNUSED(status))
{
  exStatusText(_("Ready"));

  if (m_Thread != NULL)
  {
    m_Thread->Delete();
    wxDELETE(m_Thread);
  }

  m_Owner->ProcessTerminated();
}

bool ftProcess::Run()
{
  long pid;

  if ((pid = wxExecute(m_Command, wxEXEC_ASYNC, this)) > 0)
  {
    SetPid(pid);

    m_Thread = new ftThread(this);

    if (m_Thread->Create() == wxTHREAD_NO_ERROR)
    {
      if (m_Thread->Run() == wxTHREAD_NO_ERROR)
      {
        exStatusText(m_Command);
        exApp::Log(_("Running process") + ": " + m_Command);
        return true;
      }
    }
  }

  wxLogError("Cannot run process: " + m_Command);

  return false;
}

void ftProcess::Stop()
{
  if (m_Thread != NULL)
  {
    m_Thread->Delete();
  }

  Kill(GetPid(), wxSIGKILL);

  exStatusText(_("Stopped"));
}
