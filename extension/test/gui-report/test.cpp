////////////////////////////////////////////////////////////////////////////////
// Name:      test.cpp
// Purpose:   Implementation for wxExtension report unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/extension/frd.h>
#include <wx/extension/tool.h>
#include <wx/extension/util.h>
#include "test.h"

TEST_CASE("wxEx")
{
  wxExTool tool(ID_TOOL_REPORT_FIND);
  
  wxExListView* report = new wxExListView(
    GetFrame(), 
    wxExListView::LIST_FILE);
  
  AddPane(GetFrame(), report);
    
  wxArrayString files;
  
  REQUIRE(wxDir::GetAllFiles(
    "../../../extension", 
    &files,
    "*.cpp", 
    wxDIR_FILES | wxDIR_DIRS) > 10);
    
  wxExFindReplaceData* frd = wxExFindReplaceData::Get(); 
  
  // This string should occur only once, that is here!
  frd->SetUseRegEx(false);
  frd->SetFindString("@@@@@@@@@@@@@@@@@@@");
  
  REQUIRE(GetFrame()->FindInFiles(
    wxExToVectorString(files).Get(), 
    ID_TOOL_REPORT_FIND, 
    false, 
    report) == 1);
  
#ifdef __UNIX__    
  REQUIRE(report->GetItemCount() == 1);
#endif
  
  frd->SetFindString("Author:");
  
  wxStopWatch sw;
  sw.Start();

  REQUIRE(GetFrame()->FindInFiles(
    wxExToVectorString(files).Get(), 
    ID_TOOL_REPORT_FIND, 
    false, 
    report));
    
  const long find = sw.Time();
  
  REQUIRE(find < 1000);

  INFO(wxString::Format(
    "%d %lu items in: %ld ms", 
    report->GetItemCount(), wxExToVectorString(files).Get().size(), find).ToStdString());

#ifdef __UNIX__
  // Each file has one author (files.GetCount()), add the one in SetFindString 
  // above, and the one that is already present on the 
  // list because of the first FindInFiles.
  REQUIRE(report->GetItemCount() == (
    wxExToVectorString(files).Get().size() + 2));
#endif
}
