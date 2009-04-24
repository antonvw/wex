/******************************************************************************\
* File:          stc.cpp
* Purpose:       Implementation of class 'exSTCWithFrame'
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/extension/configdialog.h>
#include <wx/extension/extension.h>
#include <wx/extension/svn.h>
#include <wx/extension/report/stc.h>
#include <wx/extension/report/defs.h>
#include <wx/extension/report/frame.h>
#include <wx/extension/report/listview.h>
#include <wx/extension/report/textfile.h>
#include <wx/extension/report/util.h>

BEGIN_EVENT_TABLE(exSTCWithFrame, exSTC)
  EVT_MENU_RANGE(ID_STC_LOWEST, ID_STC_HIGHEST, exSTCWithFrame::OnCommand)
  EVT_MENU_RANGE(ID_TOOL_LOWEST, ID_TOOL_HIGHEST, exSTCWithFrame::OnCommand)
END_EVENT_TABLE()

#if wxUSE_DRAG_AND_DROP
class STCDropTarget : public wxFileDropTarget
{
public:
  STCDropTarget(exFrameWithHistory* owner) {m_Owner = owner;}
private:
  virtual bool OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& filenames)
  {
    exOpenFiles(m_Owner, filenames);
    return true;
  }

  exFrameWithHistory* m_Owner;
};
#endif

exSTCWithFrame::exSTCWithFrame(wxWindow* parent,
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

exSTCWithFrame::exSTCWithFrame(wxWindow* parent,
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

exSTCWithFrame::exSTCWithFrame(const exSTC& stc)
  : exSTC(stc)
{
  Initialize();
}

void exSTCWithFrame::BuildPopupMenu(exMenu& menu)
{
  exSTC::BuildPopupMenu(menu);

  // Add tools if we have at least some text, the tool flag,
  // and a lexer.
  if (GetSelectedText().empty() && GetTextLength() > 0 &&
     (GetMenuFlags() & STC_MENU_TOOL) &&
      !GetFileName().GetLexer().GetScintillaLexer().empty())
  {
    menu.AppendSeparator();
    menu.AppendTools();
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
    if (GetMenuFlags() & STC_MENU_COMPARE_OR_SVN)
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
        svnmenu->AppendSeparator();
        svnmenu->Append(ID_STC_SVN_COMMIT, exEllipsed(_("&Commit")));
        menu.AppendSubMenu(svnmenu, "&SVN");
      }
    }
  }

  menu.AppendSeparator();
  menu.Append(ID_STC_ADD_HEADER, exEllipsed(_("&Add Header")));
}

bool exSTCWithFrame::Initialize()
{
  wxWindow* window = wxTheApp->GetTopWindow();
  m_Frame = wxDynamicCast(window, exFrameWithHistory);

  if (m_Frame == NULL)
  {
    wxLogError("Cannot initialize exSTCWithFrame without exFrameWithHistory");
    return false;
  }

#if wxUSE_DRAG_AND_DROP
  // Now DnD normal text inside the editor does not work.
  // Adding a wxTextDropTarget works a little, but does not move text.
  SetDropTarget(new STCDropTarget(m_Frame));
#endif

  return true;
}

void exSTCWithFrame::OnCommand(wxCommandEvent& command)
{
  if (!Continue())
  {
    return;
  }

  if (command.GetId() > ID_TOOL_LOWEST && command.GetId() < ID_TOOL_HIGHEST)
  {
    const exTool tool(command.GetId());

    if (tool.GetId() == ID_TOOL_COMMIT)
    {
    wxTextEntryDialog dlg(this,
      _("Input") + ":",
      "Commit",
      exApp::GetConfig(_("Revision comment")));

    if (dlg.ShowModal() == wxID_CANCEL)
    {
      return;
    }

    exApp::GetConfig()->Set(_("Revision comment"), dlg.GetValue());
    }

    if (exTextFileWithReport::SetupTool(tool))
    {
      exTextFileWithReport report(m_FileName);
      report.RunTool(tool);
      report.GetStatistics().Log(tool);

      if (tool.IsCountType())
      {
        exOpenFile(
          exFileNameStatistics::GetLogfileName(), 
          STC_OPEN_FROM_STATISTICS);
      }
    }

    return;
  }

  switch (command.GetId())
  {
  case ID_STC_ADD_HEADER:
    {
    std::vector<exConfigItem> v;
    v.push_back(exConfigItem(_("Purpose"), wxEmptyString, wxTE_MULTILINE, true));

    if (exApp::GetConfig(_("Author")).empty())
    {
      v.push_back(exConfigItem(_("Author"), wxEmptyString, 0, true));
    }

    exConfigDialog dlg(this, exApp::GetConfig(), v, _("File Purpose"));
    if (dlg.ShowModal() == wxID_CANCEL) return;

    DocumentStart();
    AddText(exHeader(m_FileName, exApp::GetConfig(), exApp::GetConfig(_("Purpose"))));
    }
    break;

  case ID_STC_COMPARE:
    {
      wxFileName lastfile;
      if (exFindOtherFileName(m_FileName, NULL, &lastfile))
        exCompareFile(m_FileName, lastfile);
    }
    break;

  case ID_STC_SVN_CAT:
    {
    exSVN svn(SVN_CAT, m_FileName.GetFullPath());

    if (svn.GetInfo() == 0)
    {
      m_Frame->OpenFile(m_FileName, svn.GetContents());
    }
    }
    break;

  case ID_STC_SVN_COMMIT: exSVN(SVN_COMMIT, m_FileName.GetFullPath()).GetInfoAndShowContents(); break;
  case ID_STC_SVN_DIFF: exSVN(SVN_DIFF, m_FileName.GetFullPath()).GetInfoAndShowContents(); break;
  case ID_STC_SVN_LOG: exSVN(SVN_LOG, m_FileName.GetFullPath()).GetInfoAndShowContents(); break;

  case ID_STC_FIND_FILES:
    GetSearchText();
    exFindInFiles(m_Frame);
    break;

  case ID_STC_REPLACE_FILES:
    GetSearchText();
    exFindInFiles(m_Frame, true);
    break;

  default: wxFAIL;
    break;
  }
}

bool exSTCWithFrame::Open(
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

void exSTCWithFrame::PropertiesMessage()
{
  exSTC::PropertiesMessage();

  const wxString ro = (m_FileName.GetStat().IsReadOnly() ?
    " [" + _("Readonly") + "]":
    wxString(wxEmptyString));

  m_Frame->SetTitle(m_FileName.GetFullPath() + ro, wxEmptyString);
}
