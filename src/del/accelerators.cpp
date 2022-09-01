////////////////////////////////////////////////////////////////////////////////
// Name:      accelerators.cpp
// Purpose:   Implementation of class wex::accelerators
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/del/accelerators.h>
#include <wex/del/frame.h>
#include <wex/factory/defs.h>
#include <wex/vcs/debug.h>
#include <wx/app.h>
#include <wx/window.h>

wex::accelerators::accelerators(
  const std::vector<wxAcceleratorEntry>& v,
  bool                                   debug)
{
  auto* frame(dynamic_cast<del::frame*>(wxTheApp->GetTopWindow()));

  m_size =
    v.size() +
    (debug ? frame->get_debug()->debug_entry().get_commands().size() : 0);

  m_entries = new wxAcceleratorEntry[m_size];
  int i     = 0;

  for (const auto& it : v)
  {
    m_entries[i++] = it;
  }

  if (debug)
  {
    for (int         j = ID_EDIT_DEBUG_FIRST;
         const auto& e : frame->get_debug()->debug_entry().get_commands())
    {
      if (!e.control().empty())
      {
        m_entries[i++].Set(wxACCEL_CTRL, e.control().at(0), j);
      }

      j++;
    }
  }
}

wex::accelerators::~accelerators()
{
  delete[] m_entries;
}

void wex::accelerators::set(wxWindow* parent)
{
  parent->SetAcceleratorTable(wxAcceleratorTable(m_size, m_entries));
}
