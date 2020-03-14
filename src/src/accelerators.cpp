////////////////////////////////////////////////////////////////////////////////
// Name:      accelerators.cpp
// Purpose:   Implementation of class wex::accelerators
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/app.h>
#include <wx/window.h>
#include <wex/accelerators.h>
#include <wex/debug.h>
#include <wex/managedframe.h>

wex::accelerators::accelerators(
  const std::vector < wxAcceleratorEntry > & v, bool debug) 
{
  auto* frame(dynamic_cast<managed_frame*>(wxTheApp->GetTopWindow()));
  m_size = v.size() + (debug ? 
    frame->get_debug()->debug_entry().get_commands().size(): 0);
  m_entries = new wxAcceleratorEntry[m_size];

  int i = 0;

  for (const auto& it : v)
  {
    m_entries[i++] = it;
  }

  if (debug)
  {
    int j = ID_EDIT_DEBUG_FIRST;

    for (const auto& e : frame->get_debug()->debug_entry().get_commands())
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
