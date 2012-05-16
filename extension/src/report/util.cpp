////////////////////////////////////////////////////////////////////////////////
// Name:      util.cpp
// Purpose:   Implementation of wxExtension report utility functions
// Author:    Anton van Wezenbeek
// Copyright: (c) 2012 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/regex.h>
#include <wx/extension/filedlg.h>
#include <wx/extension/listitem.h>
#include <wx/extension/util.h>
#include <wx/extension/report/util.h>
#include <wx/extension/report/defs.h>
#include <wx/extension/report/dir.h>
#include <wx/extension/report/frame.h>
#include <wx/extension/report/listviewfile.h>
#include <wx/extension/report/textfile.h>

bool wxExForEach(wxAuiNotebook* notebook, int id, const wxFont& font)
{
  for (
	// no auto, should be int
    int page = notebook->GetPageCount() - 1;
    page >= 0;
    page--)
  {
    // Required by wxExFileDialog.
    wxExListViewFile* lv = (wxExListViewFile*)notebook->GetPage(page);

    if (lv == NULL)
    {
      return false;
    }

    switch (id)
    {
    case ID_LIST_ALL_ITEMS:
      {
        if (font.IsOk())
        {
          lv->SetFont(font);
        }

        lv->ItemsUpdate();
      }
      break;

    case ID_LIST_ALL_CLOSE:
      {
      wxExFileDialog dlg(notebook, lv);
      if (dlg.ShowModalIfChanged() == wxID_CANCEL) return false;
      if (!notebook->DeletePage(page)) return false;
      }
      break;

    default: wxFAIL;
    }
  }

  return true;
}

bool wxExMake(wxExFrameWithHistory* frame, const wxFileName& makefile)
{
  return frame->GetProcess()->Execute(
    wxConfigBase::Get()->Read("Make", "make") + " " +
      wxConfigBase::Get()->Read("MakeSwitch", "-f") + " " +
      makefile.GetFullPath(),
    wxEXEC_ASYNC,
    makefile.GetPath());
}

const wxExFileStatistics wxExRun(const wxExListItem& item, const wxExTool& tool)
{
  wxLogStatus(item.GetFileName().GetFullPath());

  if (item.GetFileName().FileExists())
  {
    wxExTextFileWithListView file(item.GetFileName(), tool);
    file.RunTool();
    return file.GetStatistics();
  }
  else
  {
    wxExDirTool dir(tool, item.GetFileName().GetFullPath(), item.GetFileSpec());
    dir.FindFiles();

    return dir.GetStatistics();
  }
}
