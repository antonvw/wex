////////////////////////////////////////////////////////////////////////////////
// Name:      test.h
// Purpose:   Declaration of classes for unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <memory>
#include <string>
#include <wex/app.h>
#include <wex/path.h>
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
    static path get_testpath(const std::string& file = std::string());

    /// Prepare environment.
    virtual bool OnInit() override;

    /// Start event loop and start testing.
    virtual int OnRun() override;

    /// Sets context.
    void set_context(doctest::Context* context);
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
const std::string add_pane(wex::managed_frame* frame, wxWindow* pane);

/// Returns test path or file in dir if specified.
const wex::path get_testpath(const std::string& file = std::string());
