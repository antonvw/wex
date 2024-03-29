////////////////////////////////////////////////////////////////////////////////
// Name:      app.cpp
// Purpose:   Implementation of wex::del::app class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2023-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/wex.h>
#include <wxMaterialDesignArtProvider.hpp>

int wex::del::app::OnExit()
{
  vcs::on_exit();
  stc::on_exit();
  ctags::close();

  delete lexers::set(nullptr);
  delete printing::set(nullptr);

  return wex::app::OnExit();
}

bool wex::del::app::OnInit()
{
  if (!wex::app::OnInit())
  {
    return false;
  }

  vcs::on_init();

  wex::art::insert(
    {{id::stc::fold_all, wxART_UNFOLD_LESS_DOUBLE},
     {id::stc::open_www, wxART_OPEN_IN_BROWSER},
     {id::stc::toggle_fold, wxART_UNFOLD_LESS},
     {id::stc::unfold_all, wxART_UNFOLD_MORE_DOUBLE},
     {id::stc::zoom_in, wxART_ZOOM_IN},
     {id::stc::zoom_out, wxART_ZOOM_OUT}});

  return true;
}
