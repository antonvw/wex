/******************************************************************************\
* File:          util.cpp
* Purpose:       Implementation of wxExtension report utility functions and classes
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/extension/configdlg.h>
#include <wx/extension/filedlg.h>
#include <wx/extension/report/report.h>

bool wxExCompareFile(const wxFileName& file1, const wxFileName& file2)
{
  if (wxExApp::GetConfig(_("Comparator")).empty())
  {
    return false;
  }

  const wxString arguments =
     (file1.GetModificationTime() < file2.GetModificationTime()) ?
       "\"" + file1.GetFullPath() + "\" \"" + file2.GetFullPath() + "\"":
       "\"" + file2.GetFullPath() + "\" \"" + file1.GetFullPath() + "\"";

  if (wxExecute(wxExApp::GetConfig(_("Comparator")) + " " + arguments) == 0)
  {
    return false;
  }

  const wxString msg = _("Compared") + ": " + arguments;
  wxExApp::Log(msg);
#if wxUSE_STATUSBAR
  wxExFrame::StatusText(msg);
#endif

  return true;
}

void wxExFindInFiles(bool replace)
{
  if (wxExDir::GetIsBusy())
  {
    wxExDir::Cancel();
#if wxUSE_STATUSBAR
    wxExFrame::StatusText(_("Cancelled previous find files"));
#endif
  }

  // To initialize the combobox.
  wxExApp::GetConfig(_("In files"), wxExApp::GetLexers()->BuildComboBox());

  std::vector<wxExConfigItem> v;
  v.push_back(
    wxExConfigItem(wxExApp::GetFindReplaceData()->GetTextFindWhat(), 
    CONFIG_COMBOBOX, 
    wxEmptyString, 
    true));

  if (replace) 
  {
    v.push_back(wxExConfigItem(
      wxExApp::GetFindReplaceData()->GetTextReplaceWith(), 
      CONFIG_COMBOBOX));
  }

  v.push_back(wxExConfigItem(_("In files"), CONFIG_COMBOBOX, wxEmptyString, true));
  v.push_back(wxExConfigItem(_("In folder"), CONFIG_COMBOBOXDIR, wxEmptyString, true));
  v.push_back(wxExConfigItem());
  v.push_back(wxExConfigItem(wxExApp::GetFindReplaceData()->GetInfo()));

  if (wxExConfigDialog(NULL,
    wxConfigBase::Get(),
    v,
    (replace ? _("Replace In Files"): _("Find In Files"))).ShowModal() == wxID_CANCEL)
  {
    return;
  }

  const wxExTool tool =
    (replace ?
       ID_TOOL_REPORT_REPLACE:
       ID_TOOL_REPORT_FIND);

  if (!wxExTextFileWithListView::SetupTool(tool))
  {
    return;
  }

  wxExApp::Log(wxExApp::GetFindReplaceData()->GetText(replace));

  wxExDirWithListView dir(
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
      long view = 0;
      switch (id)
      {
      case wxID_VIEW_DETAILS: view = wxLC_REPORT; break;
      case wxID_VIEW_LIST: view = wxLC_LIST; break;
      case wxID_VIEW_SMALLICONS: view = wxLC_SMALL_ICON; break;
      default: wxFAIL;
      }

      lv->SetSingleStyle(view);
      wxConfigBase::Get()->Write("List/Style", view);
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
      if (!dlg.Continue()) return false;
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

  wxSetWorkingDirectory(makefile.GetPath());

  const bool ret = frame->ProcessRun(
    wxExApp::GetConfig("Make", "make") + " " +
    wxExApp::GetConfig("MakeSwitch", "-f") + " " +
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
        line = atoi(files[i].AfterFirst(':').c_str());

        if (line != 0) // this indicates an error in the number
        {
          file = file.BeforeFirst(':');
        }
      }

      frame->OpenFile(file, line, wxEmptyString, file_flags);
    }
  }
}

void wxExOpenFilesDialog(
  wxExFrameWithHistory* frame,
  long style,
  const wxString wildcards,
  bool ask_for_continue)
{
  wxExSTC* stc = frame->GetSTC();
  wxArrayString files;

  if (stc != NULL)
  {
    wxExFileDialog dlg(frame,
      stc,
      _("Select Files"),
      wxFileSelectorDefaultWildcardStr,
      style);

    if (dlg.ShowModal(ask_for_continue) == wxID_CANCEL) return;
    dlg.GetPaths(files);
  }
  else
  {
    wxFileDialog dlg(frame,
      _("Select Files"),
      wxEmptyString,
      wxEmptyString,
      wildcards,
      style);
    if (dlg.ShowModal() == wxID_CANCEL) return;
    dlg.GetPaths(files);
  }

  wxExOpenFiles(frame, files);
}

wxExDirWithListView::wxExDirWithListView(const wxExTool& tool,
  const wxString& fullpath, const wxString& filespec, int flags)
  : wxExDir(fullpath, filespec, flags)
  , m_Statistics(fullpath)
  , m_Frame(NULL)
  , m_ListView(NULL)
  , m_Flags(0)
  , m_Tool(tool)
{
}

wxExDirWithListView::wxExDirWithListView(wxExListViewFile* listview,
  const wxString& fullpath, const wxString& filespec, int flags)
  : wxExDir(fullpath, filespec, flags)
  , m_Statistics(fullpath)
  , m_Frame(NULL)
  , m_ListView(listview)
  , m_Flags(0)
  , m_Tool(ID_TOOL_LOWEST)
{
}

wxExDirWithListView::wxExDirWithListView(wxExFrameWithHistory* frame,
  const wxString& fullpath, 
  const wxString& filespec, 
  long file_flags,
  int dir_flags)
  : wxExDir(fullpath, filespec, dir_flags)
  , m_Statistics(fullpath)
  , m_Frame(frame)
  , m_ListView(NULL)
  , m_Flags(file_flags)
  , m_Tool(ID_TOOL_LOWEST)
{
}

void wxExDirWithListView::OnDir(const wxString& dir)
{
  if (m_ListView != NULL)
  {
    wxExListItemWithFileName(m_ListView, dir, GetFileSpec()).Insert();
  }
}

void wxExDirWithListView::OnFile(const wxString& file)
{
  if (m_Frame == NULL && m_ListView == NULL)
  {
    const wxExFileName filename(file);

    if (filename.GetStat().IsOk())
    {
      wxExTextFileWithListView report(filename, m_Tool);
      report.RunTool();
      m_Statistics += report.GetStatistics();
    }
  }
  else
  {
    if (m_Frame != NULL)
    {
      m_Frame->OpenFile(file, 0, wxEmptyString, m_Flags);
    }
    else if (m_ListView != NULL)
    {
      wxExListItemWithFileName item(m_ListView, file, GetFileSpec());
      item.Insert();

      // Don't move next code into insert, as it itself inserts!
      if (m_ListView->GetType() == wxExListViewWithFrame::LIST_VERSION)
      {
        wxExListItemWithFileName item(m_ListView, m_ListView->GetItemCount() - 1);

        wxExTextFileWithListView report(item.m_Statistics, ID_TOOL_REVISION_RECENT);
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
  SetValue(wxExApp::GetFindReplaceData()->GetFindString());
}

void ComboBox::OnCommand(wxCommandEvent& event)
{
  // README: The delete key default behaviour does not delete the char right from insertion point.
  // Instead, the event is sent to the editor and a char is deleted from the editor.
  // Therefore implement the delete here.
  switch (event.GetId())
  {
  case wxID_DELETE:
    Remove(GetInsertionPoint(), GetInsertionPoint() + 1);
    break;
  default:
    wxFAIL;
    break;
  }
}

void ComboBox::OnKey(wxKeyEvent& event)
{
  const int key = event.GetKeyCode();

  if (key == WXK_RETURN)
  {
    wxExSTC* stc = m_Frame->GetSTC();

    if (stc != NULL)
    {
      stc->FindNext(GetValue());
      wxExApp::GetFindReplaceData()->SetFindString(GetValue());

      // And keep the changed text in the combo box.
      wxString text;

      if (wxExComboBoxToString(this, text))
      {
        wxConfigBase::Get()->Write("FindReplace/FindStrings", text);
        Clear(); // so wxExComboBoxFromString can append again
        wxExComboBoxFromString(this, text);
        SetValue(wxExApp::GetFindReplaceData()->GetFindString());
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
  EVT_MENU(wxID_DOWN, wxExFindToolBar::OnCommand)
  EVT_MENU(wxID_UP, wxExFindToolBar::OnCommand)
END_EVENT_TABLE()

wxExFindToolBar::wxExFindToolBar(
  wxWindow* parent,
  wxExFrameWithHistory* frame,
  wxWindowID id)
  : wxAuiToolBar(parent, id)
  , m_Frame(frame)
  , m_MatchCase(new wxCheckBox())
  , m_MatchWholeWord(new wxCheckBox())
  , m_RegularExpression(new wxCheckBox())
{
  wxExApp::GetFindReplaceData()->CreateAndFill(
    this,
    m_MatchCase,
    ID_MATCH_CASE,
    m_MatchWholeWord,
    ID_MATCH_WHOLE_WORD,
    m_RegularExpression,
    ID_REGULAR_EXPRESSION);

#ifdef __WXMSW__
  const wxSize size(150, 20);
#else
  const wxSize size(150, -1);
#endif
  m_ComboBox = new ComboBox(this, frame, ID_FIND_TEXT, wxDefaultPosition, size);

  // And place the controls on the toolbar.
  AddControl(m_ComboBox);
  AddSeparator();

  AddTool(
    wxID_DOWN, 
    wxEmptyString, 
    wxArtProvider::GetBitmap(wxART_GO_DOWN, wxART_TOOLBAR, GetToolBitmapSize()),
    _("Find next"));
  AddTool(
    wxID_UP, 
    wxEmptyString, 
    wxArtProvider::GetBitmap(wxART_GO_UP, wxART_TOOLBAR, GetToolBitmapSize()),
    _("Find previous"));
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
  case wxID_DOWN:
  case wxID_UP:
    {
      wxExSTC* stc = m_Frame->GetSTC();

      if (stc != NULL)
      {
        stc->FindNext(m_ComboBox->GetValue(), (event.GetId() == wxID_DOWN));
      }
    }
    break;

  case ID_MATCH_WHOLE_WORD:
  case ID_MATCH_CASE:
  case ID_REGULAR_EXPRESSION:
    wxExApp::GetFindReplaceData()->SetFromCheckBoxes(
      m_MatchWholeWord,
      m_MatchCase,
      m_RegularExpression);
    break;

  default:
    wxFAIL;
    break;
  }
}
