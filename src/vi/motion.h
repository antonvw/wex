////////////////////////////////////////////////////////////////////////////////
// Name:      motion.h
// Purpose:   Declaration of enum class wex::vi::motion_t
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/vi/vi.h>

namespace wex
{
enum class vi::motion_t
{
  // motions not allowed on readonly document
  CHANGE,
  DEL,

  // special motions, not vi, but vim related
  G,
  G_aa,
  G_tilde,
  G_u,
  G_U,
  G_ZZ,

  // motions always allowed
  NAVIGATE,
  YANK,
};
};
