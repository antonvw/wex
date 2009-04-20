/******************************************************************************\
* File:          util.cpp
* Purpose:       Implementation of filetool classes
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/extension/configdialog.h>
#include <wx/extension/report/util.h>
#include <wx/extension/report/defs.h>
#include <wx/extension/report/filetool.h>
#include <wx/extension/report/frame.h>
#include <wx/extension/report/listview.h>

bool exCompareFile(const wxFileName& file1, const wxFileName& file2)
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
  exFrame::StatusText(msg);

  return true;
}

void exFindInFiles(exFrameWithHistory* frame, bool replace)
{
  // To initialize the combobox.
  exApp::GetConfig(_("In files"), exApp::GetLexers()->BuildComboBox());

  std::vector<exConfigItem> v;
  v.push_back(exConfigItem(_("Find what"), CONFIG_COMBOBOX, wxEmptyString, true));
  if (replace) v.push_back(exConfigItem(_("Replace with"), CONFIG_COMBOBOX));
  v.push_back(exConfigItem(_("In files"), CONFIG_COMBOBOX, wxEmptyString, true));
  v.push_back(exConfigItem(_("In folder"), CONFIG_COMBOBOXDIR, wxEmptyString, true));
  v.push_back(exConfigItem());
  v.push_back(exConfigItem(exApp::GetConfig()->GetFindReplaceData()->GetInfo()));

  if (exConfigDialog(NULL,
    exApp::GetConfig(),
    v,
    (replace ? _("Replace In Files"): _("Find In Files"))).ShowModal() == wxID_CANCEL)
  {
    return;
  }

  exListViewFile* output =
    frame->Activate(replace ? exListViewFile::LIST_REPLACE : exListViewFile::LIST_FIND);
  if (output == NULL) return;

  const exTool tool =
    (replace ?
       ID_TOOL_REPORT_REPLACE:
       ID_TOOL_REPORT_FIND);

  if (!exTextFileWithReport::SetupTool(tool))
  {
    return;
  }

  exApp::Log(exApp::GetConfig()->GetFindReplaceData()->GetText(replace));

  exDirWithReport dir(
    output,
    exApp::GetConfig(_("In folder")),
    exApp::GetConfig(_("In files")));

  dir.RunTool(tool);
  dir.GetStatistics().Log(tool);
}

bool exFindOtherFileName(
  const wxFileName& filename,
  exListViewFile* listview,
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
        exListItemWithFileName item(listview, fn.GetFullPath());
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

bool exForEach(wxAuiNotebook* notebook, int id, const wxFont& font)
{
  for (
    size_t page = 0;
    page < notebook->GetPageCount();
    page++)
  {
    exListViewFile* lv = (exListViewFile*)notebook->GetPage(page);

    if (lv == NULL)
    {
      wxLogError("Notebook page: %d (%s) is not an exListViewFile",
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
      default: wxFAIL;
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

    default: wxFAIL;
    }
    }
  }

  return true;
}

void exOpenFiles(
  exFrameWithHistory* frame,
  const wxArrayString& files,
  long flags)
{
  for (size_t i = 0; i < files.GetCount(); i++)
  {
    wxString file = files[i]; // cannot be const because of file = later on

    if (file.Contains("*") || file.Contains("?"))
    {
      exDirWithReport dir(frame, wxGetCwd(), file, flags);
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

      const exFileName filename(file);
      frame->OpenFile(filename, line, wxEmptyString, flags);
    }
  }
}

exDirWithReport::exDirWithReport(exListViewFile* listview,
  const wxString& fullpath, const wxString& filespec)
  : exDir(fullpath, filespec)
  , m_Statistics(fullpath)
  , m_Frame(NULL)
  , m_ListView(listview)
  , m_Flags(0)
  , m_RunningTool(false)
  , m_Tool(ID_TOOL_LOWEST)
{
}

exDirWithReport::exDirWithReport(exFrameWithHistory* frame,
  const wxString& fullpath, const wxString& filespec, long flags)
  : exDir(fullpath, filespec)
  , m_Statistics(fullpath)
  , m_Frame(frame)
  , m_ListView(NULL)
  , m_Flags(flags)
  , m_RunningTool(false)
  , m_Tool(ID_TOOL_LOWEST)
{
}

void exDirWithReport::OnFile(const wxString& file)
{
  if (m_RunningTool)
  {
    const exFileName filename(file);

    if (filename.GetStat().IsOk())
    {
      if (wxFileName::DirExists(file)) return;
      exTextFileWithReport report(filename);
      report.RunTool(m_Tool);
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
        exListItemWithFileName item(m_ListView, file, GetFileSpec());
        item.Insert();

        // Don't move next code into insert, as it itself inserts!
        if (m_ListView->GetType() == exListViewFile::LIST_VERSION)
        {
          exListItemWithFileName item(m_ListView, m_ListView->GetItemCount() - 1);

          exTextFileWithReport report(item.m_Statistics);
          if (report.SetupTool(ID_TOOL_REVISION_RECENT))
          {
            report.RunTool(ID_TOOL_REVISION_RECENT);
            item.UpdateRevisionList(report.GetRCS());
          }
        }
      }
    }
  }
}

size_t exDirWithReport::RunTool(const exTool& tool, int flags)
{
  m_Tool = tool;

  m_RunningTool = true;

  size_t result = FindFiles(flags);

  m_RunningTool = false;

  return result;
}

/// Offers a find combobox that allows you to find text
/// on a current STC on an exFrameWithHistory.
class ComboBox : public wxComboBox
{
public:
  /// Constructor. Fills the combobox box with values from FindReplace from config.
  ComboBox(
    wxWindow* parent,
    exFrameWithHistory* frame,
    wxWindowID id = wxID_ANY,
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize);
private:
  void OnCommand(wxCommandEvent& event);
  void OnKey(wxKeyEvent& event);
  exFrameWithHistory* m_Frame;

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(ComboBox, wxComboBox)
  EVT_CHAR(ComboBox::OnKey)
  EVT_MENU(wxID_DELETE, ComboBox::OnCommand)
END_EVENT_TABLE()

ComboBox::ComboBox(
  wxWindow* parent,
  exFrameWithHistory* frame,
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

void ComboBox::OnCommand(wxCommandEvent& event)
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

void ComboBox::OnKey(wxKeyEvent& event)
{
  const int key = event.GetKeyCode();

  if (key == WXK_RETURN)
  {
    exSTCWithFrame* stc = m_Frame->GetCurrentSTC();

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
        SetValue(exApp::GetConfig()->GetFindReplaceData()->GetFindString());
      }
    }
  }
  else
  {
    event.Skip();
  }
}

BEGIN_EVENT_TABLE(exFindToolBar, wxAuiToolBar)
  EVT_CHECKBOX(ID_MATCH_WHOLE_WORD, exFindToolBar::OnCommand)
  EVT_CHECKBOX(ID_MATCH_CASE, exFindToolBar::OnCommand)
  EVT_CHECKBOX(ID_REGULAR_EXPRESSION, exFindToolBar::OnCommand)
END_EVENT_TABLE()

exFindToolBar::exFindToolBar(
  wxWindow* parent,
  exFrameWithHistory* frame,
  wxWindowID id)
  : wxAuiToolBar(parent, id)
{
  m_MatchCase = new wxCheckBox(this, ID_MATCH_CASE, _("Match case"));
  m_MatchWholeWord = new wxCheckBox(this, ID_MATCH_WHOLE_WORD, _("Match whole word"));
  m_RegularExpression = new wxCheckBox(this, ID_REGULAR_EXPRESSION, _("Regular expression"));

  m_MatchCase->SetValue(exApp::GetConfig()->GetFindReplaceData()->MatchCase());
  m_MatchWholeWord->SetValue(exApp::GetConfig()->GetFindReplaceData()->MatchWord());
  m_RegularExpression->SetValue(exApp::GetConfig()->GetFindReplaceData()->IsRegularExpression());

#ifdef __WXMSW__
  const wxSize size(150, 20);
#else
  const wxSize size(150, -1);
#endif
  AddControl(new ComboBox(this, frame, ID_FIND_TEXT, wxDefaultPosition, size));
  AddSeparator();
  AddControl(m_MatchWholeWord);
  AddControl(m_MatchCase);
  AddControl(m_RegularExpression);

  Realize();
}

void exFindToolBar::OnCommand(wxCommandEvent& event)
{
  switch (event.GetId())
  {
  case ID_MATCH_WHOLE_WORD:
  case ID_MATCH_CASE:
  case ID_REGULAR_EXPRESSION:
    exApp::GetConfig()->SetFindReplaceData(
      m_MatchWholeWord->GetValue(),
      m_MatchCase->GetValue(),
      m_RegularExpression->GetValue());
    break;
  }
}
