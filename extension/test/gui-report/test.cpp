////////////////////////////////////////////////////////////////////////////////
// Name:      test.cpp
// Purpose:   Implementation for wxExtension report cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/extension/frd.h>
#include <wx/extension/tool.h>
#include <wx/extension/util.h>
#include "test.h"

#define TEST_PRJ "./test-rep.prj"

class FrameWithHistory : public wxExFrameWithHistory
{
public:
  FrameWithHistory(wxWindow* parent,
    wxWindowID id,
    const wxString& title,
    size_t maxFiles = 9,
    size_t maxProjects = 0,
    int style = wxDEFAULT_FRAME_STYLE);

  virtual wxExListView* Activate(
    wxExListView::wxExListType list_type, 
    const wxExLexer* lexer) override;
private:
  wxExListView* m_Report;
};

wxExFrameWithHistory* fixture::m_Frame = nullptr;

fixture::fixture() : m_Project("test-rep.prj")
{
  if (m_Frame == nullptr)
  {
    m_Frame = new FrameWithHistory(nullptr, wxID_ANY, wxTheApp->GetAppDisplayName());
    m_Frame->Show();
  }
}

void fixture::test()
{
  wxExTool tool(ID_TOOL_REPORT_FIND);
  
  wxExListView* report = new wxExListView(
    m_Frame, 
    wxExListView::LIST_FILE);
    
  wxArrayString files;
  
  CPPUNIT_ASSERT(wxDir::GetAllFiles(
    "../../../extension", 
    &files,
    "*.cpp", 
    wxDIR_FILES | wxDIR_DIRS) > 10);
    
  wxExFindReplaceData* frd = wxExFindReplaceData::Get(); 
  
  // This string should occur only once, that is here!
  frd->SetUseRegEx(false);
  frd->SetFindString("@@@@@@@@@@@@@@@@@@@");
  
  CPPUNIT_ASSERT(m_Frame->FindInFiles(
    wxExToVectorString(files).Get(), 
    ID_TOOL_REPORT_FIND, 
    false, 
    report));
    
  CPPUNIT_ASSERT(report->GetItemCount() == 1);
  
  frd->SetFindString("Author:");
  
  wxStopWatch sw;
  sw.Start();

  CPPUNIT_ASSERT(m_Frame->FindInFiles(
    wxExToVectorString(files).Get(), 
    ID_TOOL_REPORT_FIND, 
    false, 
    report));
    
  const long find = sw.Time();
  
  CPPUNIT_ASSERT(find < 1000);

  Report(wxString::Format(
    "wxExFrameWithHistory::FindInFiles %d items in: %ld ms", 
    report->GetItemCount(), find).ToStdString());
    
  wxLogMessage("%d %lu", 
    report->GetItemCount(), 
    wxExToVectorString(files).Get().size());
    
  // Each file has one author (files.GetCount()), add the one in SetFindString 
  // above, and the one that is already present on the 
  // list because of the first FindInFiles.
  CPPUNIT_ASSERT(report->GetItemCount() == (
    wxExToVectorString(files).Get().size() + 2));
}

FrameWithHistory::FrameWithHistory(wxWindow* parent,
  wxWindowID id,
  const wxString& title,
  size_t maxFiles,
  size_t maxProjects,
  int style)
  : wxExFrameWithHistory(parent, id, title, maxFiles, maxProjects, style)
{
  wxExLexer lexer("cpp");
  m_Report = new wxExListView(
    this, 
    wxExListView::LIST_KEYWORD,
    wxID_ANY,
    &lexer);
  AddPane(this, m_Report);
}

wxExListView* FrameWithHistory::Activate(
  wxExListView::wxExListType list_type, 
  const wxExLexer* lexer)
{
  return m_Report;
}
