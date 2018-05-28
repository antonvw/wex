////////////////////////////////////////////////////////////////////////////////
// Name:      main.cpp
// Purpose:   main for wxExtension report unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "test.h"

class FrameWithHistory : public wxExFrameWithHistory
{
public:
  FrameWithHistory()
    : wxExFrameWithHistory() {
      wxExLexer lexer("cpp");
      m_Report = new wxExListView(wxExListViewData().
        Type(LIST_KEYWORD).Lexer(&lexer));
      AddPane(this, m_Report);};

  virtual wxExListView* Activate(
    wxExListType list_type, const wxExLexer* lexer) override {
    // only for coverage
    wxExFrameWithHistory::Activate(list_type, lexer);
    return m_Report;};
  void MoreCoverage() {
    GetFileHistoryList();};
private:
  wxExListView* m_Report;
};

class wxExTestGuiApp : public wxExTestApp
{
public: 
  wxExTestGuiApp() : wxExTestApp() {;};

  bool OnInit() override
  {
    if (!wxExTestApp::OnInit())
    {
      return false;
    }
  
    m_Frame = new FrameWithHistory();
    m_Frame->MoreCoverage();
    m_Frame->Show();
    
    return true;
  }
  
  static auto* GetFrame() {return m_Frame;};
private:
  static FrameWithHistory* m_Frame;
}; 

FrameWithHistory* wxExTestGuiApp::m_Frame = nullptr;

wxExFrameWithHistory* GetFrame()
{
  return wxExTestGuiApp::GetFrame();
}
  
const std::string GetProject()
{
  return "test-rep.prj";
}
  
IMPLEMENT_APP_NO_MAIN(wxExTestGuiApp);

int main(int argc, char* argv[])
{
  return wxExTestMain(argc, argv, new wxExTestGuiApp());
}
