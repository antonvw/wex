////////////////////////////////////////////////////////////////////////////////
// Name:      app.h
// Purpose:   Include file for wex::del::app class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/core/app.h>

namespace wex::del
{
/// Offers the application, with lib specific init and exit,
/// Your application should be derived from this class.
class app : public wex::app
{
public:
  // Virtual interface

  /// Initializes vcs and loads the VCS file.
  /// In your own OnInit first set the app name,
  /// and then call this base class method.
  bool OnInit() override;

  /// Deletes all global objects and cleans up things if necessary.
  int OnExit() override;
};
}; // namespace wex::del
