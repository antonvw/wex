////////////////////////////////////////////////////////////////////////////////
// Name:      app.cpp
// Purpose:   Implementation of wex report sample classes
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "app.h"
#include <wex/wex.h>
#include <wx/aboutdlg.h>
#ifndef __WXMSW__
#include "app.xpm"
#endif

wxIMPLEMENT_APP(app);

bool app::OnInit()
{
  SetAppName("wex-sample-rep");

  if (!wex::app::OnInit())
  {
    return false;
  }

  auto* f = new frame();
  f->Show(true);

  return true;
}

frame::frame()
  : wex::report::frame()
  , m_notebook(new wex::notebook(wex::data::window().style(
      wxAUI_NB_DEFAULT_STYLE | wxAUI_NB_WINDOWLIST_BUTTON)))
  , m_stc(new wex::stc())
{
  SetIcon(wxICON(app));

  SetMenuBar(new wex::menubar(
    {{new wex::menu(
        {{wxID_OPEN}, {}, {wex::menu_item::PRINT}, {}, {wex::menu_item::EXIT}}),
      "&File"},
     {new wex::menu(
        {{}, // fix  compile error
         {this},
         {}}),
      "&View"},
     {new wex::menu(
        {{wxID_ABOUT, "", wex::data::menu().action([=, this](wxCommandEvent& event) {
            wxAboutDialogInfo info;
            info.SetIcon(GetIcon());
            info.SetVersion(wex::get_version_info().get());
            info.SetCopyright(wex::get_version_info().copyright());
            wxAboutBox(info);
          })}}),
      "&Help"}}));

  get_toolbar()->add_standard();

  setup_statusbar({{"PaneFileType", 50}, {"PaneInfo", 100}, {"PaneLexer", 60}});

  const wex::lexer lexer(wex::lexers::get()->find("cpp"));

  for (int i = wex::data::listview::FOLDER; i <= wex::data::listview::FILE; i++)
  {
    auto* vw = new wex::report::listview(
      wex::data::listview().type((wex::data::listview::type_t)i).lexer(&lexer));

    m_notebook->add_page(wex::data::notebook()
                           .page(vw)
                           .key(vw->data().type_description())
                           .select());
  }

  pane_add(
    {{m_stc,
      wxAuiPaneInfo().CenterPane().CloseButton(false).MaximizeButton(true)},
     {m_notebook,
      wxAuiPaneInfo().CloseButton(false).Bottom().MinSize(wxSize(250, 250))},
     {new wex::report::dirctrl(this),
      wxAuiPaneInfo().Caption("DirCtrl").Left().MinSize(wxSize(250, 250))}});

  wex::report::dir dir(
    (wex::listview*)m_notebook->page_by_key(
      wex::data::listview().type(wex::data::listview::FILE).type_description()),
    wex::path::current(),
    wex::data::dir().file_spec("*.cpp;*.h"));

  dir.find_files();

  wex::listitem item(
    (wex::listview*)m_notebook->page_by_key(
      wex::data::listview().type(wex::data::listview::FILE).type_description()),
    wex::path("NOT EXISTING ITEM"));

  item.insert();

  Bind(
    wxEVT_MENU,
    [=, this](wxCommandEvent& event) {
      m_stc->get_file().file_new(wex::path());
    },
    wxID_NEW);
}

wex::report::listview*
frame::activate(wex::data::listview::type_t type, const wex::lexer* lexer)
{
  for (size_t i = 0; i < m_notebook->GetPageCount(); i++)
  {
    wex::report::listview* vw = (wex::report::listview*)m_notebook->GetPage(i);

    if (vw->data().type() == type)
    {
      if (type == wex::data::listview::KEYWORD)
      {
        if (lexer != nullptr)
        {
          if (lexer->scintilla_lexer() != "cpp")
          {
            if (!lexer->display_lexer().empty())
            {
              wex::log::trace(lexer->display_lexer())
                << ", only cpp for the sample";
            }

            return nullptr;
          }
        }
      }

      return vw;
    }
  }

  return nullptr;
}

bool frame::allow_close(wxWindowID id, wxWindow* page)
{
  if (page == file_history_list())
  {
    // prevent possible crash, if set_recent_file tries
    // to add listitem to deleted history list.
    return false;
  }
  else
  {
    return wex::report::frame::allow_close(id, page);
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

wex::stc* frame::open_file(const wex::path& file, const wex::data::stc& data)
{
  m_stc->get_lexer().clear();
  m_stc->open(file, wex::data::stc(data).flags(0));

  return m_stc;
}
