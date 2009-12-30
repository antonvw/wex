/******************************************************************************\
* File:          process.cpp
* Purpose:       Implementation of class 'wxExProcess'
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/regex.h>
#include <wx/tokenzr.h>
#include <wx/txtstrm.h> // for wxTextInputStream
#include <wx/extension/configdlg.h>
#include <wx/extension/frame.h>
#include <wx/extension/log.h>
#include <wx/extension/util.h>
#include <wx/extension/report/process.h>
#include <wx/extension/report/defs.h>
#include <wx/extension/report/frame.h>
#include <wx/extension/report/listitem.h>
#include <wx/extension/report/listview.h>

BEGIN_EVENT_TABLE(wxExProcess, wxProcess)
  EVT_TIMER(-1, wxExProcess::OnTimer)
END_EVENT_TABLE()

int wxExProcess::m_Instances = 0;
wxString wxExProcess::m_Command = "";

wxExProcess::wxExProcess(
  wxExFrameWithHistory* frame,
  const wxString& command)
  : wxProcess(NULL, -1)
  , m_Frame(frame)
  , m_ListView(NULL)
  , m_Timer(this)
{
  if (m_Instances == 0)
  {
    m_Command = wxExConfigFirstOf(_("Process"));
  }

  m_Instances++;

  if (!command.empty())
  {
    m_Command = command;
  }

  Redirect();
}

wxExProcess::~wxExProcess()
{
  m_Instances--;
}

bool wxExProcess::CheckInput()
{
  wxASSERT(m_ListView != NULL); 

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
    wxRegEx regex(".*in \\(.*\\) on line \\(.*\\)", wxRE_ADVANCED);

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
      wxExListItemWithFileName item(m_ListView, fn.GetFullPath());
      item.Insert();
      item.SetColumnText(_("Line"), line);
      item.SetColumnText(_("Line No"), lineno);
    }
    else
    {
      // wxExListItem gives incorrect image
      wxExListItemWithFileName item(m_ListView, wxEmptyString);
      item.Insert();
      item.SetColumnText(_("Line"), line);
    }

    // If nothing selected, then ensure last line is visible.
    // Otherwise you are busy inspecting some line, and
    // it would be irritating to get it out of view.
    if (m_ListView->GetSelectedItemCount() == 0)
    {
      m_ListView->EnsureVisible(m_ListView->GetItemCount() - 1);
    }

#if wxUSE_STATUSBAR
    m_ListView->UpdateStatusBar();
#endif
  }

  return hasInput;
}

int wxExProcess::ConfigDialog(
  wxWindow* parent,
  const wxString& title)
{
  std::vector<wxExConfigItem> v;
  v.push_back(wxExConfigItem(_("Process"), CONFIG_COMBOBOX, wxEmptyString, true));
  v.push_back(wxExConfigItem(_("Process folder"), CONFIG_COMBOBOXDIR, wxEmptyString));

  const int result = wxExConfigDialog(parent,
    v,
    title).ShowModal();

  if (result == wxID_OK)
  {
    m_Command = wxExConfigFirstOf(_("Process"));
  }

  return result;
}

long wxExProcess::Execute()
{
  wxASSERT(!m_Command.empty());

  wxString cwd;
  const wxString dir = wxExConfigFirstOf(_("Process folder"));

  if (!dir.empty())
  {
    cwd = wxGetCwd();
    if (!wxSetWorkingDirectory(dir))
    {
      wxLogError(_("Cannot set working directory"));
      return 0;
    }
  }

  long pid = 0;

  if ((pid = wxExecute(m_Command, wxEXEC_ASYNC, this)) > 0)
  {
    m_ListView = m_Frame->Activate(wxExListViewFile::LIST_PROCESS);

    SetPid(pid);

#if wxUSE_STATUSBAR
    wxExFrame::StatusText(m_Command);
#endif
    wxExLog::Get()->Log(m_Command);

    m_Timer.Start(100); // each 100 milliseconds
  }
  else
  {
    wxLogError(_("Cannot execute") + ": " + m_Command);
  }

  if (!cwd.empty())
  {
    wxSetWorkingDirectory(cwd);
  }

  return pid;
}

wxKillError wxExProcess::Kill(wxSignal sig)
{
  if (!Exists(GetPid()))
  {
    return wxKILL_NO_PROCESS;
  }

  m_Timer.Stop();
  
#if wxUSE_STATUSBAR
  wxExFrame::StatusText(_("Stopped"));
#endif

  DeletePendingEvents();

  // Next statement causes wxGTK to crash, so outcommented.
#ifndef __WXGTK__
  delete this;
#endif

  return wxProcess::Kill(GetPid(), sig);
}

void wxExProcess::OnTerminate(
  int WXUNUSED(pid), 
  int WXUNUSED(status))
{
  m_Timer.Stop();

  // Collect remaining input.
  while (CheckInput())
  {
    // Do nothing.
  }

#if wxUSE_STATUSBAR
  wxExFrame::StatusText(_("Ready"));
#endif

  wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, ID_TERMINATED_PROCESS);
  wxPostEvent(wxTheApp->GetTopWindow(), event);
}

void wxExProcess::OnTimer(wxTimerEvent& event)
{
  while (CheckInput())
  {
    // Do nothing.
  }
}
