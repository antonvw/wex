/******************************************************************************\
* File:          stc.cpp
* Purpose:       Implementation of class 'ftSTC'
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2008 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/filetool/filetool.h>

BEGIN_EVENT_TABLE(ftSTC, exSTC)
  EVT_MENU_RANGE(ID_STC_LOWEST, ID_STC_HIGHEST, ftSTC::OnCommand)
  EVT_MENU_RANGE(ID_TOOL_LOWEST, ID_TOOL_HIGHEST, ftSTC::OnCommand)
END_EVENT_TABLE()

#if wxUSE_DRAG_AND_DROP
class ftSTCDropTarget : public wxFileDropTarget
{
public:
  ftSTCDropTarget(ftFrame* owner) {m_Owner = owner;}
private:
  virtual bool OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& filenames)
  {
    ftOpenFiles(m_Owner, filenames);
    return true;
  }

  ftFrame* m_Owner;
};
#endif

ftSTC::ftSTC(wxWindow* parent,
  long type,
  const wxString& value,
  wxWindowID id,
  const wxPoint& pos,
  const wxSize& size,
  long style,
  const wxString& name)
  : exSTC(parent, type, value, id, pos, size, style, name)
{
  Initialize();
}

ftSTC::ftSTC(wxWindow* parent,
  const exFileName& filename,
  int line_number,
  const wxString& match,
  long flags,
  long type,
  wxWindowID id,
  const wxPoint& pos,
  const wxSize& size,
  long style,
  const wxString& name)
  : exSTC(parent, filename, line_number, match, flags, type, id, pos, size, style, name)
{
  if (Initialize())
  {
    m_Frame->SetRecentFile(GetFileName().GetFullPath());
  }
}

ftSTC::ftSTC(const exSTC& stc)
  : exSTC(stc)
{
  Initialize();
}

void ftSTC::BuildPopupMenu(exMenu& menu)
{
  exSTC::BuildPopupMenu(menu);

  // Add tools if we have at least some text, the tool flag,
  // and a lexer.
  if (GetSelectedText().empty() && GetTextLength() > 0 &&
     (GetMenuFlags() & STC_MENU_TOOL) &&
      !GetFileName().GetLexer().GetScintillaLexer().empty())
  {
    menu.AppendSeparator();
    menu.AppendTools(ID_STC_TOOL_MENU);
  }

  if (GetMenuFlags() & (STC_MENU_REPORT_FIND | STC_MENU_REPORT_REPLACE))
  {
    menu.AppendSeparator();

    if (GetMenuFlags() & STC_MENU_REPORT_FIND)
    {
      menu.Append(ID_STC_FIND_FILES, exEllipsed(_("Find &In Files")));
    }

    if (GetMenuFlags() & STC_MENU_REPORT_REPLACE)
    {
      menu.Append(ID_STC_REPLACE_FILES, exEllipsed(_("&Replace In Files")));
    }
  }

  if (m_FileName.FileExists() && GetSelectedText().empty())
  {
    if (GetMenuFlags() & STC_MENU_COMPARE)
    {
      menu.AppendSeparator();

      if (!exApp::GetConfigBool("SVN"))
      {
        menu.Append(ID_STC_COMPARE, exEllipsed(_("&Compare Recent Version")));
      }
      else
      {
        wxMenu* svnmenu = new wxMenu;
        svnmenu->Append(ID_STC_SVN_DIFF, exEllipsed(_("&Diff")));
        svnmenu->Append(ID_STC_SVN_LOG, exEllipsed(_("&Log")));
        svnmenu->Append(ID_STC_SVN_CAT, exEllipsed(_("&Cat")));
        menu.Append(-1, "&SVN", wxEmptyString, wxITEM_NORMAL, svnmenu);
      }
    }
  }
}

bool ftSTC::Initialize()
{
  wxWindow* window = wxTheApp->GetTopWindow();
  m_Frame = wxDynamicCast(window, ftFrame);

  if (m_Frame == NULL)
  {
    wxLogError("Cannot initialize ftSTC without ftFrame");
    return false;
  }

#if wxUSE_DRAG_AND_DROP
  // Now DnD normal text inside the editor does not work.
  // Adding a wxTextDropTarget works a little, but does not move text.
  SetDropTarget(new ftSTCDropTarget(m_Frame));
#endif

  return true;
}

void ftSTC::OnCommand(wxCommandEvent& command)
{
  if (!Continue())
  {
    return;
  }

  if (command.GetId() > ID_TOOL_LOWEST && command.GetId() < ID_TOOL_HIGHEST)
  {
    const exTool tool(command.GetId());

    if (ftTextFile::SetupTool(tool))
    {
      ftTextFile report(m_FileName);
      report.RunTool();
      report.GetStatistics().Log();
    }

    return;
  }

  switch (command.GetId())
  {
  case ID_STC_COMPARE:
    {
      wxFileName lastfile;
      if (ftFindOtherFileName(m_FileName, NULL, &lastfile))
        ftCompareFile(m_FileName, lastfile);
    }
    break;

  case ID_STC_SVN_CAT: exSVN(SVN_CAT).Show(m_FileName.GetFullPath()); break;
  case ID_STC_SVN_DIFF: exSVN(SVN_DIFF).Show(m_FileName.GetFullPath()); break;
  case ID_STC_SVN_LOG: exSVN(SVN_LOG).Show(m_FileName.GetFullPath()); break;

  case ID_STC_FIND_FILES:
    GetSearchText();
    ftFindInFiles(m_Frame);
    break;

  case ID_STC_REPLACE_FILES:
    GetSearchText();
    ftFindInFiles(m_Frame, true);
    break;

  default: wxLogError(FILE_INFO("Unhandled"));
    break;
  }
}

bool ftSTC::Open(
  const exFileName& filename,
  int line_number,
  const wxString& match,
  long flags)
{
  bool retValue;

  if (flags & (STC_OPEN_FROM_LINK | STC_OPEN_FROM_STATISTICS))
  {
    retValue = m_Frame->OpenFile(filename, line_number, match, flags);
  }
  else
  {
    retValue = exSTC::Open(filename, line_number, match, flags);

    if (retValue)
    {
      m_Frame->SetRecentFile(filename.GetFullPath());
    }
  }

  return retValue;
}

void ftSTC::PropertiesMessage()
{
  exSTC::PropertiesMessage();

  const wxString ro = (m_FileName.GetStat().IsReadOnly() ?
    " [" + _("Readonly") + "]":
    wxString(wxEmptyString));

  m_Frame->SetTitle(m_FileName.GetFullPath() + ro, wxEmptyString);
}
