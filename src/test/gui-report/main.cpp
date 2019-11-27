////////////////////////////////////////////////////////////////////////////////
// Name:      main.cpp
// Purpose:   main for wex report unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
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
        : report::frame() {
          lexer lexer("cpp");
          m_report = new listview(listview_data().
            type(listview_data::KEYWORD).lexer(&lexer));
          wex::test::add_pane(this, m_report);};

      listview* activate(
        listview_data::type_t listview_type, const lexer* lexer) override {
        // only for coverage
        report::frame::activate(listview_type, lexer);
        return m_report;};

      void more_coverage() {
        file_history_list();};
    private:
      listview* m_report;
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
      
        m_frame = new test::frame();
        m_frame->more_coverage();
        m_frame->Show();
        
        return true;
      }
      
      static auto* frame() {return m_frame;};
    private:
      inline static test::frame* m_frame = nullptr;
    }; 
  };
};

IMPLEMENT_APP_NO_MAIN(wex::test::gui_app);

wex::report::frame* frame()
{
  return wex::test::gui_app::frame();
}
    
const std::string get_project()
{
  return "test-rep.prj";
}
  
int main(int argc, char* argv[])
{
  return wex::test::main(argc, argv, new wex::test::gui_app());
}
