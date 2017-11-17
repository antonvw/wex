////////////////////////////////////////////////////////////////////////////////
// Name:      test.h
// Purpose:   Declaration of classes for unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <memory>
#include <string>
#include <wx/extension/app.h>
#include <wx/extension/path.h>
#include "doctest.h"

class wxExManagedFrame;

namespace doctest
{
  class Context;
}

/// Derive your application from wxExApp.
class wxExTestApp: public wxExApp
{
public:
  /// Constructor.
  wxExTestApp() {};

  /// Returns test path.  
  static wxExPath GetTestPath(const std::string& file = std::string());

  /// Prepare environment.
  virtual bool OnInit() override;

  /// Start event loop and start testing.
  virtual int OnRun() override;

  /// Sets context.
  void SetContext(doctest::Context* context);
private:
  void SetTestPath();

  doctest::Context* m_Context;
  
  static wxExPath m_TestPath;
};

/// Adds managed pane.
/// Returns name of pane.
const std::string AddPane(wxExManagedFrame* frame, wxWindow* pane);

/// Returns test path or file in dir if specified.
const wxExPath GetTestPath(const std::string& file = std::string());
  
/// Invoke UI action on window, 
int wxExTestMain(int argc, char* argv[], wxExTestApp* app);

/// Returns false if an error occurred.
bool wxExUIAction(
  /// window to operate on
  wxWindow* win, 
  /// action, if starts with button is a button action,
  /// if starts with dialog is a dialog action
  const std::string& action = "button",
  /// e.g. the dialog to operate on
  const std::string& par = "right");
