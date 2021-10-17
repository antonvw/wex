////////////////////////////////////////////////////////////////////////////////
// Name:      accelerators.h
// Purpose:   Declaration of class wex::accelerators
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/accel.h>

#include <vector>

class wxWindow;

namespace wex
{
/// Offers accelerators for a window.
class accelerators
{
public:
  /// Constructor.
  accelerators(
    /// vector with accelerator entries
    const std::vector<wxAcceleratorEntry>& v,
    /// add debug menu accelerators
    bool debug = false);

  /// Destructor, deletes the entries.
  ~accelerators();

  /// Sets the accelerator entries for the specified window.
  void set(wxWindow* parent);

  /// Returns number of accelerators.
  auto size() const { return m_size; }

private:
  size_t              m_size;
  wxAcceleratorEntry* m_entries;
};
}; // namespace wex
