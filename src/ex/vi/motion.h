////////////////////////////////////////////////////////////////////////////////
// Name:      motion.h
// Purpose:   Declaration of enum class wex::vi::motion_t
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2025 Anton van Wezenbeek
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

  // all vim g commands
  G,

  // vim g other than motion
  G_8,
  G_a,
  G_d,
  G_f,
  G_hash,
  G_m,
  G_star,
  G_t,
  G_T,

  // vim g motion
  G_motion_start,
  G_tilde,
  G_u,
  G_U,
  G_motion_end,

  // all vim z commands
  Z,
};
};
