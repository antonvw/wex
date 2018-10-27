////////////////////////////////////////////////////////////////////////////////
// Name:      main.cpp
// Purpose:   main for wex report unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "test.h"

namespace wex
{
  class rptframe : public history_frame
  {
  public:
    rptframe()
      : history_frame() {
        lexer lexer("cpp");
        m_Report = new listview(listview_data().
          Type(listview_data::KEYWORD).Lexer(&lexer));
        AddPane(this, m_Report);};

    virtual listview* Activate(
      listview_data::type listview_type, const lexer* lexer) override {
      // only for coverage
      history_frame::Activate(listview_type, lexer);
      return m_Report;};
    void MoreCoverage() {
      GetFileHistoryList();};
  private:
    listview* m_Report;
  };

  class gui_test_app : public test_app
  {
  public: 
    gui_test_app() : test_app() {;};

    bool OnInit() override
    {
      if (!test_app::OnInit())
      {
        return false;
      }
    
      m_Frame = new rptframe();
      m_Frame->MoreCoverage();
      m_Frame->Show();
      
      return true;
    }
    
    static auto* GetFrame() {return m_Frame;};
  private:
    static rptframe* m_Frame;
  }; 
};

wex::rptframe* wex::gui_test_app::m_Frame = nullptr;

wex::history_frame* GetFrame()
{
  return wex::gui_test_app::GetFrame();
}
  
const std::string GetProject()
{
  return "test-rep.prj";
}
  
IMPLEMENT_APP_NO_MAIN(wex::gui_test_app);

int main(int argc, char* argv[])
{
  return wex::testmain(argc, argv, new wex::gui_test_app());
}
