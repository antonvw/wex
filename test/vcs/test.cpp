////////////////////////////////////////////////////////////////////////////////
// Name:      test.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "test.h"

wex::vcs_entry* load_git_entry()
{
  pugi::xml_document doc;

  REQUIRE(doc.load_string("<vcs name=\"git\" admin-dir=\"./\" log-flags=\"-n "
                          "1\" blame-format=\"(^[a-zA-Z0-9^]+) "
                          "(.*?)\\((.+)\\s+([0-9]{2,4}.[0-9]{2}.[0-9]{2}.[0-"
                          "9:]{8}) .[0-9]+\\s+([0-9]+)\\) (.*)\">"
                          "</vcs>"));

  auto* entry = new wex::vcs_entry(doc.document_element());

  REQUIRE(entry->name() == "git");

  return entry;
}
