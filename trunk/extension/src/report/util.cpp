/******************************************************************************\
* File:          util.cpp
* Purpose:       Implementation of wxExtension report utility functions
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/extension/configdlg.h>
#include <wx/extension/filedlg.h>
#include <wx/extension/log.h>
#include <wx/extension/report/report.h>
#include <wx/extension/report/dir.h>

bool wxExCompareFile(const wxFileName& file1, const wxFileName& file2)
{
  if (wxConfigBase::Get()->Read(_("Comparator")).empty())
  {
    return false;
  }

  const wxString arguments =
     (file1.GetModificationTime() < file2.GetModificationTime()) ?
       "\"" + file1.GetFullPath() + "\" \"" + file2.GetFullPath() + "\"":
       "\"" + file2.GetFullPath() + "\" \"" + file1.GetFullPath() + "\"";

  if (wxExecute(wxConfigBase::Get()->Read(_("Comparator")) + " " + arguments) == 0)
  {
    return false;
  }

  const wxString msg = _("Compared") + ": " + arguments;
  wxExLog::Get()->Log(msg);
#if wxUSE_STATUSBAR
  wxExFrame::StatusText(msg);
#endif

  return true;
}

size_t wxExFindInFiles(bool replace)
{
  if (wxExDir::GetIsBusy())
  {
    wxExDir::Cancel();
#if wxUSE_STATUSBAR
    wxExFrame::StatusText(_("Cancelled previous find files"));
#endif
  }

  std::vector<wxExConfigItem> v;
  v.push_back(
    wxExConfigItem(wxExFindReplaceData::Get()->GetTextFindWhat(), 
    CONFIG_COMBOBOX, 
    wxEmptyString, 
    true));

  if (replace) 
  {
    v.push_back(wxExConfigItem(
      wxExFindReplaceData::Get()->GetTextReplaceWith(), 
      CONFIG_COMBOBOX));
  }
  
  const wxString in_files = _("In files");
  const wxString in_folder = _("In folder");

  v.push_back(wxExConfigItem(in_files, CONFIG_COMBOBOX, wxEmptyString, true));
  v.push_back(wxExConfigItem(in_folder, CONFIG_COMBOBOXDIR, wxEmptyString, true));
  v.push_back(wxExConfigItem());

  if (replace) 
  {
    // Match whole word does not work with replace.
    std::set<wxString> s;
    s.insert(wxExFindReplaceData::Get()->GetTextMatchCase());
    s.insert(wxExFindReplaceData::Get()->GetTextRegEx());
    v.push_back(wxExConfigItem(s));
  }
  else
  {
    v.push_back(wxExConfigItem(wxExFindReplaceData::Get()->GetInfo()));
  }

  if (wxExConfigDialog(NULL,
    v,
    (replace ? _("Replace In Files"): _("Find In Files"))).ShowModal() == wxID_CANCEL)
  {
    return 0;
  }

  const wxExTool tool =
    (replace ?
       ID_TOOL_REPORT_REPLACE:
       ID_TOOL_REPORT_FIND);

  if (!wxExTextFileWithListView::SetupTool(tool))
  {
    return 0;
  }

  wxExLog::Get()->Log(wxExFindReplaceData::Get()->GetText(replace));

  wxExDirWithListView dir(
    tool,
    wxExConfigFirstOf(in_folder),
    wxExConfigFirstOf(in_files));

  const size_t result = dir.FindFiles();
  dir.GetStatistics().Log(tool);
  
  return result;
}

bool wxExFindOtherFileName(
  const wxFileName& filename,
  wxExListViewFile* listview,
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
#if wxUSE_STATUSBAR
    wxExFrame::StatusText(_("No version information found"));
#endif
    return false;
  }

  size_t start, len;
  if (!reg.GetMatch(&start, &len))
  {
    wxFAIL;
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
    wxFAIL;
    return false;
  }

  wxString filename_string;
  bool cont = dir.GetFirst(&filename_string, wxEmptyString, wxDIR_DIRS); // only get dirs

  wxDateTime lastmodtime((time_t)0);
  const wxString append = fullpath.substr(start + len);

  bool found = false;

  // Readme: Maybe use a thread for this.
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
        wxExListItemWithFileName item(listview, fn.GetFullPath());
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

    if (wxTheApp != NULL)
    {
      wxTheApp->Yield();
    }
  }

  if (!found && (listview != NULL || lastfile != NULL))
  {
#if wxUSE_STATUSBAR
    wxExFrame::StatusText(_("No files found"));
#endif
  }

  return found;
}

bool wxExForEach(wxAuiNotebook* notebook, int id, const wxFont& font)
{
  for (
    int page = notebook->GetPageCount() - 1;
    page >= 0;
    page--)
  {
    wxExListViewWithFrame* lv = (wxExListViewWithFrame*)notebook->GetPage(page);

    if (lv == NULL)
    {
      wxFAIL;
      return false;
    }

    if (id >= wxID_VIEW_DETAILS &&  id <= wxID_VIEW_LIST)
    {
      lv->SetStyle(id);
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
      {
      wxExFileDialog dlg(notebook, lv);
      if (dlg.ShowModalIfChanged() == wxID_CANCEL) return false;
      if (!notebook->DeletePage(page)) return false;
      }
      break;

    default: wxFAIL;
    }
    }
  }

  return true;
}

bool wxExMake(wxExFrameWithHistory* frame, const wxFileName& makefile)
{
  const wxString cwd = wxGetCwd();

  if (!wxSetWorkingDirectory(makefile.GetPath()))
  {
      wxLogError(_("Cannot set working directory"));
      return false;
  }

  const bool ret = frame->ProcessRun(
    wxConfigBase::Get()->Read("Make", "make") + " " +
    wxConfigBase::Get()->Read("MakeSwitch", "-f") + " " +
    makefile.GetFullPath());

  wxSetWorkingDirectory(cwd);

  return ret;
}

void wxExOpenFiles(
  wxExFrameWithHistory* frame,
  const wxArrayString& files,
  long file_flags,
  int dir_flags)
{
  // std::vector gives compile error.
  for (size_t i = 0; i < files.GetCount(); i++)
  {
    wxString file = files[i]; // cannot be const because of file = later on

    if (file.Contains("*") || file.Contains("?"))
    {
      wxExDirWithListView dir(frame, wxGetCwd(), file, file_flags, dir_flags);
      dir.FindFiles();
    }
    else
    {
      int line = 0;

      if (file.Contains(":"))
      {
        line = atoi(file.AfterFirst(':').c_str());
        file = file.BeforeFirst(':');
      }

      frame->OpenFile(file, line, wxEmptyString, file_flags);
    }
  }
}

void wxExOpenFilesDialog(
  wxExFrameWithHistory* frame,
  long style,
  const wxString& wildcards,
  bool ask_for_continue)
{
  wxExSTC* stc = frame->GetSTC();
  wxArrayString files;

  const wxString caption(_("Select Files"));
      
  if (stc != NULL)
  {
    wxExFileDialog dlg(frame,
      stc,
      caption,
      wildcards,
      style);

    if (ask_for_continue)
    {
      if (dlg.ShowModalIfChanged(true) == wxID_CANCEL) return;
    }
    else
    {
      if (dlg.ShowModal() == wxID_CANCEL) return;
    }

    dlg.GetPaths(files);
  }
  else
  {
    wxFileDialog dlg(frame,
      caption,
      wxEmptyString,
      wxEmptyString,
      wildcards,
      style);
    if (dlg.ShowModal() == wxID_CANCEL) return;
    dlg.GetPaths(files);
  }

  wxExOpenFiles(frame, files);
}
