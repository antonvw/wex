////////////////////////////////////////////////////////////////////////////////
// Name:      app.cpp
// Purpose:   Implementation of wex::del::app class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/path.h>
#include <wex/ctags/ctags.h>
#include <wex/del/app.h>
#include <wex/stc/stc.h>
#include <wex/syntax/lexers.h>
#include <wex/syntax/printing.h>
#include <wex/vcs/vcs.h>

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

  return true;
}
