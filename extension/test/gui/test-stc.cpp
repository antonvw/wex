////////////////////////////////////////////////////////////////////////////////
// Name:      test-stc->cpp
// Purpose:   Implementation for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/extension/stc.h>
#include <wx/extension/defs.h>
#include <wx/extension/frd.h>
#include <wx/extension/indicator.h>
#include <wx/extension/managedframe.h>
#include "test.h"

void fixture::testSTC()
{
#if wxCHECK_VERSION(3,1,0)
  wxExSTC::ConfigDialog(m_Frame, "test stc", wxExSTC::STC_CONFIG_MODELESS);
#endif
  
  wxExSTC* stc = new wxExSTC(m_Frame, "hello stc");
  wxExSTC* copy = new wxExSTC(*stc);
  
  AddPane(m_Frame, stc);
  AddPane(m_Frame, copy);
  
  CPPUNIT_ASSERT( stc->GetText() == "hello stc");
  CPPUNIT_ASSERT( stc->FindNext(wxString("hello")));
  CPPUNIT_ASSERT( stc->GetWordAtPos(0) == "hello");
  
  CPPUNIT_ASSERT(!stc->FindNext(wxString("%d")));
  CPPUNIT_ASSERT(!stc->FindNext(wxString("%ld")));
  CPPUNIT_ASSERT(!stc->FindNext(wxString("%q")));
  
  CPPUNIT_ASSERT( stc->FindNext(wxString("hello"), wxSTC_FIND_WHOLEWORD));
  CPPUNIT_ASSERT(!stc->FindNext(wxString("HELLO"), wxSTC_FIND_MATCHCASE));
  CPPUNIT_ASSERT( stc->GetSearchFlags() & wxSTC_FIND_MATCHCASE);
  
  wxExFindReplaceData::Get()->SetMatchCase(false);
  CPPUNIT_ASSERT( stc->FindNext(wxString("HELLO"))); // uses flags from frd
  
  CPPUNIT_ASSERT( stc->AllowChangeIndicator());
  
  stc->AppendText("more text");
  
  CPPUNIT_ASSERT( stc->GetText() != "hello stc");
  
  CPPUNIT_ASSERT( stc->CanCut());
  stc->Copy();
  CPPUNIT_ASSERT( stc->CanPaste());
  
  stc->DocumentStart();
  wxExFindReplaceData::Get()->SetMatchWord(false);
  CPPUNIT_ASSERT( stc->FindNext(wxString("more text")));
  CPPUNIT_ASSERT( stc->GetFindString() == "more text");
  CPPUNIT_ASSERT( stc->ReplaceAll("more", "less") == 1);
  CPPUNIT_ASSERT( stc->ReplaceAll("more", "less") == 0);
  CPPUNIT_ASSERT(!stc->FindNext(wxString("more text")));
  stc->SelectNone();
  CPPUNIT_ASSERT(!stc->FindNext());
  CPPUNIT_ASSERT( stc->FindNext(wxString("less text")));
  CPPUNIT_ASSERT( stc->ReplaceNext("less text", ""));
  CPPUNIT_ASSERT(!stc->ReplaceNext());
  CPPUNIT_ASSERT(!stc->FindNext(wxString("less text")));
  CPPUNIT_ASSERT( stc->GetFindString() != "less text");
  CPPUNIT_ASSERT( stc->ReplaceAll("%", "percent") == 0);
  
  stc->GotoLineAndSelect(1);
  CPPUNIT_ASSERT(stc->GetCurrentLine() == 0);
  CPPUNIT_ASSERT(stc->GetCurrentPos() == 0);
  stc->GotoLineAndSelect(1, wxEmptyString, 5);
  CPPUNIT_ASSERT(stc->GetCurrentLine() == 0);
  CPPUNIT_ASSERT(stc->GetCurrentPos() == 4);
  
  stc->SetText("more text\notherline");
  stc->GetVi().Command("V");
  stc->GetVi().GetMode() == wxExVi::MODE_VISUAL_LINE;
  CPPUNIT_ASSERT( stc->FindNext(wxString("more text")));
  
  stc->SetText("new text");
  CPPUNIT_ASSERT(stc->GetText() == "new text");
  
  CPPUNIT_ASSERT(stc->SetLexer("cpp"));
  CPPUNIT_ASSERT(stc->GetLexer().GetScintillaLexer() == "cpp");
  stc->ResetLexer();
  CPPUNIT_ASSERT(stc->GetLexer().GetScintillaLexer().empty());

  wxExLexer lexer;
  CPPUNIT_ASSERT( lexer.Reset(stc));
  CPPUNIT_ASSERT( lexer.Set("cpp", stc, false));
  CPPUNIT_ASSERT(!lexer.Set("xyz", stc, false));
  CPPUNIT_ASSERT( stc->SetLexer(lexer));
  
  // do the same test as with wxExFile in base for a binary file
  CPPUNIT_ASSERT(stc->Open(wxExFileName(GetTestDir() + "test.bin")));
  CPPUNIT_ASSERT(stc->GetFlags() == 0);
  const wxCharBuffer& buffer = stc->GetTextRaw();
  CPPUNIT_ASSERT(buffer.length() == 40);

  stc->AddText("hello");
  CPPUNIT_ASSERT( stc->GetFile().GetContentsChanged());
  stc->GetFile().ResetContentsChanged();
  CPPUNIT_ASSERT(!stc->GetFile().GetContentsChanged());
  
  stc->ConfigGet();
  
  stc->Cut();
  
  //  stc->FileTypeMenu();
  
  stc->Fold();
  stc->Fold(true); // FoldAll
  
  CPPUNIT_ASSERT(!stc->GetEOL().empty());
  
  stc->GuessType();
  
  CPPUNIT_ASSERT(stc->MarkerDeleteAllChange());
  
  stc->Paste();
  
  CPPUNIT_ASSERT(!stc->PositionRestore());
  stc->PositionSave();
  CPPUNIT_ASSERT( stc->PositionRestore());
  
  //  stc->Print();
  stc->PrintPreview();
  
  stc->ProcessChar(5);
  
  stc->PropertiesMessage();
  
  stc->Reload();
  
  stc->ResetMargins();
  
  stc->SelectNone();
  
  CPPUNIT_ASSERT(!stc->SetIndicator(wxExIndicator(4,5), 100, 200));
  
  stc->SetLexerProperty("xx", "yy");
  
  CPPUNIT_ASSERT(!(stc->GetSearchFlags() & wxSTC_FIND_MATCHCASE));
  wxExFindReplaceData::Get()->SetMatchCase(false);
  stc->SetSearchFlags(-1);
  CPPUNIT_ASSERT(!(stc->GetSearchFlags() & wxSTC_FIND_MATCHCASE));

  // Test AutoIndentation
  // first test auto indentation on next line
  wxConfigBase::Get()->Write(_("Auto indent"), 3);
  CPPUNIT_ASSERT( wxConfigBase::Get()->ReadLong(_("Auto indent"), 1) == 3);
  stc->SetText("  \n  line with indentation");
  stc->DocumentEnd();
  CPPUNIT_ASSERT(!stc->AutoIndentation('x'));
  CPPUNIT_ASSERT( stc->GetText() == "  \n  line with indentation");
  CPPUNIT_ASSERT( stc->GetLineCount() == 2);
  CPPUNIT_ASSERT( stc->AutoIndentation('\n'));
  // the \n is not added, but indentation does
  CPPUNIT_ASSERT( stc->GetText() == "  \n  line with indentation");
  CPPUNIT_ASSERT( stc->GetLineCount() == 2);
  // test auto indentation for level change
  CPPUNIT_ASSERT( stc->SetLexer("cpp"));
  stc->SetText("\nif ()\n{\n");
  stc->DocumentEnd();
#if wxCHECK_VERSION(3,1,0)
//  CPPUNIT_ASSERT( stc->AutoIndentation('\n'));
#endif
  
  stc->Sync(false);
  stc->Sync(true);
  
  stc->ShowLineNumbers(false);
  stc->ShowLineNumbers(true);
  
  stc->Undo();
  
  stc->UseAutoComplete(true);
  stc->UseAutoComplete(false);
  
  stc->UseModificationMarkers(true);
  stc->UseModificationMarkers(false);
  
  stc->ClearDocument();
  
  stc->Reload(wxExSTC::STC_WIN_HEX);
  CPPUNIT_ASSERT(stc->HexMode());
  stc->GetHexMode().AppendText("in hex mode");
  
  // Test stc with file (not yet done ???).
  wxExSTC stc2(m_Frame, GetTestFile());
  
  CPPUNIT_ASSERT( stc2.GetFileName().GetFullPath().Contains("test.h"));
  CPPUNIT_ASSERT( stc2.Open(GetTestFile()));
  CPPUNIT_ASSERT(!stc2.Open(wxExFileName("XXX")));
  
  stc2.PropertiesMessage();
  
  stc->Reload();
  
  // Test events.
  for (auto id : std::vector<int> {
    ID_EDIT_HEX_DEC_CALLTIP, ID_EDIT_MARKER_NEXT, ID_EDIT_MARKER_PREVIOUS,
    ID_EDIT_OPEN_LINK, ID_EDIT_SHOW_PROPERTIES, ID_EDIT_ZOOM_IN, ID_EDIT_ZOOM_OUT}) 
  {
    wxPostEvent(stc, wxCommandEvent(wxEVT_MENU, id));
  }
  
  stc->LineHome();
  stc->LineHomeExtend();
  stc->LineHomeRectExtend();
  stc->ParaUpRectExtend();
  stc->ParaDownRectExtend();
  stc->WordLeftRectExtend();
  stc->WordRightRectExtend();
  stc->WordRightEndRectExtend();
  
  stc->Clear();
}
