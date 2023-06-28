////////////////////////////////////////////////////////////////////////////////
// Name:      test.cpp
// Purpose:   Implementation of general test functions.
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/ui/frame.h>

#include "test.h"

bool wex::test::ctags::OnInit()
{
  if (!test::app::OnInit())
  {
    return false;
  }

  auto* frame = new wex::frame();
  SetTopWindow(frame);

  m_stc = new test::stc();
  frame->Show();
  frame->pane_add(m_stc);

  return true;
}

wex::syntax::stc* get_stc()
{
  return wex::test::ctags::get_stc();
}
