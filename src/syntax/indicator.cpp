////////////////////////////////////////////////////////////////////////////////
// Name:      indicator.cpp
// Purpose:   Implementation of class wex::indicator
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019-2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/syntax/indicator.h>

wex::indicator::indicator(const pugi::xml_node& node)
  : presentation(INDICATOR, node)
{
}

wex::indicator::indicator(int no, int style)
  : presentation(INDICATOR, no, style)
{
}
