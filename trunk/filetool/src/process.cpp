/******************************************************************************\
* File:          process.cpp
* Purpose:       Implementation of class 'exProcessWithListView' and support classes
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/tokenzr.h>
#include <wx/txtstrm.h> // for wxTextInputStream
#include <wx/extension/configdialog.h>
#include <wx/filetool/process.h>
#include <wx/filetool/defs.h>
#include <wx/filetool/listitem.h>
#include <wx/filetool/listview.h>

class exThread : public wxThread
{
public:
  exThread(exProcessWithListView* process) 
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
  exProcessWithListView* m_Process;
};

wxString exProcessWithListView::m_Command;

exProcessWithListView::exProcessWithListView(
  exListViewFile* listview, 
  const wxString& command)
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

void exProcessWithListView::CheckInput()
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
      exListItemWithFileName item(m_Owner, fn.GetFullPath());
      item.Insert();
      item.SetColumnText(_("Line"), line);
      item.SetColumnText(_("Line No"), lineno);
    }
    else
    {
      exListItemWithFileName item(m_Owner, wxEmptyString); // exListItem gives incorrect image
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

int exProcessWithListView::ConfigDialog()
{
  std::vector<exConfigItem> v;
  v.push_back(exConfigItem(_("Process"), CONFIG_COMBOBOX, wxEmptyString, true));
  v.push_back(exConfigItem(_("In folder"), CONFIG_COMBOBOXDIR, wxEmptyString, true));

  const int result = exConfigDialog(NULL,
    exApp::GetConfig(),
    v,
    _("Select Process")).ShowModal();

  if (result == wxID_OK)
  {
    wxSetWorkingDirectory(exApp::GetConfig(_("In folder")));
    m_Command = exApp::GetConfig(_("Process"));
  }

  return result;
}

bool exProcessWithListView::IsRunning() const 
{
  return m_Thread != NULL && m_Thread->IsRunning();
}

void exProcessWithListView::OnTerminate(int WXUNUSED(pid), int WXUNUSED(status))
{
  exFrame::StatusText(_("Ready"));

  if (m_Thread != NULL)
  {
    m_Thread->Delete();
    // Joinable threads should be deleted explicitly
    delete m_Thread;
    m_Thread = NULL;
  }

  wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, ID_TERMINATED_PROCESS);
  wxPostEvent(m_Owner, event);
}

bool exProcessWithListView::Run()
{
  if (m_Command.empty())
  {
    wxLogError("Process is empty");
    return false;
  }

  long pid;

  if ((pid = wxExecute(m_Command, wxEXEC_ASYNC, this)) > 0)
  {
    SetPid(pid);

    m_Thread = new exThread(this);

    if (m_Thread->Create() == wxTHREAD_NO_ERROR)
    {
      if (m_Thread->Run() == wxTHREAD_NO_ERROR)
      {
        exFrame::StatusText(m_Command);
        exApp::Log(_("Running process") + ": " + m_Command);
        return true;
      }
    }
  }

  wxLogError("Cannot run process: " + m_Command);

  return false;
}

void exProcessWithListView::Stop()
{
  Kill(GetPid(), wxSIGKILL);

  exFrame::StatusText(_("Stopped"));
}
