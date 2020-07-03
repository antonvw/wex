////////////////////////////////////////////////////////////////////////////////
// Name:      main.cpp
// Purpose:   main for wex report unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "test.h"

namespace wex
{
  namespace test
  {
    class frame : public report::frame
    {
    public:
      frame()
        : report::frame()
      {
        lexer lexer("cpp");
        m_report = new listview(
          data::listview().type(data::listview::KEYWORD).lexer(&lexer));
        wex::test::add_pane(this, m_report);
      };

      listview*
      activate(data::listview::type_t listview_type, const lexer* lexer) override
      {
        // only for coverage
        report::frame::activate(listview_type, lexer);
        return m_report;
      };

      void more_coverage() { file_history_list(); };

    private:
      listview* m_report;
    };

    class report_app : public app
    {
    public:
      bool OnInit() override
      {
        if (!test::app::OnInit())
        {
          return false;
        }

        m_frame = new test::frame();
        m_frame->more_coverage();
        m_frame->Show();

        return true;
      }

      static auto* frame() { return m_frame; };

    private:
      inline static test::frame* m_frame = nullptr;
    };
  }; // namespace test
};   // namespace wex

IMPLEMENT_APP_NO_MAIN(wex::test::report_app);

wex::report::frame* report_frame()
{
  return wex::test::report_app::frame();
}

const std::string get_project()
{
  return "test-rep.prj";
}

int main(int argc, char* argv[])
{
  return wex::test::main(argc, argv, new wex::test::report_app());
}
