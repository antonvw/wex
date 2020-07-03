////////////////////////////////////////////////////////////////////////////////
// Name:      test.h
// Purpose:   Declaration of classes for unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <doctest.h>
#include <string>
#include <wex/app.h>
#include <wex/managedframe.h>
#include <wex/path.h>
#include <wex/process.h>
#include <wex/shell.h>
#include <wex/stc.h>

namespace doctest
{
  class Context;
}

namespace wex
{
  class managed_frame;

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

    class managed_frame : public wex::managed_frame
    {
    public:
      managed_frame()
        : wex::managed_frame()
        , m_process(new process())
      {
        ;
      };
      process* get_process(const std::string& command) override
      {
        m_process->execute(command);
        return m_process;
      };

    private:
      process* m_process;
    };

    class gui_app : public app
    {
    public:
      bool OnInit() override
      {
        if (!test::app::OnInit())
        {
          return false;
        }

        m_frame = new managed_frame();
        m_statusbar =
          m_frame->setup_statusbar({{"Pane0"}, // the order of panes is tested
                                    {"Pane1"},
                                    {"Pane2"},
                                    {"Pane3"},
                                    {"Pane4"},
                                    {"PaneInfo"},
                                    {"PaneLexer"},
                                    {"PaneMode"},
                                    {"PaneFileType"},
                                    {"LastPane"}});
        m_stc = new stc();

        m_frame->Show();

        process::prepare_output(m_frame); // before adding pane

        add_pane(m_frame, m_stc);
        add_pane(m_frame, process::get_shell());

        return true;
      }

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

/// Processes string on shell.
void process(const std::string& str, wex::shell* shell);
