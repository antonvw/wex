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

/// Adds managed pane.
/// Returns name of pane.
const std::string AddPane(wxExManagedFrame* frame, wxWindow* pane);

/// Returns test dir.
const std::string GetTestDir();

/// Returns test file.
const wxExPath GetTestFile();
  
/// Sets environment. 
void SetEnvironment(const std::string& dir);

/// Invoke UI action on window, 
/// Returns false if an error occurred.
bool wxExUIAction(
  /// window to operate on
  wxWindow* win, 
  /// action, if starts with button is a button action,
  /// if starts with dialog is a dialog action
  const std::string& action = "button",
  /// e.g. the dialog to operate on
  const std::string& par = "right");
  
/// Derive your application from wxExApp.
class wxExTestApp: public wxExApp
{
public:
  /// Constructor.
  wxExTestApp() {}
  
  static auto GetTestPath() {return m_TestPath;};

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

int wxExTestMain(int argc, char* argv[], wxExTestApp* app, bool use_eventloop);
