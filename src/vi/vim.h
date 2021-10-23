////////////////////////////////////////////////////////////////////////////////
// Name:      vim.h
// Purpose:   Declaration of vim special commands
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/vi/vi.h>

namespace wex
{
enum class motion_t;

/// Performs the vim g command on vi component.
bool command_g(vi* vi, vi::motion_t t, int start);

/// Returns the motion type for specified command.
vi::motion_t command_g_motion(const std::string& command);
}; // namespace wex
