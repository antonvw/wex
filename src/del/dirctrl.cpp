////////////////////////////////////////////////////////////////////////////////
// Name:      dirctrl.cpp
// Purpose:   Implementation of class wex::del::dirctrl
// Author:    Anton van Wezenbeek
// Copyright: (c) 2010-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/wex.h>
#include <wx/stockitem.h> // for wxGetStockLabel

#define GET_VECTOR_FILES                                                       \
  const auto files(wex::to_vector_path(*this).get());                          \
  /* NOLINTNEXTLINE */                                                         \
  if (files.empty())                                                           \
  {                                                                            \
    event.Skip();                                                              \
    return;                                                                    \
  }

#include <numeric>

const int idShowHidden = wxWindow::NewControlId();

wex::del::dirctrl::dirctrl(frame* frame, const data::window& data)
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
    [=, this](const std::string& back)
    {
      GetTreeCtrl()->SetBackgroundColour(wxColour(back));
    },
    [=, this](const std::string& fore)
    {
      GetTreeCtrl()->SetForegroundColour(wxColour(fore));
    });

  bind(this).command(
    {{[=, this](const wxCommandEvent& event)
      {
        vcs_execute(
          frame,
          event.GetId() - ID_EDIT_VCS_LOWEST - 1,
          to_vector_path(*this).get());
      },
      ID_EDIT_VCS_LOWEST},
     {[=, this](const wxCommandEvent& event)
      {
        const auto v(to_vector_string(*this).get());
        clipboard_add(std::accumulate(
          v.begin(),
          v.end(),
          std::string(),
          [](const std::string& a, const std::string& b)
          {
            return a + b + "\n";
          }));
      },
      ID_TREE_COPY},
     {[=, this](const wxCommandEvent& event)
      {
        open_files(
          frame,
          to_vector_path(*this).get(),
          data::stc(),
          data::dir::type_t().set(data::dir::FILES));
      },
      ID_EDIT_OPEN},
     {[=, this](wxCommandEvent& event)
      {
        GET_VECTOR_FILES

        build(path_lexer(files[0]));
      },
      ID_TREE_RUN_BUILD},
     {[=, this](const wxCommandEvent& event)
      {
        frame->find_in_files(
          to_vector_path(*this).get(),
          tool((wex::window_id)event.GetId()));
      },
      ID_TOOL_REPORT_FIND},
     {[=, this](const wxCommandEvent& event)
      {
        frame->find_in_files(
          to_vector_path(*this).get(),
          tool((wex::window_id)event.GetId()));
      },
      ID_TOOL_REPLACE},
     {[=, this](const wxCommandEvent& event)
      {
        ShowHidden(!config(_("Show hidden")).get(false));
        config(_("Show hidden")).set(!config(_("Show hidden")).get(false));
      },
      idShowHidden}});

  Bind(
    wxEVT_TREE_ITEM_ACTIVATED,
    [=, this](wxTreeEvent& event)
    {
      GET_VECTOR_FILES

      if (const auto& fn(files[0]); !fn.file_exists() && fn.dir_exists())
      {
        if (!GetTreeCtrl()->IsExpanded(event.GetItem()))
        {
          expand_and_select_path(fn);
        }
        else
        {
          CollapsePath(fn.string());
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

  Bind(
    wxEVT_TREE_ITEM_MENU,
    [=, this](wxTreeEvent& event)
    {
      GET_VECTOR_FILES

      const wex::path_lexer filename(files[0]);

      wex::menu menu(
        menu::menu_t_def().set(menu::IS_POPUP).set(menu::IS_VISUAL));

      if (filename.file_exists())
      {
        menu.append({{ID_EDIT_OPEN, _("&Open")}, {}});
      }

      menu.append(
        {{ID_TREE_COPY,
          wxGetStockLabel(wxID_COPY),
          data::menu().art(wxART_COPY)}});

      // If the filename is under vcs append vcs submenu.
      if (vcs::dir_exists(filename))
      {
        menu.append({{}, {filename, frame}});
      }

      if (filename.is_build())
      {
        menu.append({{}, {ID_TREE_RUN_BUILD, "&Build"}});
      }

      menu.append(
        {{},
         {ID_TOOL_REPORT_FIND,
          ellipsed(frame->find_in_files_title(ID_TOOL_REPORT_FIND))},
         {ID_TOOL_REPLACE,
          ellipsed(frame->find_in_files_title(ID_TOOL_REPLACE))},
         {}});

      if (auto* item = menu.AppendCheckItem(idShowHidden, _("Show hidden"));
          config(_("Show hidden")).get(false))
      {
        item->Check();
      }

      PopupMenu(&menu);
    });

  Bind(
    wxEVT_TREE_SEL_CHANGED,
    [=, this](wxTreeEvent& event)
    {
      GET_VECTOR_FILES
      log::status() << files[0];
    });
}

void wex::del::dirctrl::expand_and_select_path(const wex::path& path)
{
  ExpandPath(path.string());
  SelectPath(path.string());
}
