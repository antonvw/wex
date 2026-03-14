////////////////////////////////////////////////////////////////////////////////
// Name:      util.h
// Purpose:   Declaration of factory util functions
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <boost/regex.hpp>

class wxWindow;

namespace wex
{
namespace factory
{
class find_replace_data;
};

/// Binds to focus.
void bind_set_focus(wxWindow* window);

/// Gets regex flags according to find replace data.
boost::regex::flag_type get_regex_flags(const factory::find_replace_data& data);
}; // namespace wex
