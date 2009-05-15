/******************************************************************************\
* File:          stc.cpp
* Purpose:       Implementation of class 'wxExSTCWithFrame'
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

BEGIN_EVENT_TABLE(wxExSTCWithFrame, wxExSTC)
  EVT_MENU_RANGE(ID_STC_LOWEST, ID_STC_HIGHEST, wxExSTCWithFrame::OnCommand)
  EVT_MENU_RANGE(ID_TOOL_LOWEST, ID_TOOL_HIGHEST, wxExSTCWithFrame::OnCommand)
END_EVENT_TABLE()

#if wxUSE_DRAG_AND_DROP
class STCDropTarget : public wxFileDropTarget
{
public:
  STCDropTarget(wxExFrameWithHistory* owner) {m_Owner = owner;}
private:
  virtual bool OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& filenames)
  {
    wxExOpenFiles(m_Owner, filenames);
    return true;
  }

  wxExFrameWithHistory* m_Owner;
};
#endif

wxExSTCWithFrame::wxExSTCWithFrame(wxWindow* parent,
  long type,
  const wxString& value,
  wxWindowID id,
  const wxPoint& pos,
  const wxSize& size,
  long style,
  const wxString& name)
  : wxExSTC(parent, type, value, id, pos, size, style, name)
{
  Initialize();
}

wxExSTCWithFrame::wxExSTCWithFrame(wxWindow* parent,
  const wxExFileName& filename,
  int line_number,
  const wxString& match,
  long flags,
  long type,
  wxWindowID id,
  const wxPoint& pos,
  const wxSize& size,
  long style,
  const wxString& name)
  : wxExSTC(parent, filename, line_number, match, flags, type, id, pos, size, style, name)
{
  if (Initialize())
  {
    m_Frame->SetRecentFile(GetFileName().GetFullPath());
  }
}

wxExSTCWithFrame::wxExSTCWithFrame(const wxExSTC& stc)
  : wxExSTC(stc)
{
  Initialize();
}

void wxExSTCWithFrame::BuildPopupMenu(wxExMenu& menu)
{
  wxExSTC::BuildPopupMenu(menu);

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
      menu.Append(ID_STC_FIND_FILES, wxExEllipsed(_("Find &In Files")));
    }

    if (GetMenuFlags() & STC_MENU_REPORT_REPLACE)
    {
      menu.Append(ID_STC_REPLACE_FILES, wxExEllipsed(_("&Replace In Files")));
    }
  }

  if (m_FileName.FileExists() && GetSelectedText().empty())
  {
    if (GetMenuFlags() & STC_MENU_COMPARE_OR_SVN)
    {
      wxFileName path (GetFileName().GetPath());
      path.AppendDir(".svn");
        
      if (path.DirExists())
      {
        wxMenu* svnmenu = new wxMenu;
        svnmenu->Append(ID_STC_SVN_DIFF, wxExEllipsed(_("&Diff")));
        svnmenu->Append(ID_STC_SVN_LOG, wxExEllipsed(_("&Log")));
        svnmenu->Append(ID_STC_SVN_CAT, wxExEllipsed(_("&Cat")));
        svnmenu->Append(ID_STC_SVN_BLAME, wxExEllipsed(_("&Blame")));
        svnmenu->AppendSeparator();
        svnmenu->Append(ID_STC_SVN_COMMIT, wxExEllipsed(_("&Commit")));
        menu.AppendSeparator();
        menu.AppendSubMenu(svnmenu, "&SVN");
      }
      else if (!wxExApp::GetConfig(_("Comparator")).empty())
      {
        menu.AppendSeparator();
        menu.Append(ID_STC_COMPARE, wxExEllipsed(_("&Compare Recent Version")));
      }
    }
  }

  if (!GetReadOnly() &&
      !m_FileName.GetLexer().GetScintillaLexer().empty())
  {
    menu.AppendSeparator();
    menu.Append(ID_STC_ADD_HEADER, wxExEllipsed(_("&Add Header")));
  }
}

bool wxExSTCWithFrame::Initialize()
{
  wxWindow* window = wxTheApp->GetTopWindow();
  m_Frame = wxDynamicCast(window, wxExFrameWithHistory);

  if (m_Frame == NULL)
  {
    wxFAIL;
    return false;
  }

#if wxUSE_DRAG_AND_DROP
  // Now DnD normal text inside the editor does not work.
  // Adding a wxTextDropTarget works a little, but does not move text.
  SetDropTarget(new STCDropTarget(m_Frame));
#endif

  return true;
}

void wxExSTCWithFrame::OnCommand(wxCommandEvent& command)
{
  if (!Continue())
  {
    return;
  }

  if (command.GetId() > ID_TOOL_LOWEST && command.GetId() < ID_TOOL_HIGHEST)
  {
    const wxExTool tool(command.GetId());

    if (wxExTextFileWithListView::SetupTool(tool))
    {
      wxExTextFileWithListView report(m_FileName, tool);
      report.RunTool();
      report.GetStatistics().Log(tool);

      if (tool.IsCount())
      {
        wxExOpenFile(
          wxExFileNameStatistics::GetLogfileName(),
          STC_OPEN_FROM_STATISTICS | STC_OPEN_IS_SYNCED);
      }
    }

    return;
  }

  switch (command.GetId())
  {
  case ID_STC_ADD_HEADER:
    {
    std::vector<wxExConfigItem> v;

    // Purpose is required.
    v.push_back(wxExConfigItem(_("Purpose"), wxEmptyString, wxTE_MULTILINE, true));

    // Author is required, but only presented if empty.
    // Email and License also are only presented if Author empty.
    if (wxExApp::GetConfig(_("Author")).empty())
    {
      v.push_back(wxExConfigItem(_("Author"), wxEmptyString, 0, true));

      if (wxExApp::GetConfig(_("Email")).empty())
      {
        v.push_back(wxExConfigItem(_("Email")));
      }

      if (wxExApp::GetConfig(_("License")).empty())
      {
        v.push_back(wxExConfigItem(_("License")));
      }
    }

    wxExConfigDialog dlg(this, wxExApp::GetConfig(), v, _("File Purpose"));
    if (dlg.ShowModal() == wxID_CANCEL) return;

    DocumentStart();
    AddText(wxExHeader(m_FileName, wxExApp::GetConfig(), wxExApp::GetConfig(_("Purpose"))));
    }
    break;

  case ID_STC_COMPARE:
    {
      wxFileName lastfile;

      if (wxExFindOtherFileName(m_FileName, NULL, &lastfile))
      {
        wxExCompareFile(m_FileName, lastfile);
      }
    }
    break;

  case ID_STC_SVN_CAT:
    {
    wxExSVN svn(SVN_CAT, m_FileName.GetFullPath());

    if (svn.Execute() == 0)
    {
      m_Frame->OpenFile(m_FileName, svn.GetOutput());
    }
    }
    break;

  case ID_STC_SVN_BLAME: wxExSVN(SVN_BLAME, m_FileName.GetFullPath()).ExecuteAndShowOutput(); break;
  case ID_STC_SVN_COMMIT: wxExSVN(SVN_COMMIT, m_FileName.GetFullPath()).ExecuteAndShowOutput(); break;
  case ID_STC_SVN_DIFF: wxExSVN(SVN_DIFF, m_FileName.GetFullPath()).ExecuteAndShowOutput(); break;
  case ID_STC_SVN_LOG: wxExSVN(SVN_LOG, m_FileName.GetFullPath()).ExecuteAndShowOutput(); break;

  case ID_STC_FIND_FILES:
    GetSearchText();
    wxExFindInFiles(m_Frame);
    break;

  case ID_STC_REPLACE_FILES:
    GetSearchText();
    wxExFindInFiles(m_Frame, true);
    break;

  default: wxFAIL;
    break;
  }
}

bool wxExSTCWithFrame::Open(
  const wxExFileName& filename,
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
    retValue = wxExSTC::Open(filename, line_number, match, flags);

    if (retValue)
    {
      m_Frame->SetRecentFile(filename.GetFullPath());
    }
  }

  return retValue;
}

void wxExSTCWithFrame::PropertiesMessage()
{
  wxExSTC::PropertiesMessage();

  const wxString ro = (m_FileName.GetStat().IsReadOnly() ?
    " [" + _("Readonly") + "]":
    wxString(wxEmptyString));

  m_Frame->SetTitle(m_FileName.GetFullPath() + ro, wxEmptyString);
}
