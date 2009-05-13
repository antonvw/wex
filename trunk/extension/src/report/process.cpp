/******************************************************************************\
* File:          process.cpp
* Purpose:       Implementation of class 'wxExProcessWithListView' and support classes
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/tokenzr.h>
#include <wx/txtstrm.h> // for wxTextInputStream
#include <wx/extension/app.h>
#include <wx/extension/configdialog.h>
#include <wx/extension/report/process.h>
#include <wx/extension/report/defs.h>
#include <wx/extension/report/listitem.h>
#include <wx/extension/report/listview.h>

BEGIN_EVENT_TABLE(wxExProcessWithListView, wxProcess)
  EVT_TIMER(-1, wxExProcessWithListView::OnTimer)
END_EVENT_TABLE()

wxString wxExProcessWithListView::m_Command = "";

wxExProcessWithListView::wxExProcessWithListView(
  wxExListViewFile* listview,
  const wxString& command)
  : wxProcess(listview, -1)
  , m_Owner(listview)
  , m_Timer(this)
{
  if (!command.empty())
  {
    m_Command = command;
  }

  Redirect();
}

void wxExProcessWithListView::CheckInput()
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
      wxExListItemWithFileName item(m_Owner, fn.GetFullPath());
      item.Insert();
      item.SetColumnText(_("Line"), line);
      item.SetColumnText(_("Line No"), lineno);
    }
    else
    {
      wxExListItemWithFileName item(m_Owner, wxEmptyString); // wxExListItem gives incorrect image
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

int wxExProcessWithListView::ConfigDialog()
{
  std::vector<wxExConfigItem> v;
  v.push_back(wxExConfigItem(_("Process"), CONFIG_COMBOBOX, wxEmptyString, true));
  v.push_back(wxExConfigItem(_("In folder"), CONFIG_COMBOBOXDIR, wxEmptyString, true));

  const int result = wxExConfigDialog(NULL,
    wxExApp::GetConfig(),
    v,
    _("Select Process")).ShowModal();

  if (result == wxID_OK)
  {
    wxSetWorkingDirectory(wxExApp::GetConfig(_("In folder")));
    m_Command = wxExApp::GetConfig(_("Process"));
  }

  return result;
}

long wxExProcessWithListView::Execute()
{
  long pid;

  if ((pid = wxExecute(m_Command, wxEXEC_ASYNC, this)) > 0)
  {
    SetPid(pid);

    wxExFrame::StatusText(m_Command);
    wxExApp::Log(_("Running process") + ": " + m_Command);

    m_Timer.Start(100); // each 100 milliseconds
  }
  else
  {
    wxLogError(_("Cannot run process") + ": " + m_Command);
  }

  return pid;
}

bool wxExProcessWithListView::IsRunning() const
{
  return Exists(GetPid());
}

wxKillError wxExProcessWithListView::Kill(wxSignal sig)
{
  wxKillError output = wxKILL_OK;
  
  if (Exists(GetPid()))
  {
    output = wxProcess::Kill(GetPid(), sig);

    wxExFrame::StatusText(_("Stopped"));
  }

  m_Timer.Stop();
  
  return output;
}

void wxExProcessWithListView::OnTerminate(int WXUNUSED(pid), int WXUNUSED(status))
{
  m_Timer.Stop();

  // Collect remaining input.
  CheckInput();

  wxExFrame::StatusText(_("Ready"));

  wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, ID_TERMINATED_PROCESS);
  wxPostEvent(m_Owner, event);
}

void wxExProcessWithListView::OnTimer(wxTimerEvent& event)
{
  CheckInput();
}
