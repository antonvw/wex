////////////////////////////////////////////////////////////////////////////////
// Name:      main.cpp
// Purpose:   main for wex report unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

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
          m_Report = new listview(listview_data().
            type(listview_data::KEYWORD).lexer(&lexer));
          wex::test::add_pane(this, m_Report);};

      virtual listview* activate(
        listview_data::type_t listview_type, const lexer* lexer) override {
        // only for coverage
        report::frame::activate(listview_type, lexer);
        return m_Report;};

      void more_coverage() {
        file_history_list();};
    private:
      listview* m_Report;
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
      
        m_Frame = new test::frame();
        m_Frame->more_coverage();
        m_Frame->Show();
        
        return true;
      }
      
      static auto* frame() {return m_Frame;};
    private:
      inline static test::frame* m_Frame = nullptr;
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
