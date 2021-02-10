////////////////////////////////////////////////////////////////////////////////
// Name:      test.h
// Purpose:   Declaration of classes for unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <doctest.h>
#include <string>
#include <wex/app.h>
#include <wex/managed-frame.h>
#include <wex/path.h>
#include <wex/stc.h>

namespace doctest
{
  class Context;
}

namespace wex
{
  namespace test
  {
    /// Adds managed pane.
    /// Returns name of pane.
    const std::string add_pane(wex::managed_frame* frame, wxWindow* pane);

    /// Returns the test path or file in the dir if specified.
    const wex::path get_path(const std::string& file = std::string());

    /// Offers the doctest based test application,
    /// with lib specific init and exit.
    /// Your test application should be derived from this class.
    class app : public wex::app
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
      doctest::Context*  m_context;
      static inline path m_path;
    };

    class gui_app : public app
    {
    public:
      bool OnInit() override;

      static auto* frame() { return m_frame; };
      static auto* get_statusbar() { return m_statusbar; };
      static auto* get_stc() { return m_stc; };

    private:
      inline static managed_frame* m_frame     = nullptr;
      inline static statusbar*     m_statusbar = nullptr;
      inline static stc*           m_stc       = nullptr;
    };

    /// Connects main proc and test app. All doctests will start.
    ///
    /// E.g.:
    /// int main (int argc, char* argv[])
    /// {
    ///   return wex::test::main(argc, argv, new wex::test::app());
    /// }
    int main(int argc, char* argv[], app* app);

  }; // namespace test
};   // namespace wex

/// Returns abbreviations.
std::vector<std::pair<std::string, std::string>> get_abbreviations();

/// Returns variables.
std::vector<std::string> get_builtin_variables();

/// Returns the frame.
wex::managed_frame* frame();

/// Returns the statusbar.
wex::statusbar* get_statusbar();

/// Returns an stc.
wex::stc* get_stc();
