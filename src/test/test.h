////////////////////////////////////////////////////////////////////////////////
// Name:      test.h
// Purpose:   Declaration of classes for unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <doctest.h>
#include <wex/app.h>
#include <wex/path.h>

namespace doctest
{
  class Context;
}

namespace wex
{
  class managed_frame;
  
  namespace test
  {
    /// Offers the doctest based test application, 
    /// with lib specific init and exit.
    /// Your test application should be derived from this class.
    class app: public wex::app
    {
    public:
      /// Virtual interface

      /// Prepare environment.
      bool OnInit() override;

      /// Start event loop and start testing.
      int OnRun() override;

      /// Other methods
    
      /// Returns the test path.  
      static path get_path(const std::string& file = std::string());

      /// Sets context.
      void set_context(doctest::Context* context);
    private:
      doctest::Context* m_context;
      static inline path m_path;
    };
    
    /// Connects main proc and test app. All doctests will start.
    /// 
    /// E.g.:
    /// int main (int argc, char* argv[])
    /// {
    ///   return wex::test::main(argc, argv, new wex::test::app());
    /// }  
    int main(int argc, char* argv[], app* app);

    /// Adds managed pane.
    /// Returns name of pane.
    const std::string add_pane(wex::managed_frame* frame, wxWindow* pane);

    /// Returns the test path or file in the dir if specified.
    const wex::path get_path(const std::string& file = std::string());
  };
};
