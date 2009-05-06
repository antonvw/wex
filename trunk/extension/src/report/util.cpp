/******************************************************************************\
* File:          util.cpp
* Purpose:       Implementation of wxextension report utility functions and classes
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
#include <wx/extension/report/frame.h>
#include <wx/extension/report/listitem.h>
#include <wx/extension/report/listview.h>
#include <wx/extension/report/stc.h>
#include <wx/extension/report/textfile.h>

bool wxExCompareFile(const wxFileName& file1, const wxFileName& file2)
{
  const wxString comparator = wxExApp::GetConfig(_("Comparator"));

  if (comparator.empty())
  {
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
  wxExApp::Log(msg);
  wxExFrame::StatusText(msg);

  return true;
}

void wxExFindInFiles(wxExFrameWithHistory* frame, bool replace)
{
  // To initialize the combobox.
  wxExApp::GetConfig(_("In files"), wxExApp::GetLexers()->BuildComboBox());

  std::vector<wxExConfigItem> v;
  v.push_back(wxExConfigItem(_("Find what"), CONFIG_COMBOBOX, wxEmptyString, true));
  if (replace) v.push_back(wxExConfigItem(_("Replace with"), CONFIG_COMBOBOX));
  v.push_back(wxExConfigItem(_("In files"), CONFIG_COMBOBOX, wxEmptyString, true));
  v.push_back(wxExConfigItem(_("In folder"), CONFIG_COMBOBOXDIR, wxEmptyString, true));
  v.push_back(wxExConfigItem());
  v.push_back(wxExConfigItem(wxExApp::GetConfig()->GetFindReplaceData()->GetInfo()));

  if (wxExConfigDialog(NULL,
    wxExApp::GetConfig(),
    v,
    (replace ? _("Replace In Files"): _("Find In Files"))).ShowModal() == wxID_CANCEL)
  {
    return;
  }

  const wxExTool tool =
    (replace ?
       ID_TOOL_REPORT_REPLACE:
       ID_TOOL_REPORT_FIND);

  if (!wxExTextFileWithReport::SetupTool(tool))
  {
    return;
  }

  wxExApp::Log(wxExApp::GetConfig()->GetFindReplaceData()->GetText(replace));

  wxExDirWithReport dir(
    tool,
    wxExApp::GetConfig(_("In folder")),
    wxExApp::GetConfig(_("In files")));

  dir.FindFiles();
  dir.GetStatistics().Log(tool);
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
    wxExFrame::StatusText(_("No version information found"));
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

    wxTheApp->Yield();
  }

  if (!found && (listview != NULL || lastfile != NULL))
  {
    wxExFrame::StatusText(_("No files found"));
  }

  return found;
}

bool wxExForEach(wxAuiNotebook* notebook, int id, const wxFont& font)
{
  for (
    size_t page = 0;
    page < notebook->GetPageCount();
    page++)
  {
    wxExListViewFile* lv = (wxExListViewFile*)notebook->GetPage(page);

    if (lv == NULL)
    {
      wxLogError("Notebook page: %d (%s) is not an wxExListViewFile",
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
      wxExApp::GetConfig()->Set("List/Style", view);
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

void wxExOpenFiles(
  wxExFrameWithHistory* frame,
  const wxArrayString& files,
  long flags)
{
  for (size_t i = 0; i < files.GetCount(); i++)
  {
    wxString file = files[i]; // cannot be const because of file = later on

    if (file.Contains("*") || file.Contains("?"))
    {
      wxExDirWithReport dir(frame, wxGetCwd(), file, flags);
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

      const wxExFileName filename(file);
      frame->OpenFile(filename, line, wxEmptyString, flags);
    }
  }
}

wxExDirWithReport::wxExDirWithReport(const wxExTool& tool,
  const wxString& fullpath, const wxString& filespec)
  : wxExDir(fullpath, filespec)
  , m_Statistics(fullpath)
  , m_Frame(NULL)
  , m_ListView(NULL)
  , m_Flags(0)
  , m_Tool(tool)
{
}

wxExDirWithReport::wxExDirWithReport(wxExListViewFile* listview,
  const wxString& fullpath, const wxString& filespec)
  : wxExDir(fullpath, filespec)
  , m_Statistics(fullpath)
  , m_Frame(NULL)
  , m_ListView(listview)
  , m_Flags(0)
  , m_Tool(ID_TOOL_LOWEST)
{
}

wxExDirWithReport::wxExDirWithReport(wxExFrameWithHistory* frame,
  const wxString& fullpath, const wxString& filespec, long flags)
  : wxExDir(fullpath, filespec)
  , m_Statistics(fullpath)
  , m_Frame(frame)
  , m_ListView(NULL)
  , m_Flags(flags)
  , m_Tool(ID_TOOL_LOWEST)
{
}

void wxExDirWithReport::OnFile(const wxString& file)
{
  if (m_Frame == NULL && m_ListView == NULL)
  {
    const wxExFileName filename(file);

    if (filename.GetStat().IsOk())
    {
      if (wxFileName::DirExists(file)) return;
      wxExTextFileWithReport report(filename, m_Tool);
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
    else if (m_ListView != NULL)
    {
      wxExListItemWithFileName item(m_ListView, file, GetFileSpec());
      item.Insert();

      // Don't move next code into insert, as it itself inserts!
      if (m_ListView->GetType() == wxExListViewFile::LIST_VERSION)
      {
        wxExListItemWithFileName item(m_ListView, m_ListView->GetItemCount() - 1);

        wxExTextFileWithReport report(item.m_Statistics, ID_TOOL_REVISION_RECENT);
        if (report.SetupTool(ID_TOOL_REVISION_RECENT))
        {
          report.RunTool();
          item.UpdateRevisionList(report.GetRCS());
        }
      }
    }
  }
}

/// Offers a find combobox that allows you to find text
/// on a current STC on an wxExFrameWithHistory.
class ComboBox : public wxComboBox
{
public:
  /// Constructor. Fills the combobox box with values from FindReplace from config.
  ComboBox(
    wxWindow* parent,
    wxExFrameWithHistory* frame,
    wxWindowID id = wxID_ANY,
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize);
private:
  void OnCommand(wxCommandEvent& event);
  void OnKey(wxKeyEvent& event);
  wxExFrameWithHistory* m_Frame;

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(ComboBox, wxComboBox)
  EVT_CHAR(ComboBox::OnKey)
  EVT_MENU(wxID_DELETE, ComboBox::OnCommand)
END_EVENT_TABLE()

ComboBox::ComboBox(
  wxWindow* parent,
  wxExFrameWithHistory* frame,
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
    wxExApp::GetConfig("FindFont", 8),
    wxFONTFAMILY_DEFAULT,
    wxFONTSTYLE_NORMAL,
    wxFONTWEIGHT_NORMAL);

  SetFont(font);

  wxExComboBoxFromString(this, wxExApp::GetConfig("FindReplace/FindStrings"));

  // And override the value set by previous, as we want text to be same as in Find.
  SetValue(wxExApp::GetConfig()->GetFindReplaceData()->GetFindString());
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
    wxExSTCWithFrame* stc = m_Frame->GetCurrentSTC();

    if (stc != NULL)
    {
      stc->FindNext(GetValue());
      wxExApp::GetConfig()->GetFindReplaceData()->SetFindString(GetValue());

      // And keep the changed text in the combo box.
      wxString text;

      if (wxExComboBoxToString(this, text))
      {
        wxExApp::GetConfig()->Set("FindReplace/FindStrings", text);
        Clear(); // so wxExComboBoxFromString can append again
        wxExComboBoxFromString(this, text);
        SetValue(wxExApp::GetConfig()->GetFindReplaceData()->GetFindString());
      }
    }
  }
  else
  {
    event.Skip();
  }
}

BEGIN_EVENT_TABLE(wxExFindToolBar, wxAuiToolBar)
  EVT_CHECKBOX(ID_MATCH_WHOLE_WORD, wxExFindToolBar::OnCommand)
  EVT_CHECKBOX(ID_MATCH_CASE, wxExFindToolBar::OnCommand)
  EVT_CHECKBOX(ID_REGULAR_EXPRESSION, wxExFindToolBar::OnCommand)
END_EVENT_TABLE()

wxExFindToolBar::wxExFindToolBar(
  wxWindow* parent,
  wxExFrameWithHistory* frame,
  wxWindowID id)
  : wxAuiToolBar(parent, id)
{
  m_MatchCase = new wxCheckBox(this, ID_MATCH_CASE, _("Match case"));
  m_MatchWholeWord = new wxCheckBox(this, ID_MATCH_WHOLE_WORD, _("Match whole word"));
  m_RegularExpression = new wxCheckBox(this, ID_REGULAR_EXPRESSION, _("Regular expression"));

  m_MatchCase->SetValue(wxExApp::GetConfig()->GetFindReplaceData()->MatchCase());
  m_MatchWholeWord->SetValue(wxExApp::GetConfig()->GetFindReplaceData()->MatchWord());
  m_RegularExpression->SetValue(wxExApp::GetConfig()->GetFindReplaceData()->IsRegularExpression());

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

void wxExFindToolBar::OnCommand(wxCommandEvent& event)
{
  switch (event.GetId())
  {
  case ID_MATCH_WHOLE_WORD:
  case ID_MATCH_CASE:
  case ID_REGULAR_EXPRESSION:
    wxExApp::GetConfig()->SetFindReplaceData(
      m_MatchWholeWord->GetValue(),
      m_MatchCase->GetValue(),
      m_RegularExpression->GetValue());
    break;
  }
}
