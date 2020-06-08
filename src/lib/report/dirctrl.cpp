////////////////////////////////////////////////////////////////////////////////
// Name:      dirctrl.cpp
// Purpose:   Implementation of class wex::report::dirctrl
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/bind.h>
#include <wex/core.h>
#include <wex/lexers.h>
#include <wex/path.h>
#include <wex/report/defs.h>
#include <wex/report/dirctrl.h>
#include <wex/report/frame.h>
#include <wex/tostring.h>
#include <wex/util.h>
#include <wex/vcs.h>
#include <wx/stockitem.h> // for wxGetStockLabel

#define GET_VECTOR_FILES                              \
  const auto files(wex::to_vector_path(*this).get()); \
  if (files.empty())                                  \
  {                                                   \
    event.Skip();                                     \
    return;                                           \
  }

const int idShowHidden = wxWindow::NewControlId();

wex::report::dirctrl::dirctrl(frame* frame, const data::window& data)
  : wxGenericDirCtrl(
      data.parent(),
      data.id(),
      wxDirDialogDefaultFolderStr,
      data.pos(),
      data.size(),
      data.style(),
      "*", // filter
      0,   // filter index
      data.name())
{
  if (config(_("Show hidden")).get(false))
  {
    ShowHidden(true);
  }

  lexers::get()->apply_default_style(
    [=](const std::string& back) {
      GetTreeCtrl()->SetBackgroundColour(wxColour(back));
    },
    [=](const std::string& fore) {
      GetTreeCtrl()->SetForegroundColour(wxColour(fore));
    });

  bind(this).command(
    {{[=](wxCommandEvent& event) {
        vcs_execute(
          frame,
          event.GetId() - ID_EDIT_VCS_LOWEST - 1,
          to_vector_path(*this).get());
      },
      ID_EDIT_VCS_LOWEST + 1},
     {[=](wxCommandEvent& event) {
        std::string clipboard;
        const auto  v(to_vector_string(*this).get());
        for (const auto& it : v)
        {
          clipboard += it + "\n";
        }
        clipboard_add(clipboard);
      },
      ID_TREE_COPY},
     {[=](wxCommandEvent& event) {
        open_files(
          frame,
          to_vector_path(*this).get(),
          data::stc(),
          data::dir::type_t().set(data::dir::FILES));
      },
      ID_EDIT_OPEN},
     {[=](wxCommandEvent& event) {
        GET_VECTOR_FILES
        make(files[0]);
      },
      ID_TREE_RUN_MAKE},
     {[=](wxCommandEvent& event) {
        frame->find_in_files(to_vector_path(*this).get(), event.GetId());
      },
      ID_TOOL_REPORT_FIND},
     {[=](wxCommandEvent& event) {
        frame->find_in_files(to_vector_path(*this).get(), event.GetId());
      },
      ID_TOOL_REPLACE},
     {[=](wxCommandEvent& event) {
        ShowHidden(!config(_("Show hidden")).get(false));
        config(_("Show hidden")).set(!config(_("Show hidden")).get(false));
      },
      idShowHidden}});

  Bind(wxEVT_TREE_ITEM_ACTIVATED, [=](wxTreeEvent& event) {
    GET_VECTOR_FILES

    if (const wex::path fn(files[0]); !fn.file_exists() && fn.dir_exists())
    {
      if (!GetTreeCtrl()->IsExpanded(event.GetItem()))
      {
        expand_and_select_path(files[0]);
      }
      else
      {
        CollapsePath(files[0].string());
      }
    }
    else
    {
      open_files(
        frame,
        files,
        data::stc(),
        data::dir::type_t().set(data::dir::FILES));
    }
  });

  Bind(wxEVT_TREE_ITEM_MENU, [=](wxTreeEvent& event) {
    GET_VECTOR_FILES
    const wex::path filename(files[0]);

    wex::menu menu(menu::menu_t().set(menu::IS_POPUP));

    if (filename.file_exists())
    {
      menu.append({{ID_EDIT_OPEN, _("&Open")}, {}});
    }

    menu.append({{ID_TREE_COPY,
                  wxGetStockLabel(wxID_COPY),
                  data::menu().art(wxART_COPY)}});

    if (vcs::dir_exists(filename))
    {
      menu.append({{}, {filename}});
    }

    if (filename.lexer().scintilla_lexer() == "makefile")
    {
      menu.append({{}, {ID_TREE_RUN_MAKE, "&Make"}});
    }

    menu.append(
      {{},
       {ID_TOOL_REPORT_FIND,
        ellipsed(frame->find_in_files_title(ID_TOOL_REPORT_FIND))},
       {ID_TOOL_REPLACE, ellipsed(frame->find_in_files_title(ID_TOOL_REPLACE))},
       {}});

    auto* item = menu.AppendCheckItem(idShowHidden, _("Show hidden"));
    if (config(_("Show hidden")).get(false))
      item->Check();

    PopupMenu(&menu);
  });

  Bind(wxEVT_TREE_SEL_CHANGED, [=](wxTreeEvent& event) {
    GET_VECTOR_FILES
    log::status() << files[0];
  });
}

void wex::report::dirctrl::expand_and_select_path(const wex::path& path)
{
  ExpandPath(path.string());
  SelectPath(path.string());
}
