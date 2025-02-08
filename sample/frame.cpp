////////////////////////////////////////////////////////////////////////////////
// Name:      frame.cpp
// Purpose:   Implementation of wex sample class frame
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/generic/numdlgg.h>

#ifndef __WXMSW__
#include "app.xpm"
#endif

#include "../test/ui/test-configitem.h"
#include "../test/ui/test-item.h"
#include "app.h"
#include "frame.h"

enum
{
  ID_SAMPLE_LOWEST = wex::del::ID_HIGHEST + 1,
  ID_DLG_CONFIG_ITEM,
  ID_DLG_CONFIG_ITEM_COL,
  ID_DLG_CONFIG_ITEM_READONLY,
  ID_DLG_ITEM,
  ID_DLG_LISTVIEW,
  ID_DLG_STC_ENTRY,
  ID_DLG_VCS,
  ID_PROJECT_OPEN,
  ID_RECENTFILE_MENU,
  ID_SHOW_VCS,
  ID_STATISTICS_SHOW,
  ID_STC_FLAGS,
  ID_STC_SPLIT,
  ID_SAMPLE_HIGHEST
};

frame::frame(app* app)
  : m_notebook(new wex::notebook(wex::data::window().style(
      wxAUI_NB_DEFAULT_STYLE | wxAUI_NB_WINDOWLIST_BUTTON)))
  , m_app(app)
  , m_grid(new wex::grid(wex::data::window().parent(m_notebook)))
  , m_listview(new wex::listview(wex::data::window().parent(m_notebook)))
  , m_process(new wex::process())
  , m_statistics(
      new wex::grid_statistics<int>({}, wex::data::window().parent(m_notebook)))
  , m_shell(new wex::shell(
      wex::data::stc().window(wex::data::window().parent(m_notebook)),
      ">"))
{
  wex::process::prepare_output(this);
  m_statistics->show(false);

  SetIcon(wxICON(app));

  SetMenuBar(new wex::menubar(
    {{new wex::menu(
        {{wxID_OPEN},
         {ID_RECENTFILE_MENU, file_history()},
         {ID_PROJECT_OPEN, "Open Project..."},
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
     {new wex::menu({{}, {this}, {ID_STATISTICS_SHOW, "Statistics"}}), "&View"},
     {new wex::menu(
        {{ID_DLG_ITEM, wex::ellipsed("Item Dialog")},
         {},
         {ID_DLG_CONFIG_ITEM, wex::ellipsed("Config Dialog")},
         {ID_DLG_CONFIG_ITEM_COL, wex::ellipsed("Config Dialog Columns")},
         {ID_DLG_CONFIG_ITEM_READONLY, wex::ellipsed("Config Dialog Readonly")},
         {},
         {ID_DLG_LISTVIEW, wex::ellipsed("List Dialog")},
         {},
         {wxID_PREFERENCES},
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

  m_grid->CreateGrid(0, 0);
  m_grid->AppendCols(2);

  m_listview->append_columns(
    {{"String", wex::column::STRING_MEDIUM},
     {"Number", wex::column::INT},
     {"Float", wex::column::FLOAT},
     {"Date", wex::column::DATE}});

  get_toolbar()->add_standard();
  get_options_toolbar()->add_checkboxes_standard();

  setup_statusbar(
    {{"PaneFileType", 50},
     {"PaneInfo", 100},
     {"PaneLexer", 60},
     {"PaneMacro", 50}});
}

frame::~frame()
{
  delete m_process;
}

wex::del::listview*
frame::activate(wex::data::listview::type_t type, const wex::lexer* lexer)
{
  for (size_t i = 0; i < m_notebook->GetPageCount(); i++)
  {
    auto* vw = (wex::del::listview*)m_notebook->GetPage(i);

    if (vw->data().type() == type)
    {
      return vw;
    }
  }

  return nullptr;
}

bool frame::allow_close(wxWindowID id, wxWindow* page)
{
  return page == file_history_list() || page == get_listview() ?
           // prevent possible crash, if set_recent_file tries
           // to add listitem to deleted history list.
           false :
           wex::del::frame::allow_close(id, page);
}

void frame::bind_all()
{
  // The on_command keeps statistics.
  Bind(wxEVT_MENU, &frame::on_command, this, wxID_COPY);
  Bind(wxEVT_MENU, &frame::on_command, this, wxID_CUT);
  Bind(wxEVT_MENU, &frame::on_command, this, wxID_EXECUTE);
  Bind(wxEVT_MENU, &frame::on_command, this, wxID_JUMP_TO);
  Bind(wxEVT_MENU, &frame::on_command, this, wxID_PASTE);
  Bind(wxEVT_MENU, &frame::on_command, this, wxID_OPEN, wxID_SAVEAS);
  Bind(wxEVT_MENU, &frame::on_command, this, wxID_UNDO, wxID_REDO);
  Bind(wxEVT_MENU, &frame::on_command, this, ID_STC_SPLIT);

  wex::bind(this).command(
    {{[=, this](const wxCommandEvent& event)
      {
        m_statistics->inc(std::to_string(event.GetId()));
        wex::version_info_dialog().show();
      },
      wxID_ABOUT},
     {[=, this](const wxCommandEvent& event)
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
            wex::test_config_item().vector(0, val),
            wex::data::window()
              .title("Config Dialog Columns")
              .size(wxSize(600, 400)),
            0,
            val)
            .ShowModal();
        }
      },
      ID_DLG_CONFIG_ITEM_COL},
     {[=, this](const wxCommandEvent& event)
      {
        auto* dlg = new wex::item_dialog(
          wex::test_config_item().vector(0, 1),
          wex::data::window()
            .title("Config Dialog")
            .id(ID_DLG_CONFIG_ITEM)
            .
#ifdef __WXMSW__
          size(wxSize(500, 500)));
#else
          size(wxSize(600, 400)));
#endif
        dlg->ShowModal();
      },
      ID_DLG_CONFIG_ITEM},
     {[=, this](const wxCommandEvent& event)
      {
        wex::item_dialog(
          wex::test_config_item().vector(0, 1),
          wex::data::window()
            .button(wxCANCEL)
            .id(ID_DLG_CONFIG_ITEM_READONLY)
            .title("Config Dialog Readonly")
            .size(wxSize(600, 400)),
          0,
          4)
          .ShowModal();
      },
      ID_DLG_CONFIG_ITEM_READONLY},
     {[=, this](const wxCommandEvent& event)
      {
        wex::item_dialog(
          wex::test_item().vector(),
          wex::data::window().id(ID_DLG_ITEM))
          .ShowModal();
      },
      ID_DLG_ITEM},
     {[=, this](const wxCommandEvent& event)
      {
        m_listview->config_dialog();
      },
      ID_DLG_LISTVIEW},
     {[=, this](const wxCommandEvent& event)
      {
        wex::stc::config_dialog(
          wex::data::window().id(wxID_PREFERENCES).button(wxAPPLY | wxCANCEL));
      },
      wxID_PREFERENCES},
     {[=, this](const wxCommandEvent& event)
      {
        std::stringstream text;
        for (auto i = 0; i < 100; i++)
        {
          text << "Hello from line: " << i << "\n";
        }
        wex::stc_entry_dialog(
          text.str(),
          "Greetings from " + std::string(wxTheApp->GetAppDisplayName()),
          wex::data::window().title("Hello world"))
          .ShowModal();
      },
      ID_DLG_STC_ENTRY},

     {[=, this](const wxCommandEvent& event)
      {
        const std::string project_wildcard{
          _("Project Files") + " (*.prj)|*.prj"};

        wxFileDialog dlg(
          this,
          _("Select Projects"),
          (!get_project_history()[0].empty() ?
             get_project_history()[0].parent_path() :
             wex::config::dir().string()),
          wxEmptyString,
          project_wildcard,
// osx asserts on GetPath with wxFD_MULTIPLE flag,
// and the to_vector_path does not do anything
#ifdef __WXOSX__
          wxFD_OPEN);
#else
          wxFD_OPEN | wxFD_MULTIPLE);
#endif
        if (dlg.ShowModal() == wxID_CANCEL)
          return;
        const std::vector<wex::path> v(
#ifdef __WXOSX__
          {wex::path(dlg.GetPath().ToStdString())});
#else
          wex::to_vector_path(dlg).get());
#endif
        wex::open_files(
          this,
          v,
          wex::data::stc().flags(
            wex::data::stc::window_t().set(wex::data::stc::WIN_IS_PROJECT)));
      },
      ID_PROJECT_OPEN},

     {[=, this](const wxCommandEvent& event)
      {
        wex::vcs().config_dialog();
      },
      ID_DLG_VCS},
     {[=, this](const wxCommandEvent& event)
      {
        m_shell->prompt(
          "\nHello '" + event.GetString().ToStdString() + "' from the shell");
      },
      wex::ID_SHELL_COMMAND},
     {[=, this](const wxCommandEvent& event)
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

     {[=, this](const wxCommandEvent& event)
      {
        if (m_notebook->set_selection("Statistics") == nullptr)
        {
          m_notebook->add_page(wex::data::notebook()
                                 .page(m_statistics->show())
                                 .caption("Statistics")
                                 .key("Statistics")
                                 .select());
        }
      },
      ID_STATISTICS_SHOW},
     {[=, this](const wxCommandEvent& event)
      {
        if (const auto value = wxGetNumberFromUser(
              "Input:",
              wxEmptyString,
              "STC Open Flag",
              static_cast<unsigned long>(m_app->data().flags().to_ulong()),
              0,
              0xFFFF);
            value != -1)
        {
          m_app->data().flags(value);
        }
      },
      ID_STC_FLAGS},
     {[=, this](const wxCommandEvent& event)
      {
        m_process->async_system(wex::process_data());
      },
      wxID_EXECUTE}});
}

wex::listview* frame::get_listview()
{
  return (wex::listview*)m_notebook->GetPage(m_notebook->GetSelection());
}

wex::stc* frame::get_stc()
{
  return m_stc;
}

void frame::on_command(const wxCommandEvent& event)
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
      const auto start = std::chrono::system_clock::now();

      if (event.GetString().empty())
      {
        if (wex::file_dialog dlg(&m_stc->get_file());
            dlg.show_modal_if_changed(true) != wxID_CANCEL)
        {
          open_file_same_page(wex::path(dlg.GetPath().ToStdString()));
        }
      }
      else
      {
        open_file_same_page(wex::path(event.GetString()));
      }

      const auto milli = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now() - start);
      wex::log::status("Open")
        << milli.count() << "milliseconds" << m_stc->GetTextLength() << "bytes";
    }
    break;

    case wxID_SAVE:
      m_stc->get_file().file_save();
      set_recent_file(m_stc->path());

      if (m_stc->path().data() == wex::lexers::get()->path().data())
      {
        wex::lexers::get()->load_document();
        wex::log::trace("file contains")
          << wex::lexers::get()->get_lexers().size() << "lexers";
        // As the lexer might have changed, update status bar field as well.
        update_statusbar(m_stc, "PaneLexer");
      }
      break;

    case wxID_SAVEAS:
      m_stc->get_file().file_save(wex::path(event.GetString()));
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
          wex::data::stc(m_app->data())
            .window(wex::data::window().parent(m_notebook)));
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
    if (event.GetId() != wxID_CANCEL && m_stc != nullptr)
    {
      m_stc->config_get();
    }
  }
  else if (dialogid > ID_SAMPLE_LOWEST && dialogid < ID_SAMPLE_HIGHEST)
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
  if (data.flags().test(wex::data::stc::WIN_IS_PROJECT))
  {
    m_project->file_load(file);
    m_notebook->set_selection("wex::project");

    return nullptr;
  }
  else if (m_stc != nullptr)
  {
    m_stc->get_lexer().clear();
    open_file_same_page(file);
    m_notebook->set_selection("wex::stc");

    return m_stc;
  }
  else
  {
    return nullptr;
  }
}

void frame::open_file_same_page(const wex::path& p)
{
  m_stc->open(p, wex::data::stc().recent(false).flags(m_app->data().flags()));
  m_stc->get_lexer().set(wex::path_lexer(p).lexer().display_lexer(), true);
  m_stc->properties_message();
}

void frame::update()
{
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

  wex::path project;

  if (
    m_app->data().flags().test(wex::data::stc::WIN_IS_PROJECT) &&
    !m_app->get_files().empty())
  {
    project = m_app->get_files().front();
    m_app->get_files().erase(m_app->get_files().begin());
  }

  m_project = new wex::del::file(
    project,
    wex::data::listview().window(wex::data::window().parent(m_notebook)));

  m_stc = new wex::stc(
    !m_app->get_files().empty() ? m_app->get_files().front() :
                                  wex::path("no name"),
    wex::data::stc(m_app->data())
      .window(wex::data::window().parent(m_notebook)));

  m_notebook->add_page(
    wex::data::notebook().page(m_stc).key("wex::stc").select());

  m_notebook->add_page(
    wex::data::notebook().page(m_project).key("wex::project"));

  m_notebook->add_page(
    wex::data::notebook().page(m_statistics).key("wex::statistics"));

  m_notebook->add_page(
    wex::data::notebook().page(m_listview).key("wex::listview"));
  m_notebook->add_page(wex::data::notebook().page(m_grid).key("wex::grid"));
  m_notebook->add_page(wex::data::notebook().page(m_shell).key("wex::shell"));

  const wex::lexer lexer(wex::lexers::get()->find("cpp"));

  for (int i = wex::data::listview::FOLDER; i <= wex::data::listview::FILE; i++)
  {
    auto* vw = new wex::del::listview(
      wex::data::listview().type((wex::data::listview::type_t)i).lexer(&lexer));

    m_notebook->add_page(
      wex::data::notebook().page(vw).key(vw->data().type_description()));
  }

  bind_all();
}
