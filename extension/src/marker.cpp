////////////////////////////////////////////////////////////////////////////////
// Name:      marker.cpp
// Purpose:   Implementation of class wex::marker
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/marker.h>

wex::marker::marker(const pugi::xml_node& node)
  : presentation(MARKER, node)
{
}

wex::marker::marker(int no, int symbol)
  : presentation(MARKER, no, symbol)
{
}
