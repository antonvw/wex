/******************************************************************************\
* File:          filetool.cpp
* Purpose:       Implementation of filetool classes
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2008 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/stdpaths.h> // strangely enough, for wxTheFileIconsTable
#include <wx/generic/dirctrlg.h> // for wxTheFileIconsTable
#include <wx/filetool/filetool.h>

bool ftCompareFile(const wxFileName& file1, const wxFileName& file2)
{
  const wxString comparator = exApp::GetConfig(_("Comparator"));

  if (comparator.empty())
  {
    wxLogMessage(_("Please add a comparator"));
    return false;
  }

  const wxString arguments =
     (file1.GetModificationTime() < file2.GetModificationTime()) ?
       "\"" + file1.GetFullPath() + "\" \"" + file2.GetFullPath() + "\"":
       "\"" + file2.GetFullPath() + "\" \"" + file1.GetFullPath() + "\"";

  if (wxExecute(comparator + " " + arguments) == 0)
  {
    return false;
  }

  const wxString msg = _("Compared") + ": " + arguments;
  exApp::Log(msg);
  exStatusText(msg);

  return true;
}

void ftFindInFiles(ftFrame* frame, bool replace)
{
  std::vector<exConfigItem> v;
  v.push_back(exConfigItem(_("Find what"), CONFIG_COMBOBOX, wxEmptyString, true));
  if (replace) v.push_back(exConfigItem(_("Replace with"), CONFIG_COMBOBOX));
  v.push_back(exConfigItem(_("In files"), CONFIG_COMBOBOX, wxEmptyString, true));
  v.push_back(exConfigItem(_("In folder"), CONFIG_COMBOBOXDIR, wxEmptyString, true));
  v.push_back(exConfigItem());
  v.push_back(exConfigItem(_("Match whole word"), CONFIG_CHECKBOX));
  v.push_back(exConfigItem());
  v.push_back(exConfigItem(_("Match case"), CONFIG_CHECKBOX));

  if (exConfigDialog(NULL,
    v,
    (replace ? _("Replace In Files"): _("Find In Files"))).ShowModal() == wxID_CANCEL)
  {
    return;
  }

  ftListView* output =
    frame->Activate(replace ? ftListView::LIST_REPLACE : ftListView::LIST_FIND);
  if (output == NULL) return;

  exApp::GetConfig()->GetFindReplaceData()->Update();

  const exTool tool =
    (replace ?
       ID_TOOL_REPORT_REPLACE:
       ID_TOOL_REPORT_FIND);

  if (!ftTextFile::SetupTool(tool))
  {
    return;
  }

  ftFindLog(replace);

  ftDir dir(
    output,
    exApp::GetConfig(_("In folder")),
    exApp::GetConfig(_("In files")));

  dir.RunTool();
  dir.GetStatistics().Log();
}

void ftFindLog(bool replace)
{
  wxString log = _("Searching for") + ": " + exApp::GetConfig(_("Find what"));

  if (replace)
  {
    log += " " + _("Replacing with") + ": " + exApp::GetConfig(_("Replace with"));
  }

  exApp::Log(log);
}

bool ftFindOtherFileName(
  const wxFileName& filename,
  ftListView* listview,
  wxFileName* lastfile)
{
  /* Add the base version if present. E.g.
  fullpath: F:\CCIS\v990308\com\atis\atis-ctrl\atis-ctrl.cc
  base:  F:\CCIS\
  append:   \com\atis\atis-ctrl\atis-ctrl.cc
  */
  const wxString fullpath = filename.GetFullPath();

  wxRegEx reg("[/|\\][a-z-]*[0-9]+\\.?[0-9]*\\.?[0-9]*\\.?[0-9]*");

  if (!reg.Matches(fullpath.Lower()))
  {
    exFrame::StatusText(_("No version information found"));
    return false;
  }

  size_t start, len;
  if (!reg.GetMatch(&start, &len))
  {
    exFrame::StatusText("No match after all");
    return false;
  }

  wxString base = fullpath.substr(0, start);
  if (!wxEndsWithPathSeparator(base))
  {
    base += wxFileName::GetPathSeparator();
  }

  wxDir dir(base);

  if (!dir.IsOpened())
  {
    exFrame::StatusText("Could not open base dir: " + base);
    return false;
  }

  wxString filename_string;
  bool cont = dir.GetFirst(&filename_string, wxEmptyString, wxDIR_DIRS); // only get dirs

  wxDateTime lastmodtime((time_t)0);
  const wxString append = fullpath.substr(start + len);

  bool found = false;

  // TODO: Use a thread for this.
  while (cont)
  {
    wxFileName fn(base + filename_string + append);

    if (fn.FileExists() &&
        fn.GetPath().CmpNoCase(filename.GetPath()) != 0 &&
        fn.GetModificationTime() != filename.GetModificationTime())
    {
      found = true;

      if (listview == NULL && lastfile == NULL)
      {
        // We are only interested in return value, so speed it up.
        return true;
      }

      if (listview != NULL)
      {
        ftListItem item(listview, fn.GetFullPath());
        item.Insert();
      }

      if (lastfile != NULL)
      {
        if (fn.GetModificationTime() > lastmodtime)
        {
          lastmodtime = fn.GetModificationTime();
          *lastfile = fn;
        }
      }
    }

    cont = dir.GetNext(&filename_string);

    wxTheApp->Yield();
  }

  if (!found && (listview != NULL || lastfile != NULL))
  {
    exFrame::StatusText(_("No files found"));
  }

  return found;
}

bool ftForEach(wxAuiNotebook* notebook, int id, const wxFont& font)
{
  for (
    size_t page = 0;
    page < notebook->GetPageCount();
    page++)
  {
    ftListView* lv = (ftListView*)notebook->GetPage(page);

    if (lv == NULL)
    {
      wxLogError(FILE_INFO("Notebook page: %d (%s) is not an ftListView"),
        page,
        notebook->GetPageText(page).c_str());
      return false;
    }

    if (id >= wxID_VIEW_DETAILS &&  id <= wxID_VIEW_LIST)
    {
      long view = 0;
      switch (id)
      {
      case wxID_VIEW_DETAILS: view = wxLC_REPORT; break;
      case wxID_VIEW_LIST: view = wxLC_LIST; break;
      case wxID_VIEW_SMALLICONS: view = wxLC_SMALL_ICON; break;
      default: wxLogError(FILE_INFO("Unhandled"));
      }

      lv->SetSingleStyle(view);
      exApp::GetConfig()->Set("List/Style", view);
    }
    else
    {
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
      if (!lv->Continue()) return false;
      if (!notebook->DeletePage(page)) return false;
      break;

    default: wxLogError("Unhandled: %d (ID_TOOL_LOWEST: %d ID_TOOL_SQL: %d ID_LIST_LOWEST: %d",
      id, (int)ID_TOOL_LOWEST, (int)ID_TOOL_SQL, (int)ID_LIST_LOWEST);
    }
    }
  }

  return true;
}

int ftGetFileIcon(const exFileName* filename)
{
  if (filename->GetStat().IsOk() && !filename->GetExt().empty())
  {
    // README: DirExists from wxFileName is not okay, so use the static one here!
    return
      (wxFileName::DirExists(filename->GetFullPath()) ?
       wxFileIconsTable::folder:
       wxTheFileIconsTable->GetIconID(filename->GetExt()));
  }
  else
  {
    return wxFileIconsTable::computer;
  }
}

void ftOpenFiles(
  ftFrame* frame,
  const wxArrayString& files,
  long flags)
{
  for (size_t i = 0; i < files.GetCount(); i++)
  {
    wxString file = files[i];

    if (file.Contains("*") || file.Contains("?"))
    {
      ftDir dir(frame, wxGetCwd(), file, flags);
      dir.FindFiles();
    }
    else
    {
      int line = 0;

      if (file.Contains(":"))
      {
        line = atoi(files[i].AfterFirst(':').c_str());

        if (line != 0) // this indicates an error in the number
        {
          file = file.BeforeFirst(':');
       }
      }

      wxFileName filename(file);

      if (!filename.FileExists()) filename.Normalize();
      if (!filename.FileExists())
      {
        wxLogError("File does not exist: " + filename.GetFullPath());
        continue;
      }

      frame->OpenFile(filename.GetFullPath(), line, wxEmptyString, flags);
    }
  }
}

ftDir::ftDir(ftListView* listview,
  const wxString& fullpath, const wxString& filespec, wxStatusBar* statusbar)
  : exDir(fullpath, filespec, statusbar)
  , m_Statistics(fullpath)
  , m_Frame(NULL)
  , m_ListView(listview)
  , m_Flags(0)
  , m_RunningTool(false)
{
}

ftDir::ftDir(ftFrame* frame,
  const wxString& fullpath, const wxString& filespec, long flags,
  wxStatusBar* statusbar)
  : exDir(fullpath, filespec, statusbar)
  , m_Statistics(fullpath)
  , m_Frame(frame)
  , m_ListView(NULL)
  , m_Flags(flags)
  , m_RunningTool(false)
{
}

bool ftDir::Cancelled()
{
  // TODO: Implement.
  return false;
}

void ftDir::OnFile(const wxString& file)
{
  if (m_RunningTool)
  {
    const exFileName filename(file);

    if (filename.FileExists())
    {
      ftTextFile report(filename);
      report.RunTool();
      m_Statistics += report.GetStatistics();
    }
  }
  else
  {
    if (m_Frame != NULL)
    {
      if (wxFileName::DirExists(file)) return;
      m_Frame->OpenFile(file, 0, wxEmptyString, m_Flags);
    }
    else
    {
      if (m_ListView != NULL)
      {
        ftListItem item(m_ListView, file, GetFileSpec());
        item.Insert();

        // Don't move next code into insert, as it itself inserts!
        if (m_ListView->GetType() == ftListView::LIST_VERSION)
        {
          ftListItem item(m_ListView, m_ListView->GetItemCount() - 1);

          ftTextFile report(item.m_Statistics);
          if (report.SetupTool(ID_TOOL_REVISION_RECENT))
          {
            report.RunTool();
            item.UpdateRevisionList(report.GetRCS());
          }
        }
      }
    }
  }
}

size_t ftDir::RunTool(int flags)
{
  m_RunningTool = true;

  size_t result = FindFiles(flags);

  m_RunningTool = false;

  return result;
}

BEGIN_EVENT_TABLE(ftFind, wxComboBox)
  EVT_CHAR(ftFind::OnKey)
  EVT_MENU(wxID_DELETE, ftFind::OnCommand)
END_EVENT_TABLE()

ftFind::ftFind(
  wxWindow* parent,
  ftFrame* frame,
  wxWindowID id,
  const wxPoint& pos,
  const wxSize& size)
  : wxComboBox(parent, id, wxEmptyString, pos, size)
  , m_Frame(frame)
{
  const int accels = 1;
  wxAcceleratorEntry entries[accels];
  entries[0].Set(wxACCEL_NORMAL, WXK_DELETE, wxID_DELETE);
  wxAcceleratorTable accel(accels, entries);
  SetAcceleratorTable(accel);

  const wxFont font(
    exApp::GetConfig("FindFont", 8),
    wxFONTFAMILY_DEFAULT,
    wxFONTSTYLE_NORMAL,
    wxFONTWEIGHT_NORMAL);

  SetFont(font);

  exComboBoxFromString(this, exApp::GetConfig("FindReplace/FindStrings"));

  // And override the value set by previous, as we want text to be same as in Find.
  SetValue(exApp::GetConfig()->GetFindReplaceData()->GetFindString());
}

void ftFind::OnCommand(wxCommandEvent& event)
{
  // README: The delete key default behaviour does not delete the char right from insertion point.
  // Instead, the event is sent to the editor and a char is deleted from the editor.
  // Therefore implement the delete here.
  if (event.GetId() == wxID_DELETE)
  {
    Remove(GetInsertionPoint(), GetInsertionPoint() + 1);
  }
  else
  {
    event.Skip();
  }
}

void ftFind::OnKey(wxKeyEvent& event)
{
  event.Skip();

  const int key = event.GetKeyCode();

  if (key == WXK_RETURN)
  {
    ftSTC* stc = m_Frame->GetCurrentSTC();

    if (stc != NULL)
    {
      stc->FindNext(GetValue());
      exApp::GetConfig()->GetFindReplaceData()->SetFindString(GetValue());

      // And keep the changed text in the combo box.
      wxString text;

      if (exComboBoxToString(this, text))
      {
        exApp::GetConfig()->Set("FindReplace/FindStrings", text);
        Clear(); // so exComboBoxFromString can append again
        exComboBoxFromString(this, text);
      }
    }
  }
}
