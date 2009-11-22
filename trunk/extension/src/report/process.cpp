/******************************************************************************\
* File:          process.cpp
* Purpose:       Implementation of class 'wxExProcessWithListView'
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/tokenzr.h>
#include <wx/txtstrm.h> // for wxTextInputStream
#include <wx/extension/configdlg.h>
#include <wx/extension/extension.h>
#include <wx/extension/log.h>
#include <wx/extension/report/defs.h>
#include <wx/extension/report/frame.h>
#include <wx/extension/report/listitem.h>
#include <wx/extension/report/process.h>

BEGIN_EVENT_TABLE(wxExProcessWithListView, wxProcess)
  EVT_TIMER(-1, wxExProcessWithListView::OnTimer)
END_EVENT_TABLE()

wxString wxExProcessWithListView::m_Command = "";

wxExProcessWithListView::wxExProcessWithListView(
  wxExFrameWithHistory* frame,
  wxExListView* listview,
  const wxString& command)
  : wxProcess(NULL, -1)
  , m_Frame(frame)
  , m_Owner(listview)
  , m_Timer(this)
{
  if (!command.empty())
  {
    m_Command = command;
  }

  Redirect();
}

bool wxExProcessWithListView::CheckInput()
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
      wxExListItemWithFileName item(m_Owner, fn.GetFullPath());
      item.Insert();
      item.SetColumnText(_("Line"), line);
      item.SetColumnText(_("Line No"), lineno);
    }
    else
    {
      // wxExListItem gives incorrect image
      wxExListItemWithFileName item(m_Owner, wxEmptyString);
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

#if wxUSE_STATUSBAR
    m_Owner->UpdateStatusBar();
#endif
  }

  return hasInput;
}

int wxExProcessWithListView::ConfigDialog(
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

long wxExProcessWithListView::Execute()
{
  if (m_Command.empty())
  {
    wxFAIL;
  }

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

void wxExProcessWithListView::InitCommandFromConfig()
{
  // The process is a combobox, we want only the first from the list,
  // so use the default separator, causing only first field to be returned.
  m_Command = wxExConfigFirstOf(_("Process"));
}

wxKillError wxExProcessWithListView::Kill(wxSignal sig)
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

void wxExProcessWithListView::OnTerminate(
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
  wxPostEvent(m_Frame, event);
}

void wxExProcessWithListView::OnTimer(wxTimerEvent& event)
{
  while (CheckInput())
  {
    // Do nothing.
  }
}
