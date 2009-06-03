/******************************************************************************\
* File:          svn.cpp
* Purpose:       Implementation of wxExSVN class
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/extension/svn.h>
#include <wx/extension/configdialog.h>
#include <wx/extension/stc.h>

#if wxUSE_GUI

wxExSTCEntryDialog* wxExSVN::m_STCEntryDialog = NULL;

wxExSVN::wxExSVN(int command_id, const wxString& fullpath)
  : m_Type(GetType(command_id))
  , m_Output()
  , m_FullPath(fullpath)
  , m_ReturnCode(-2)
{
  Initialize();
}

wxExSVN::wxExSVN(wxExSVNType type, const wxString& fullpath)
  : m_Type(type)
  , m_Output()
  , m_FullPath(fullpath)
  , m_ReturnCode(-2)
{
  Initialize();
}

int wxExSVN::Execute(bool show_dialog)
{
  const wxString svn_flags_name = wxString::Format("svn/flags%d", m_Type);
  const wxString svn_flags_contents = wxExApp::GetConfig(svn_flags_name);

  if (show_dialog)
  {
    std::vector<wxExConfigItem> v;

    if (m_Type == SVN_COMMIT)
    {
      v.push_back(wxExConfigItem(_("Revision comment"), CONFIG_COMBOBOX));
    }

    if (m_FullPath.empty())
    {
      v.push_back(wxExConfigItem(_("Base folder"), CONFIG_COMBOBOXDIR, wxEmptyString, true));
    }

    // SVN_UPDATE has no flags to ask for.
    if (m_Type != SVN_UPDATE)
    {
      wxExApp::SetConfig(_("Flags"), svn_flags_contents);
      v.push_back(wxExConfigItem(_("Flags")));
    }

    if (wxExConfigDialog(wxTheApp->GetTopWindow(),
      wxExApp::GetConfig(),
      v,
      m_Caption).ShowModal() == wxID_CANCEL)
    {
      m_ReturnCode = -1;
      return m_ReturnCode;
    }
  }

  const wxString cwd = wxGetCwd();

  wxString file;

  if (m_FullPath.empty())
  {
    wxSetWorkingDirectory(wxExApp::GetConfig(_("Base folder")));
  }
  else
  {
    file = " \"" + m_FullPath + "\"";
  }

  wxString arg;

  if (m_Type == SVN_COMMIT)
  {
    arg = " -m \"" + wxExApp::GetConfig(_("Revision comment")) + "\"";
  }

  wxString flags = wxExApp::GetConfig(_("Flags"));

  if (!flags.empty())
  {
    flags += " ";

    wxExApp::SetConfig(svn_flags_name, flags);
  }

  const wxString command = "svn " + flags + m_Command + arg + file;

  wxArrayString output;
  wxArrayString errors;

  if (wxExecute(
    command,
    output,
    errors) == -1)
  {
    m_ReturnCode = -1;
    return m_ReturnCode;
  }

  if (errors.GetCount() == 0)
  {
    wxExApp::Log(command);
  }

  if (m_FullPath.empty())
  {
    wxSetWorkingDirectory(cwd);
  }

  m_Output.clear();

  // First output the errors.
  for (size_t i = 0; i < errors.GetCount(); i++)
  {
    m_Output += errors[i] + "\n";
  }

  // Then the normal output, will be empty if there are errors.
  for (size_t j = 0; j < output.GetCount(); j++)
  {
    m_Output += output[j] + "\n";
  }

  m_ReturnCode = errors.GetCount();

  return m_ReturnCode;
}

int wxExSVN::ExecuteAndShowOutput()
{
  if (Execute() >= 0)
  {
    ShowOutput();
  }

  return m_ReturnCode;
}

wxExSVNType wxExSVN::GetType(int command_id) const
{
  switch (command_id)
  {
    case ID_EDIT_SVN_BLAME: return SVN_BLAME; break;
    case ID_EDIT_SVN_CAT: return SVN_CAT; break;
    case ID_EDIT_SVN_COMMIT: return SVN_COMMIT; break;
    case ID_EDIT_SVN_DIFF: return SVN_DIFF; break;
    case ID_EDIT_SVN_LOG: return SVN_LOG; break;
    default:
      wxFAIL;
      break;
  }
  
  return SVN_STAT;
}

void wxExSVN::Initialize()
{
  switch (m_Type)
  {
    case SVN_BLAME:  m_Caption = "SVN Blame"; break;
    case SVN_CAT:    m_Caption = "SVN Cat"; break;
    case SVN_COMMIT: m_Caption = "SVN Commit"; break;
    case SVN_DIFF:   m_Caption = "SVN Diff"; break;
    case SVN_INFO:   m_Caption = "SVN Info"; break;
    case SVN_LOG:    m_Caption = "SVN Log"; break;
    case SVN_STAT:   m_Caption = "SVN Stat"; break;
    case SVN_UPDATE: m_Caption = "SVN Update"; break;
    default:
      wxFAIL;
      break;
  }

  m_Command = m_Caption.AfterFirst(' ').Lower();
}

void wxExSVN::ShowOutput() const
{
  // If we did not yet ask Execute, or cancelled, return.
  if (m_ReturnCode < 0)
  {
    return;
  }

  const wxString caption = m_Caption +
    (!m_FullPath.empty() ? " " + wxFileName(m_FullPath).GetFullName(): wxString(wxEmptyString));

  // Create a dialog for contents.
  if (m_STCEntryDialog == NULL)
  {
    m_STCEntryDialog = new wxExSTCEntryDialog(
      wxTheApp->GetTopWindow(),
      caption,
      m_Output,
      wxEmptyString,
      wxOK,
      wxID_ANY,
      wxDefaultPosition, wxSize(575, 250));
  }
  else
  {
    m_STCEntryDialog->SetText(m_Output);
    m_STCEntryDialog->SetTitle(caption);
  }

  // Add a lexer if we specified a path, asked for cat or blame 
  // and there were no errors, and there is a lexer.
  if (
    !m_FullPath.empty() &&
    (m_Type == SVN_CAT || m_Type == SVN_BLAME) &&
     m_ReturnCode == 0)
  {
    const wxExFileName fn(m_FullPath);

    if (!fn.GetLexer().GetScintillaLexer().empty())
    {
      m_STCEntryDialog->SetLexer(fn.GetLexer().GetScintillaLexer());
    }
  }

  m_STCEntryDialog->Show();
}

#endif
