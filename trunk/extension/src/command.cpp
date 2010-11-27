////////////////////////////////////////////////////////////////////////////////
// Name:      command.cpp
// Purpose:   Implementation of wxExCommand class
// Author:    Anton van Wezenbeek
// Created:   2010-11-26
// RCS-ID:    $Id$
// Copyright: (c) 2010 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/command.h>
#include <wx/extension/frame.h>
#include <wx/extension/log.h>

#if wxUSE_GUI
wxExSTCEntryDialog* wxExCommand::m_Dialog = NULL;
#endif

long wxExCommand::Execute(const wxString& command, const wxString& wd)
{
  m_Command = command;
  m_Error = false;
  
#if wxUSE_STATUSBAR
  wxExFrame::StatusText(m_Command);
#endif

  if (m_Dialog == NULL)
  {
    m_Dialog = new wxExSTCEntryDialog(
      wxTheApp->GetTopWindow(),
      "Command",
      wxEmptyString,
      wxEmptyString,
      wxOK,
      wxID_ANY,
      wxDefaultPosition,
      wxSize(350, 50));
  }
  
  wxString cwd;
  
  if (!wd.empty())
  {
    cwd = wxGetCwd();
    wxSetWorkingDirectory(wd);
  }

  wxArrayString output;
  wxArrayString errors;
  long retValue;

  // Call wxExcute to execute the vcs command and
  // collect the output and the errors.
  if ((retValue = wxExecute(
    m_Command,
    output,
    errors)) == -1)
  {
    // See also process, same log is shown.
    wxLogError(_("Cannot execute") + ": " + m_Command);
  }
  else
  {
    wxExLog::Get()->Log(m_Command);
  }

  if (!cwd.empty())
  {
    wxSetWorkingDirectory(cwd);
  }

  m_Output.clear();

  // First output the errors.
  for (
    size_t i = 0;
    i < errors.GetCount();
    i++)
  {
    m_Output += errors[i] + "\n";
  }
  
  m_Error = !errors.empty();

  // Then the normal output, will be empty if there are errors.
  for (
    size_t j = 0;
    j < output.GetCount();
    j++)
  {
    m_Output += output[j] + "\n";
  }
  
  return retValue;
}

#if wxUSE_GUI
void wxExCommand::ShowOutput(const wxString& caption) const
{
  m_Dialog->SetText(m_Output);
  m_Dialog->SetTitle(caption.empty() ? m_Command: caption);
  m_Dialog->Show();
}
#endif
