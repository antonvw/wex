////////////////////////////////////////////////////////////////////////////////
// Name:      motion.h
// Purpose:   Declaration of enum class wex::vi::motion_t
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2022 Anton van Wezenbeek
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

  // motions always allowed
  NAVIGATE,
  YANK,

  // all vim commands
  G,

  // vim special
  G_a,
  G_d,
  G_f,
  G_hash,
  G_star,

  // vim motion
  G_tilde,
  G_u,
  G_U,
};
};
