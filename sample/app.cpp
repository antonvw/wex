////////////////////////////////////////////////////////////////////////////////
// Name:      app.cpp
// Purpose:   Implementation of wex sample classes
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/aboutdlg.h>
#include <wx/generic/numdlgg.h>

#ifndef __WXMSW__
#include "app.xpm"
#endif

#include "../test/ui/test-configitem.h"
#include "../test/ui/test-item.h"
#include "app.h"

#define PRINT_COMPONENT(ID)                                                \
  wxEVT_UPDATE_UI,                                                         \
    [=, this](wxUpdateUIEvent& event)                                      \
  {                                                                        \
    event.Enable(                                                          \
      (get_listview() != nullptr && get_listview()->GetItemCount() > 0) || \
      (get_stc() != nullptr && get_stc()->GetLength() > 0));               \
  },                                                                       \
    ID

enum
{
  ID_DLG_CONFIG_ITEM = wex::del::ID_HIGHEST + 1,
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

wxIMPLEMENT_APP(app);

bool app::OnInit()
{
  SetAppName("wex-sample");

  if (wex::data::cmdline c(argc, argv);
      !wex::cmdline().parse(c) || !wex::app::OnInit())
  {
    return false;
  }

  auto* f = new frame();
  f->Show(true);

  wex::log::status("Locale")
    << get_locale().GetLocale().ToStdString() << "dir" << get_catalog_dir();

  return true;
}

frame::frame()
  : m_notebook(new wex::notebook(wex::data::window().style(
      wxAUI_NB_DEFAULT_STYLE | wxAUI_NB_WINDOWLIST_BUTTON)))
  , m_grid(new wex::grid(wex::data::window().parent(m_notebook)))
  , m_listview(new wex::listview(wex::data::window().parent(m_notebook)))
  , m_process(new wex::process())
  , m_statistics(
      new wex::grid_statistics<int>({}, wex::data::window().parent(m_notebook)))
  , m_shell(new wex::shell(wex::data::stc(), ">"))
  , m_stc(new wex::stc())
  , m_stc_lexers(new wex::stc())
{
  wex::process::prepare_output(this);

  SetIcon(wxICON(app));

  SetMenuBar(new wex::menubar(
    {{new wex::menu(
        {{wxID_OPEN},
         {ID_RECENTFILE_MENU, file_history()},
         {},
         {ID_SHOW_VCS, "Show VCS"},
         {wex::menu_item::PRINT},
         {},
         {wxID_EXECUTE},
         {wxID_STOP},
         {},
         {wex::menu_item::EXIT}}),
      "&File"},
     {new wex::menu(
        {{wxID_UNDO},
         {wxID_REDO},
         {},
         {wxID_CUT},
         {wxID_COPY},
         {wxID_PASTE},
         {},
         {wxID_JUMP_TO},
         {},
         {new wex::menu(
            {{wxID_FIND},
             {wxID_REPLACE},
             {},
             {wex::ID_TOOL_REPORT_FIND, "Find In Files"},
             {wex::ID_TOOL_REPLACE, "Replace In Files"}}),
          _("&Find And Replace")}}),
      "&Edit"},
     {new wex::menu({{this}, {ID_STATISTICS_SHOW, "Statistics"}}), "&View"},
     {new wex::menu(
        {{ID_DLG_ITEM, wex::ellipsed("Item Dialog")},
         {},
         {ID_DLG_CONFIG_ITEM, wex::ellipsed("Config Dialog")},
         {ID_DLG_CONFIG_ITEM_COL, wex::ellipsed("Config Dialog Columns")},
         {ID_DLG_CONFIG_ITEM_READONLY, wex::ellipsed("Config Dialog Readonly")},
         {},
         {ID_DLG_LISTVIEW, wex::ellipsed("List Dialog")},
         {},
         {ID_DLG_STC_CONFIG, wex::ellipsed("STC Dialog")},
         {ID_DLG_STC_ENTRY, wex::ellipsed("STC Entry Dialog")},
         {},
         {ID_DLG_VCS, wex::ellipsed("VCS Dialog")}}),
      "&Dialog"},
     {new wex::menu(
        {{ID_STC_FLAGS, wex::ellipsed("Open Flag")},
         {},
         {ID_STC_SPLIT, "Split"}}),
      "&STC"},
     {new wex::menu({{wxID_ABOUT, ""}}), "&Help"}}));

  pane_add(
    {{m_notebook,
      wxAuiPaneInfo()
        .Name("NOTEBOOK")
        .CloseButton(false)
        .CenterPane()
        .MinSize(wxSize(250, 250))},
     {m_process->get_shell(),
      wxAuiPaneInfo()
        .Bottom()
        .Name("PROCESS")
        .MinSize(250, 100)
        .Caption(_("Process"))
        .CloseButton(false)}});

  m_stc_lexers->open(wex::lexers::get()->path());

  m_notebook->add_page(wex::data::notebook()
                         .page(m_stc_lexers)
                         .key(wex::lexers::get()->path().filename()));
  m_notebook->add_page(
    wex::data::notebook().page(m_listview).key("wex::listview"));
  m_notebook->add_page(wex::data::notebook().page(m_grid).key("wex::grid"));
  m_notebook->add_page(wex::data::notebook().page(m_stc).key("wex::stc"));
  m_notebook->add_page(wex::data::notebook().page(m_shell).key("wex::shell"));

  m_grid->CreateGrid(0, 0);
  m_grid->AppendCols(2);

  dir dir(wex::path::current(), "*.*", m_grid);
  dir.find_files();

  m_grid->AutoSizeColumns();

  m_listview->append_columns(
    {{"String", wex::column::STRING},
     {"Number", wex::column::INT},
     {"Float", wex::column::FLOAT},
     {"Date", wex::column::DATE}});

  const int items = 50;

  for (auto i = 0; i < items; i++)
  {
    m_listview->insert_item(
      {"item " + std::to_string(i),
       std::to_string(i),
       std::to_string((float)i / 2.0),
       wex::now()});

    // Set some images.
    if (i == 0)
      m_listview->set_item_image(i, wxART_CDROM);
    else if (i == 1)
      m_listview->set_item_image(i, wxART_REMOVABLE);
    else if (i == 2)
      m_listview->set_item_image(i, wxART_FOLDER);
    else if (i == 3)
      m_listview->set_item_image(i, wxART_FOLDER_OPEN);
    else if (i == 4)
      m_listview->set_item_image(i, wxART_GO_DIR_UP);
    else if (i == 5)
      m_listview->set_item_image(i, wxART_EXECUTABLE_FILE);
    else if (i == 6)
      m_listview->set_item_image(i, wxART_NORMAL_FILE);
    else
      m_listview->set_item_image(i, wxART_TICK_MARK);
  }

  get_toolbar()->add_standard();
  get_options_toolbar()->add_checkboxes_standard();

  setup_statusbar({{"PaneFileType", 50}, {"PaneInfo", 100}, {"PaneLexer", 60}});

  const wex::lexer lexer(wex::lexers::get()->find("cpp"));

  for (int i = wex::data::listview::FOLDER; i <= wex::data::listview::FILE; i++)
  {
    auto* vw = new wex::del::listview(
      wex::data::listview().type((wex::data::listview::type_t)i).lexer(&lexer));

    m_notebook->add_page(wex::data::notebook()
                           .page(vw)
                           .key(vw->data().type_description())
                           .select());
  }

  {
    wex::dir dir(
      wex::path(wex::path::current()),
      wex::data::dir().file_spec("*.cpp;*.h"),
      (wex::listview*)m_notebook->page_by_key(wex::data::listview()
                                                .type(wex::data::listview::FILE)
                                                .type_description()));

    dir.find_files();

    wex::listitem item(
      (wex::listview*)m_notebook->page_by_key(wex::data::listview()
                                                .type(wex::data::listview::FILE)
                                                .type_description()),
      wex::path("NOT EXISTING ITEM"));

    item.insert();
  }

  wex::bind(this).command(
    {{[=, this](wxCommandEvent& event)
      {
        m_statistics->inc(std::to_string(event.GetId()));
        wxAboutDialogInfo info;
        info.SetIcon(GetIcon());
        info.SetVersion(wex::get_version_info().get());
        info.SetDescription(wex::get_version_info().external_libraries().str());
        info.SetCopyright(wex::get_version_info().copyright());
        wxAboutBox(info);
      },
      wxID_ABOUT},
     {[=, this](wxCommandEvent& event)
      {
        const auto val = wxGetNumberFromUser(
          "Input columns:",
          wxEmptyString,
          _("Columns"),
          1,
          1,
          100);
        if (val >= 0)
        {
          wex::item_dialog(
            test_config_items(0, val),
            wex::data::window().title("Config Dialog Columns"),
            0,
            val)
            .ShowModal();
        }
      },
      ID_DLG_CONFIG_ITEM_COL},
     {[=, this](wxCommandEvent& event)
      {
        auto* dlg = new wex::item_dialog(
          test_config_items(0, 1),
          wex::data::window()
            .title("Config Dialog")
            .button(wxAPPLY | wxCANCEL)
            .
#ifdef __WXMSW__
          size(wxSize(500, 500)));
#else
          size(wxSize(600, 600)));
#endif
        dlg->Show();
      },
      ID_DLG_CONFIG_ITEM},
     {[=, this](wxCommandEvent& event)
      {
        wex::item_dialog(
          test_config_items(0, 1),
          wex::data::window().button(wxCANCEL).title("Config Dialog Readonly"),
          0,
          4)
          .ShowModal();
      },
      ID_DLG_CONFIG_ITEM_READONLY},
     {[=, this](wxCommandEvent& event)
      {
        wex::item_dialog(test_items()).ShowModal();
      },
      ID_DLG_ITEM},
     {[=, this](wxCommandEvent& event)
      {
        m_listview->config_dialog();
      },
      ID_DLG_LISTVIEW},
     {[=, this](wxCommandEvent& event)
      {
        wex::stc::config_dialog(wex::data::window().button(wxAPPLY | wxCANCEL));
      },
      ID_DLG_STC_CONFIG},
     {[=, this](wxCommandEvent& event)
      {
        std::string text;
        for (auto i = 0; i < 100; i++)
        {
          text += wxString::Format("Hello from line: %d\n", i);
        }
        wex::stc_entry_dialog(
          text,
          "Greetings from " + std::string(wxTheApp->GetAppDisplayName()),
          wex::data::window().title("Hello world"))
          .ShowModal();
      },
      ID_DLG_STC_ENTRY},
     {[=, this](wxCommandEvent& event)
      {
        wex::vcs().config_dialog();
      },
      ID_DLG_VCS},
     {[=, this](wxCommandEvent& event)
      {
        m_shell->prompt(
          "\nHello '" + event.GetString().ToStdString() + "' from the shell");
      },
      wex::ID_SHELL_COMMAND},
     {[=, this](wxCommandEvent& event)
      {
        wxFileDialog dlg(
          this,
          _("Open File"),
          "",
          "",
          "All files (*.*)|*.*",
          wxFD_OPEN | wxFD_FILE_MUST_EXIST);
        if (dlg.ShowModal() == wxID_CANCEL)
          return;
        const wex::vcs vcs(
          std::vector<wex::path>{wex::path(dlg.GetPath().ToStdString())});
        wex::stc_entry_dialog(vcs.name()).ShowModal();
      },
      ID_SHOW_VCS},
     {[=, this](wxCommandEvent& event)
      {
        if (m_notebook->set_selection("Statistics") == nullptr)
        {
          wex::data::window data;
          data.parent(m_notebook);
          m_notebook->add_page(wex::data::notebook()
                                 .page(m_statistics->show())
                                 .caption("Statistics")
                                 .key("Statistics"));
        }
      },
      ID_STATISTICS_SHOW},
     {[=, this](wxCommandEvent& event)
      {
        const long value = wxGetNumberFromUser(
          "Input:",
          wxEmptyString,
          "STC Open Flag",
          m_flags_stc,
          0,
          0xFFFF);
        if (value != -1)
        {
          m_flags_stc = value;
        }
      },
      ID_STC_FLAGS},
     {[=, this](wxCommandEvent& event)
      {
        m_process->async_system();
      },
      wxID_EXECUTE}});

  Bind(PRINT_COMPONENT(wxID_PRINT));

  Bind(PRINT_COMPONENT(wxID_PREVIEW));
}

wex::del::listview*
frame::activate(wex::data::listview::type_t type, const wex::lexer* lexer)
{
  for (size_t i = 0; i < m_notebook->GetPageCount(); i++)
  {
    wex::del::listview* vw = (wex::del::listview*)m_notebook->GetPage(i);

    if (vw->data().type() == type)
    {
      return vw;
    }
  }

  return nullptr;
}

bool frame::allow_close(wxWindowID id, wxWindow* page)
{
  if (page == file_history_list() || page == get_listview())
  {
    // prevent possible crash, if set_recent_file tries
    // to add listitem to deleted history list.
    return false;
  }
  else
  {
    return wex::del::frame::allow_close(id, page);
  }
}

wex::listview* frame::get_listview()
{
  return (wex::listview*)m_notebook->GetPage(m_notebook->GetSelection());
}

wex::stc* frame::get_stc()
{
  return m_stc;
}

void frame::on_command(wxCommandEvent& event)
{
  m_statistics->inc(std::to_string(event.GetId()));

  auto* editor   = get_stc();
  auto* grid     = get_grid();
  auto* listview = get_listview();

  switch (event.GetId())
  {
    case wxID_NEW:
      m_stc->get_file().file_new(wex::path());
      break;
    case wxID_OPEN:
    {
      wex::file_dialog dlg(&m_stc->get_file());
      if (dlg.show_modal_if_changed(true) == wxID_CANCEL)
        return;
      const auto start = std::chrono::system_clock::now();
      m_stc->open(
        wex::path(dlg.GetPath().ToStdString()),
        wex::data::stc().flags((wex::data::stc::window_t)m_flags_stc));
      const auto milli = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now() - start);
      wex::log::status("Open")
        << milli.count() << "milliseconds" << m_stc->GetTextLength() << "bytes";
    }
    break;
    case wxID_SAVE:
      m_stc->get_file().file_save();

      if (m_stc->path().data() == wex::lexers::get()->path().data())
      {
        wex::lexers::get()->load_document();
        wex::log::trace("file contains")
          << wex::lexers::get()->get_lexers().size() << "lexers";
        // As the lexer might have changed, update status bar field as well.
        update_statusbar(m_stc, "PaneLexer");
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
        auto* stc = new wex::stc(
          editor->path(),
          wex::data::stc().window(wex::data::window().parent(m_notebook)));
        m_notebook->add_page(wex::data::notebook()
                               .page(stc)
                               .key("stc" + std::to_string(stc->GetId()))
                               .caption(m_stc->path().filename()));
        stc->SetDocPointer(m_stc->GetDocPointer());
      }
      break;

    default:
      assert(0);
      break;
  }
}

void frame::on_command_item_dialog(
  wxWindowID            dialogid,
  const wxCommandEvent& event)
{
  if (dialogid == wxID_PREFERENCES)
  {
    if (event.GetId() != wxID_CANCEL)
    {
      m_stc->config_get();
      m_stc_lexers->config_get();
    }
  }
  else if (event.GetId() >= 1000 && event.GetId() < 1050)
  {
    wex::log::trace("button")
      << event.GetId() << "checked:" << event.IsChecked();
  }
  else
  {
    wex::del::frame::on_command_item_dialog(dialogid, event);
  }
}

wex::stc* frame::open_file(const wex::path& file, const wex::data::stc& data)
{
  m_stc->get_lexer().clear();
  m_stc->open(file, wex::data::stc(data).flags(0));

  return m_stc;
}

dir::dir(const wex::path& path, const std::string& findfiles, wex::grid* grid)
  : wex::dir(path, wex::data::dir().file_spec(findfiles))
  , m_grid(grid)
{
}

bool dir::on_file(const wex::path& file) const
{
  m_grid->AppendRows(1);
  const auto no = m_grid->GetNumberRows() - 1;
  m_grid->SetCellValue(no, 0, wxString::Format("cell%d", no));
  m_grid->SetCellValue(no, 1, file.string());

  // Let's make these cells readonly and colour them, so we can test
  // things like cutting and dropping is forbidden.
  m_grid->SetReadOnly(no, 1);
  m_grid->SetCellBackgroundColour(no, 1, *wxLIGHT_GREY);
  return true;
}
