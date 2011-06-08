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

#if wxUSE_GUI
wxExSTCEntryDialog* wxExCommand::m_Dialog = NULL;
#endif

wxExCommand::wxExCommand()
  : m_Error(false)
{
}

long wxExCommand::Execute(const wxString& command, const wxString& wd)
{
  // See also wxExProcess, uses similar messages.
  
  wxString cwd;
  
  if (!wd.empty())
  {
    cwd = wxGetCwd();
    
    if (!wxSetWorkingDirectory(wd))
    {
      wxLogError(_("Cannot set working directory"));
      m_Error = true;
      return -1;
    }
  }

#if wxUSE_GUI
  // Cannot be in the constructor, that gives an assert.
  if (m_Dialog == NULL)
  {
    m_Dialog = new wxExSTCEntryDialog(
      wxTheApp->GetTopWindow(),
      _("Command"),
      wxEmptyString,
      wxEmptyString,
      wxOK);
  }
#endif

  m_Command = command;
  wxLogStatus(m_Command);
  
  wxArrayString output;
  wxArrayString errors;
  long retValue;

  // Call wxExecute to execute the command and
  // collect the output and the errors.
  if ((retValue = wxExecute(
    m_Command,
    output,
    errors)) != -1)
  {
    wxLogVerbose(_("Execute") + ": " + m_Command);
  }

  if (!cwd.empty())
  {
    wxSetWorkingDirectory(cwd);
  }

  // We have an error if the command could not be executed.  
  m_Error = (retValue == -1);
  m_Output = wxJoin(errors, '\n') + wxJoin(output, '\n');

  return retValue;
}

#if wxUSE_GUI
void wxExCommand::ShowOutput(const wxString& caption) const
{
  if (!m_Error)
  {
    if (m_Dialog != NULL)
    {
      m_Dialog->SetText(m_Output);
      m_Dialog->SetTitle(caption.empty() ? m_Command: caption);
      m_Dialog->Show();
    }
    else if (!m_Output.empty())
    {
      wxMessageBox(m_Output);
    }
  }
}
#endif
