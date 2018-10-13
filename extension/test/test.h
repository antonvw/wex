////////////////////////////////////////////////////////////////////////////////
// Name:      test.h
// Purpose:   Declaration of classes for unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <memory>
#include <string>
#include <wx/extension/app.h>
#include <wx/extension/path.h>
#include "doctest.h"

namespace doctest
{
  class Context;
}

namespace wex
{
  class managed_frame;

  /// Derive your application from app.
  class test_app: public app
  {
  public:
    /// Constructor.
    test_app() {};

    /// Returns test path.  
    static path GetTestPath(const std::string& file = std::string());

    /// Prepare environment.
    virtual bool OnInit() override;

    /// Start event loop and start testing.
    virtual int OnRun() override;

    /// Sets context.
    void SetContext(doctest::Context* context);
  private:
    void SetTestPath();

    doctest::Context* m_Context;
    
    static inline path m_TestPath;
  };
  
  /// Invoke UI action on window, 
  int testmain(int argc, char* argv[], test_app* app);
};

/// Adds managed pane.
/// Returns name of pane.
const std::string AddPane(wex::managed_frame* frame, wxWindow* pane);

/// Returns test path or file in dir if specified.
const wex::path GetTestPath(const std::string& file = std::string());
