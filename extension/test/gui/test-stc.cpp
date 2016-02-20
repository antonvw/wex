////////////////////////////////////////////////////////////////////////////////
// Name:      test-stc->cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
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

TEST_CASE("wxExSTC", "[stc][vi]")
{
#if wxCHECK_VERSION(3,1,0)
  wxExSTC::ConfigDialog(GetFrame(), "test stc", wxExSTC::STC_CONFIG_MODELESS);
#endif
  
  wxExSTC* stc = GetSTC();

  stc->GetVi().Command("\x1b");
  REQUIRE(stc->GetVi().GetMode() == wxExVi::MODE_NORMAL);
  
  stc->SetText("hello stc");
  REQUIRE( stc->GetText() == "hello stc");

  REQUIRE( stc->FindNext(wxString("hello")));
  REQUIRE( stc->GetWordAtPos(0) == "hello");
  
  REQUIRE(!stc->FindNext(wxString("%d")));
  REQUIRE(!stc->FindNext(wxString("%ld")));
  REQUIRE(!stc->FindNext(wxString("%q")));
  
  REQUIRE( stc->FindNext(wxString("hello"), wxSTC_FIND_WHOLEWORD));
  REQUIRE(!stc->FindNext(wxString("HELLO"), wxSTC_FIND_MATCHCASE));
  REQUIRE((stc->GetSearchFlags() & wxSTC_FIND_MATCHCASE));
  
  wxExFindReplaceData::Get()->SetMatchCase(false);
  REQUIRE( stc->FindNext(wxString("HELLO"))); // uses flags from frd
  
  REQUIRE( stc->AllowChangeIndicator());
  
  stc->AppendText("more text");
  
  REQUIRE( stc->GetText() != "hello stc");
  
  REQUIRE( stc->CanCut());
  stc->Copy();
  REQUIRE( stc->CanPaste());
  
  stc->DocumentStart();
  wxExFindReplaceData::Get()->SetMatchWord(false);
  REQUIRE( stc->FindNext(wxString("more text")));
  INFO (stc->GetSelectedText() << stc->GetVi().GetMode());
  REQUIRE( stc->GetFindString() == "more text");
  REQUIRE( stc->ReplaceAll("more", "less") == 1);
  REQUIRE( stc->ReplaceAll("more", "less") == 0);
  REQUIRE(!stc->FindNext(wxString("more text")));
  stc->SelectNone();
  REQUIRE(!stc->FindNext());
  REQUIRE( stc->FindNext(wxString("less text")));
  REQUIRE( stc->ReplaceNext("less text", ""));
  REQUIRE(!stc->ReplaceNext());
  REQUIRE(!stc->FindNext(wxString("less text")));
  REQUIRE( stc->GetFindString() != "less text");
  REQUIRE( stc->ReplaceAll("%", "percent") == 0);
  
  stc->GotoLineAndSelect(1);
  REQUIRE(stc->GetCurrentLine() == 0);
  REQUIRE(stc->GetCurrentPos() == 0);
  stc->GotoLineAndSelect(1, wxEmptyString, 5);
  REQUIRE(stc->GetCurrentLine() == 0);
  REQUIRE(stc->GetCurrentPos() == 4);
  
  stc->SetText("more text\notherline");
  stc->GetVi().Command("V");
  REQUIRE( stc->GetVi().GetMode() == wxExVi::MODE_VISUAL_LINE);
  REQUIRE( stc->FindNext(wxString("more text")));
  
  stc->SetText("new text");
  REQUIRE(stc->GetText() == "new text");
  
  REQUIRE(stc->SetLexer("cpp"));
  REQUIRE(stc->GetLexer().GetScintillaLexer() == "cpp");
  stc->ResetLexer();
  REQUIRE(stc->GetLexer().GetScintillaLexer().empty());

  wxExLexer lexer;
  REQUIRE( lexer.Reset(stc));
  REQUIRE( lexer.Set("cpp", stc, false));
  REQUIRE(!lexer.Set("xyz", stc, false));
  REQUIRE( stc->SetLexer(lexer));
  
  // do the same test as with wxExFile in base for a binary file
  REQUIRE(stc->Open(wxExFileName(GetTestDir() + "test.bin")));
  REQUIRE(stc->GetFlags() == 0);
  const wxCharBuffer& buffer = stc->GetTextRaw();
  REQUIRE(buffer.length() == 40);

  stc->AddText("hello");
  REQUIRE( stc->GetFile().GetContentsChanged());
  stc->GetFile().ResetContentsChanged();
  REQUIRE(!stc->GetFile().GetContentsChanged());
  
  stc->ConfigGet();
  
  stc->Cut();
  
  //  stc->FileTypeMenu();
  
  stc->Fold();
  stc->Fold(true); // FoldAll
  
  REQUIRE(!stc->GetEOL().empty());
  
  stc->GuessType();
  
  REQUIRE(stc->MarkerDeleteAllChange());
  
  stc->Paste();
  
  REQUIRE(!stc->PositionRestore());
  stc->PositionSave();
  REQUIRE( stc->PositionRestore());
  
  //  stc->Print();
  stc->PrintPreview();
  
  stc->ProcessChar(5);
  
  stc->PropertiesMessage();
  
  stc->Reload();
  
  stc->ResetMargins();
  
  stc->SelectNone();
  
  REQUIRE(!stc->SetIndicator(wxExIndicator(4,5), 100, 200));
  
  stc->SetLexerProperty("xx", "yy");
  
  REQUIRE(!(stc->GetSearchFlags() & wxSTC_FIND_MATCHCASE));
  wxExFindReplaceData::Get()->SetMatchCase(false);
  stc->SetSearchFlags(-1);
  REQUIRE(!(stc->GetSearchFlags() & wxSTC_FIND_MATCHCASE));

  // Test AutoIndentation
  // first test auto indentation on next line
  wxConfigBase::Get()->Write(_("Auto indent"), 3);
  REQUIRE( wxConfigBase::Get()->ReadLong(_("Auto indent"), 1) == 3);
  stc->SetText("  \n  line with indentation");
  stc->DocumentEnd();
  REQUIRE(!stc->AutoIndentation('x'));
  REQUIRE( stc->GetText() == "  \n  line with indentation");
  REQUIRE( stc->GetLineCount() == 2);
  REQUIRE( stc->AutoIndentation('\n'));
  // the \n is not added, but indentation does
  REQUIRE( stc->GetText() == "  \n  line with indentation");
  REQUIRE( stc->GetLineCount() == 2);
  // test auto indentation for level change
  REQUIRE( stc->SetLexer("cpp"));
  stc->SetText("\nif ()\n{\n");
  stc->DocumentEnd();
#if wxCHECK_VERSION(3,1,0)
//  REQUIRE( stc->AutoIndentation('\n'));
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
  REQUIRE(stc->HexMode());
  stc->GetHexMode().AppendText("in hex mode");
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
  stc->LineScrollDownExtend();
  stc->LineScrollDownRectExtend();
  stc->LineScrollUpExtend();
  stc->LineScrollUpRectExtend();
  stc->ParaUpRectExtend();
  stc->ParaDownRectExtend();
  stc->WordLeftRectExtend();
  stc->WordRightRectExtend();
  stc->WordRightEndRectExtend();
  
  stc->Clear();
  
  SECTION("load file")
  {
    wxExSTC stc(GetFrame(), GetTestFile());
    REQUIRE( stc.GetFileName().GetFullPath().Contains("test.h"));
    REQUIRE( stc.Open(GetTestFile()));
    REQUIRE(!stc.Open(wxExFileName("XXX")));
    stc.PropertiesMessage();
  }
  
  TestAndContinue(stc, [](wxWindow* window) {
    wxPostEvent(window, wxMouseEvent(wxEVT_RIGHT_DOWN));});
}
