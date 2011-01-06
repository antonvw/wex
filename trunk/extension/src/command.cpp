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
#include <wx/extension/log.h>

#if wxUSE_GUI
wxExSTCEntryDialog* wxExCommand::m_Dialog = NULL;
#endif

wxExCommand::wxExCommand()
  : m_Error(false)
{
}

long wxExCommand::Execute(const wxString& command, const wxString& wd)
{
  m_Command = command;
  m_Error = false;
  
  wxLogStatus(m_Command);

#if wxUSE_GUI
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
#endif
  
  m_Output.clear();

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

  wxArrayString output;
  wxArrayString errors;
  long retValue;

  // Call wxExecute to execute the command and
  // collect the output and the errors.
  if ((retValue = wxExecute(
    m_Command,
    output,
    errors)) == -1)
  {
    // See also wxExProcess, same log error is shown.
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

  // First output the errors.
  for (
    size_t i = 0;
    i < errors.GetCount();
    i++)
  {
    m_Output += errors[i] + "\n";
  }

  // Set the error member variable. 
  // We have an error if there were errors, and no output, 
  // or the  command could not be executed.  
  m_Error = (!errors.empty() && output.empty()) || retValue == -1;

  // Then add the normal output, will be empty if there are errors.
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
  if (m_Dialog != NULL)
  {
    m_Dialog->SetText(m_Output);
    m_Dialog->SetTitle(caption.empty() ? m_Command: caption);
    m_Dialog->Show();
  }
}
#endif
