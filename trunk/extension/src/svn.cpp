/******************************************************************************\
* File:          svn.cpp
* Purpose:       Implementation of exSVN class
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/extension/svn.h>
#include <wx/extension/stc.h>

#if wxUSE_GUI
exSVN::exSVN(exSvnType m_Type, const wxString& fullpath)
  : m_Type(m_Type)
  , m_Contents()
  , m_FullPath(fullpath)
{
  switch (m_Type)
  {
    case SVN_CAT: 
      m_Caption = _("SVN Cat");
      m_Command = "cat";
      break;
    case SVN_COMMIT: 
      m_Caption = _("SVN Commit");
      m_Command = "commit";
      break;
    case SVN_DIFF: 
      m_Caption = _("SVN Diff"); 
      m_Command = "diff";
      break;
    case SVN_INFO: 
      m_Caption = _("SVN Info"); 
      m_Command = "info";
      break;
    case SVN_LOG: 
      m_Caption = _("SVN Log"); 
      m_Command = "log";
      break;
    case SVN_STAT: 
      m_Caption = _("SVN Stat"); 
      m_Command = "stat";
      break;
    default:
      wxLogError("SVN type: %d not handled", m_Type);
      break;
  }
}

int exSVN::Get()
{
  m_Contents.clear();

  std::vector<exConfigItem> v;

  if (m_Type == SVN_COMMIT)
  {
    v.push_back(exConfigItem(_("Revision comment"), CONFIG_COMBOBOX));
  }

  if (m_FullPath.empty())
  {
    v.push_back(exConfigItem(_("Base folder"), CONFIG_COMBOBOXDIR, wxEmptyString, true));
  }

  v.push_back(exConfigItem(_("Flags")));

  if (exConfigDialog(wxTheApp->GetTopWindow(),
    v,
    m_Caption).ShowModal() == wxID_CANCEL)
  {
    m_ReturnCode = -1;
    return m_ReturnCode;
  }

  const wxString cwd = wxGetCwd();

  wxString file;

  if (m_FullPath.empty())
  {
    wxSetWorkingDirectory(exApp::GetConfig(_("Base folder")));  
  }
  else
  {
    file = " \"" + m_FullPath + "\"";
  }

  wxArrayString output;
  wxArrayString errors;
  
  wxString arg;
  if (m_Type == SVN_COMMIT) arg = " -m \"" + exApp::GetConfig(_("Revision comment")) + "\"";

  wxExecute(
    "svn " + exApp::GetConfig(_("Flags")) + " " + m_Command + arg + file,
    output,
    errors);
    
  if (m_FullPath.empty())
  {
    wxSetWorkingDirectory(cwd);
  }

  // First output the errors.
  for (size_t i = 0; i < errors.GetCount(); i++)
  {
    m_Contents += errors[i] + "\n";
  }

  // Then the normal output, will be empty if there are errors.
  for (size_t j = 0; j < output.GetCount(); j++)
  {
    m_Contents += output[j] + "\n";
  }

  m_ReturnCode = errors.GetCount();
  return m_ReturnCode;
}

bool exSVN::Show()
{
  if (Get() < 0) return false;

  ShowContents();

  return true;
}

void exSVN::ShowContents()
{
  // Create a dialog for contents.
  exSTCEntryDialog* dlg = new exSTCEntryDialog(
    wxTheApp->GetTopWindow(), 
    m_Caption + (!m_FullPath.empty() ? " " + wxFileName(m_FullPath).GetFullName(): wxString(wxEmptyString)),
    m_Contents, 
    wxEmptyString, 
    wxOK,
    wxID_ANY,
    wxDefaultPosition, wxSize(550, 250));

  // Add a lexer if we specified a path, asked for cat and there were no errors.
  if (
    !m_FullPath.empty() && 
     m_Type == SVN_CAT &&
     m_ReturnCode == 0)
  { 
    exFileName fn(m_FullPath); 
    dlg->SetLexer(fn.GetLexer().GetScintillaLexer());
  }
  
  dlg->Show();
}

#endif
