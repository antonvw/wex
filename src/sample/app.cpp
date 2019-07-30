////////////////////////////////////////////////////////////////////////////////
// Name:      app.cpp
// Purpose:   Implementation of wex sample classes
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <chrono>
#include <numeric>
#include <functional>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/aboutdlg.h>
#include <wx/numdlg.h>
#include <wex/defs.h>
#include <wex/filedlg.h>
#include <wex/itemdlg.h>
#include <wex/lexers.h>
#include <wex/printing.h>
#include <wex/stcdlg.h>
#include <wex/toolbar.h>
#include <wex/util.h>
#include <wex/vcs.h>
#include <wex/version.h>
#include "app.h"
#ifndef __WXMSW__
#include "app.xpm"
#endif

#include "../test/test-configitem.h"
#include "../test/test-item.h"

enum
{
  ID_DLG_CONFIG_ITEM = wex::ID_EDIT_HIGHEST + 1,
  ID_DLG_CONFIG_ITEM_COL,
  ID_DLG_CONFIG_ITEM_READONLY,
  ID_DLG_ITEM,
  ID_DLG_LISTVIEW,
  ID_DLG_STC_CONFIG,
  ID_DLG_STC_ENTRY,
  ID_DLG_VCS,
  ID_RECENTFILE_MENU,
  ID_SHOW_VCS,
  ID_STATISTICS_SHOW,
  ID_STC_FLAGS,
  ID_STC_SPLIT,
};

wxIMPLEMENT_APP(sample_app);

bool sample_app::OnInit()
{
  SetAppName("wex-sample");

  if (!wex::app::OnInit())
  {
    return false;
  }

  sample_frame *frame = new sample_frame();
  frame->Show(true);
  
  wex::log::status("Locale") << get_locale().GetLocale() << "dir" << get_catalog_dir();

  return true;
}

sample_dir::sample_dir(
  const std::string& fullpath, const std::string& findfiles, wex::grid* grid)
  : wex::dir(fullpath, findfiles)
  , m_Grid(grid)
{
}

bool sample_dir::on_file(const wex::path& file)
{
  m_Grid->AppendRows(1);
  const auto no = m_Grid->GetNumberRows() - 1;
  m_Grid->SetCellValue(no, 0, wxString::Format("cell%d", no));
  m_Grid->SetCellValue(no, 1, file.string());

  // Let's make these cells readonly and colour them, so we can test
  // things like cutting and dropping is forbidden.
  m_Grid->SetReadOnly(no, 1);
  m_Grid->SetCellBackgroundColour(no, 1, *wxLIGHT_GREY);
  return true;
}

sample_frame::sample_frame()
  : wex::managed_frame(4)
  , m_Process(new wex::process())
  , m_Notebook(new wex::notebook())
  , m_STC(new wex::stc())
  , m_Shell(new wex::shell(wex::stc_data(), ">", "\n"))
  , m_STCLexers(new wex::stc(wex::lexers::get()->get_filename()))
{
  wex::process::prepare_output(this);
  
  SetIcon(wxICON(app));

  auto* menuFile = new wex::menu;
  menuFile->append(wxID_OPEN);
  file_history().use_menu(ID_RECENTFILE_MENU, menuFile);
  menuFile->append_separator();
  menuFile->append(ID_SHOW_VCS, "Show VCS");
  menuFile->append_print();
  menuFile->append_separator();
  menuFile->append(wxID_EXECUTE);
  menuFile->append(wxID_STOP);
  menuFile->append_separator();
  menuFile->append(wxID_EXIT);

  auto *menuEdit = new wex::menu();
  menuEdit->append(wxID_UNDO);
  menuEdit->append(wxID_REDO);
  menuEdit->append_separator();
  menuEdit->append(wxID_CUT);
  menuEdit->append(wxID_COPY);
  menuEdit->append(wxID_PASTE);
  menuEdit->append_separator();
  menuEdit->append(wxID_JUMP_TO);
  menuEdit->append_separator();
  wex::menu* menuFind = new wex::menu();
  menuFind->append(wxID_FIND);
  menuFind->append(wxID_REPLACE);
  menuEdit->append_submenu(menuFind, _("&Find And Replace"));
  
  auto* menuDialog = new wex::menu;
  menuDialog->append(ID_DLG_ITEM, wex::ellipsed("Item Dialog"));
  menuDialog->append_separator();
  menuDialog->append(ID_DLG_CONFIG_ITEM, wex::ellipsed("Config Dialog"));
  menuDialog->append(ID_DLG_CONFIG_ITEM_COL, wex::ellipsed("Config Dialog Columns"));
  menuDialog->append(ID_DLG_CONFIG_ITEM_READONLY, wex::ellipsed("Config Dialog Readonly"));
  menuDialog->append_separator();
  menuDialog->append(ID_DLG_LISTVIEW, wex::ellipsed("List Dialog"));
  menuDialog->append_separator();
  menuDialog->append(ID_DLG_STC_CONFIG, wex::ellipsed("STC Dialog"));
  menuDialog->append(ID_DLG_STC_ENTRY, wex::ellipsed("STC Entry Dialog"));
  menuDialog->append_separator();
  menuDialog->append(ID_DLG_VCS, wex::ellipsed("VCS Dialog"));

  auto* menuSTC = new wex::menu;
  menuSTC->append(ID_STC_FLAGS, wex::ellipsed("Open Flag"));
  menuSTC->append_separator();
  menuSTC->append(ID_STC_SPLIT, "Split");

  auto *menuView = new wex::menu;
  append_panes(menuView);
  menuView->append_separator();
  menuView->append(ID_STATISTICS_SHOW, "Statistics");
  
  auto* menuHelp = new wex::menu;
  menuHelp->append(wxID_ABOUT);

  auto *menubar = new wxMenuBar;
  menubar->Append(menuFile, "&File");
  menubar->Append(menuEdit, "&Edit");
  menubar->Append(menuView, "&View");
  menubar->Append(menuDialog, "&Dialog");
  menubar->Append(menuSTC, "&STC");
  menubar->Append(menuHelp, "&Help");
  SetMenuBar(menubar);

  m_Grid = new wex::grid(wex::window_data().parent(m_Notebook));
  m_ListView = new wex::listview(wex::window_data().parent(m_Notebook));

  manager().AddPane(m_Notebook, 
    wxAuiPaneInfo().CenterPane().MinSize(wxSize(250, 250)));
  manager().AddPane(m_STC, 
    wxAuiPaneInfo().Bottom().Caption("STC"));
  manager().AddPane(m_Shell, 
    wxAuiPaneInfo().Bottom().Caption("Shell").MinSize(wxSize(250, 250)));
  manager().AddPane(m_Process->get_shell(), wxAuiPaneInfo()
    .Bottom()
    .Name("PROCESS")
    .MinSize(250, 100)
    .Caption(_("Process")));

  manager().Update();

  m_Notebook->add_page(m_STCLexers, wex::lexers::get()->get_filename().fullname());
  m_Notebook->add_page(m_ListView, "wex::listview");

  m_Notebook->add_page(m_Grid, "wex::grid");
  m_Grid->CreateGrid(0, 0);
  m_Grid->AppendCols(2);
  sample_dir dir(wex::path::current(), "*.*", m_Grid);
  dir.find_files();
  m_Grid->AutoSizeColumns();

  m_ListView->append_columns({
    {"String", wex::column::STRING},
    {"Number", wex::column::INT},
    {"Float", wex::column::FLOAT},
    {"Date", wex::column::DATE}});

  const int items = 50;

  for (auto i = 0; i < items; i++)
  {
    m_ListView->insert_item({
      "item " + std::to_string(i),
      std::to_string(i),
      std::to_string((float)i / 2.0),
      wxDateTime::Now().Format("%c").ToStdString()});

    // Set some images.
    if      (i == 0) m_ListView->set_item_image(i, wxART_CDROM);
    else if (i == 1) m_ListView->set_item_image(i, wxART_REMOVABLE);
    else if (i == 2) m_ListView->set_item_image(i, wxART_FOLDER);
    else if (i == 3) m_ListView->set_item_image(i, wxART_FOLDER_OPEN);
    else if (i == 4) m_ListView->set_item_image(i, wxART_GO_DIR_UP);
    else if (i == 5) m_ListView->set_item_image(i, wxART_EXECUTABLE_FILE);
    else if (i == 6) m_ListView->set_item_image(i, wxART_NORMAL_FILE);
    else             m_ListView->set_item_image(i, wxART_TICK_MARK);
  }

  setup_statusbar({
    {"PaneFileType", 50, "File type"},
    {"PaneInfo", 100, "Lines or items"},
    {"PaneLexer", 60}});

  get_toolbar()->add_controls();
  get_options_toolbar()->add_controls();
  
  // The OnCommand keeps statistics.
  Bind(wxEVT_MENU, &sample_frame::OnCommand, this, wxID_COPY);
  Bind(wxEVT_MENU, &sample_frame::OnCommand, this, wxID_CUT);
  Bind(wxEVT_MENU, &sample_frame::OnCommand, this, wxID_EXECUTE);
  Bind(wxEVT_MENU, &sample_frame::OnCommand, this, wxID_JUMP_TO);
  Bind(wxEVT_MENU, &sample_frame::OnCommand, this, wxID_PASTE);
  Bind(wxEVT_MENU, &sample_frame::OnCommand, this, wxID_OPEN, wxID_SAVEAS);
  Bind(wxEVT_MENU, &sample_frame::OnCommand, this, wxID_UNDO, wxID_REDO);
  
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    wxAboutDialogInfo info;
    info.SetIcon(GetIcon());
    info.SetVersion(wex::get_version_info().get());
    info.SetCopyright(wex::get_version_info().copyright());
    wxAboutBox(info);}, wxID_ABOUT);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    Close(true);}, wxID_EXIT);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    m_ListView->print();}, wxID_PRINT);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    m_ListView->print_preview();}, wxID_PREVIEW);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    wex::printing::get()->get_html_printer()->PageSetup();}, wxID_PRINT_SETUP);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    const long val = wxGetNumberFromUser("Input columns:",
      wxEmptyString, _("Columns"), 1, 1, 100);
    if (val >= 0)
    {
      wex::item_dialog(test_config_items(0, val), 
        wex::window_data().title("Config Dialog Columns"), 0, val).ShowModal();
    }}, ID_DLG_CONFIG_ITEM_COL);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    wex::item_dialog* dlg = new wex::item_dialog(test_config_items(0, 1), 
      wex::window_data().
        title("Config Dialog").
        button(wxAPPLY | wxCANCEL).
#ifdef __WXMSW__    
        size(wxSize(500, 500)));
#else
        size(wxSize(600, 600)));
#endif    
    //  dlg->force_checkbox_checked("Group", "Checkboxes");
    dlg->Show();}, ID_DLG_CONFIG_ITEM);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    wex::item_dialog(test_config_items(0, 1), 
      wex::window_data().
        button(wxCANCEL).
        title("Config Dialog Readonly"), 0, 4).ShowModal();}, ID_DLG_CONFIG_ITEM_READONLY);
      
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    wex::item_dialog(test_items()).ShowModal();}, ID_DLG_ITEM);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    m_ListView->config_dialog();}, ID_DLG_LISTVIEW);
  
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    wex::stc::config_dialog(
      wex::window_data().button(wxAPPLY | wxCANCEL));}, ID_DLG_STC_CONFIG);
      
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    std::string text;
    for (auto i = 0; i < 100; i++)
    {
      text += wxString::Format("Hello from line: %d\n", i);
    }
    wex::stc_entry_dialog(
      text,      
      "Greetings from " + std::string(wxTheApp->GetAppDisplayName()),
      wex::window_data().title("Hello world")).ShowModal();
    }, ID_DLG_STC_ENTRY);
      
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    wex::vcs().config_dialog();}, ID_DLG_VCS);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    m_Shell->prompt(
      "\nHello '" + event.GetString().ToStdString() + "' from the shell");}, wex::ID_SHELL_COMMAND);
      
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    wxFileDialog dlg(this, _("Open File"), "", "",
      "All files (*.*)|*.*", wxFD_OPEN|wxFD_FILE_MUST_EXIST);
    if (dlg.ShowModal() == wxID_CANCEL) return;
    const wex::vcs vcs(std::vector< wex::path > {dlg.GetPath().ToStdString()});
    wex::stc_entry_dialog(vcs.name()).ShowModal();}, ID_SHOW_VCS);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    if (m_Notebook->set_selection("Statistics") == nullptr)
    {
      m_Notebook->add_page(m_Statistics.show(m_Notebook), "Statistics");
    }}, ID_STATISTICS_SHOW);
  
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    const long value = wxGetNumberFromUser("Input:",
      wxEmptyString, "STC Open Flag",
      m_FlagsSTC,
      0,
      0xFFFF);
    if (value != -1)
    {
      m_FlagsSTC = value;
    }}, ID_STC_FLAGS);
      
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    m_Process->execute();}, wxID_EXECUTE);
  
  Bind(wxEVT_MENU, &sample_frame::OnCommand, this, ID_STC_SPLIT);
  
  Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
    event.Enable(
      (get_listview() != nullptr && get_listview()->GetItemCount() > 0) ||
      (get_stc() != nullptr && get_stc()->GetLength() > 0));}, wxID_PRINT);
  
  Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
    event.Enable(
      (get_listview() != nullptr && get_listview()->GetItemCount() > 0) ||
      (get_stc() != nullptr && get_stc()->GetLength() > 0));}, wxID_PREVIEW);
}

void sample_frame::OnCommand(wxCommandEvent& event)
{
  m_Statistics.inc(std::to_string(event.GetId()));

  auto* editor = get_stc();
  auto* grid = get_grid();
  auto* listview = get_listview();


  switch (event.GetId())
  {
    case wxID_NEW:
      m_STC->get_file().file_new(wex::path());
      break;
    case wxID_OPEN:
      {
      wex::file_dialog dlg(&m_STC->get_file());
      if (dlg.show_modal_if_changed(true) == wxID_CANCEL) return;
      const auto start = std::chrono::system_clock::now();
      m_STC->open(dlg.GetPath().ToStdString(), 
        wex::stc_data().flags((wex::stc_data::window_t)m_FlagsSTC));
      const auto milli = std::chrono::duration_cast
        <std::chrono::milliseconds>(std::chrono::system_clock::now() - start);
      wex::log::status("Open") 
         << milli.count() << "milliseconds" << m_STC->GetTextLength() << "bytes";
      }
      break;
    case wxID_SAVE:
      m_STC->get_file().file_save();
  
      if (m_STC->get_filename().data() == wex::lexers::get()->get_filename().data())
      {
        wex::lexers::get()->load_document();
        wex::log::verbose("File contains") << wex::lexers::get()->get_lexers().size() << "lexers";
          // As the lexer might have changed, update status bar field as well.
        update_statusbar(m_STC, "PaneLexer");
      }
      break;
  
    case wxID_COPY:
    case wxID_CUT:
    case wxID_JUMP_TO:
    case wxID_PASTE:
    case wxID_REDO:
    case wxID_UNDO:
      if (editor != nullptr)
      {
        wxPostEvent(editor, event);
      }
      else if (grid != nullptr)
      {
        wxPostEvent(grid, event);
      }
      else if (listview != nullptr)
      {
        wxPostEvent(listview, event);
      }
    break;

    case ID_STC_SPLIT:
      if (editor != nullptr)
      {
        wex::stc* stc = new wex::stc(editor->get_filename(), 
          wex::stc_data().window(wex::window_data().parent(m_Notebook)));
        m_Notebook->add_page(
          stc,
          "stc" + std::to_string(stc->GetId()),
          m_STC->get_filename().fullname());
        stc->SetDocPointer(m_STC->GetDocPointer());
      }
      break;
      
    default:
      assert(0);
      break;
    }
}

void sample_frame::on_command_item_dialog(
  wxWindowID dialogid,
  const wxCommandEvent& event)
{
  if (dialogid == wxID_PREFERENCES)
  {
    if (event.GetId() != wxID_CANCEL)
    {
      m_STC->config_get();
      m_STCLexers->config_get();
    }
  }
  else if (event.GetId() >= 1000 && event.GetId() < 1050)
  {
    wex::log::verbose("button") << event.GetId() << "checked:" << event.IsChecked();
  }
  else
  {
    wex::managed_frame::on_command_item_dialog(dialogid, event);
  }
}
