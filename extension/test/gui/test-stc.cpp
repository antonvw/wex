////////////////////////////////////////////////////////////////////////////////
// Name:      test-stc.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
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

TEST_CASE("wxExSTC")
{
#if wxCHECK_VERSION(3,1,0)
  wxExSTC::ConfigDialog(GetFrame(), "test stc", STC_CONFIG_MODELESS);
#endif
  
  wxExSTC* stc = GetSTC();
  stc->GetVi().Command("\x1b");
  
  SUBCASE("SetText")
  {
    stc->SetText("hello stc");
    REQUIRE( stc->GetText() == "hello stc");
  }
  
  SUBCASE("Find and Replace")
  {
    stc->SetText("hello stc and more text");
    REQUIRE( stc->FindNext(std::string("hello")));
    REQUIRE( stc->GetWordAtPos(0) == "hello");
    
    REQUIRE(!stc->FindNext(std::string("%d")));
    REQUIRE(!stc->FindNext(std::string("%ld")));
    REQUIRE(!stc->FindNext(std::string("%q")));
    
    REQUIRE( stc->FindNext(std::string("hello"), wxSTC_FIND_WHOLEWORD));
    REQUIRE(!stc->FindNext(std::string("HELLO"), wxSTC_FIND_MATCHCASE));
    REQUIRE((stc->GetSearchFlags() & wxSTC_FIND_MATCHCASE) > 0);
    
    wxExFindReplaceData::Get()->SetMatchCase(false);
    REQUIRE( stc->FindNext(std::string("HELLO"))); // uses flags from frd
    
    REQUIRE(!stc->SetIndicator(wxExIndicator(4,5), 100, 200));
    REQUIRE(!(stc->GetSearchFlags() & wxSTC_FIND_MATCHCASE));
    wxExFindReplaceData::Get()->SetMatchCase(false);
    stc->SetSearchFlags(-1);
    REQUIRE(!(stc->GetSearchFlags() & wxSTC_FIND_MATCHCASE));
    
    REQUIRE( stc->CanCut());
    stc->Copy();
    REQUIRE( stc->CanPaste());
    
    stc->DocumentStart();
    wxExFindReplaceData::Get()->SetMatchWord(false);
    REQUIRE( stc->FindNext(std::string("more text")));
    REQUIRE( stc->GetFindString() == "more text");
    REQUIRE( stc->ReplaceAll("more", "less") == 1);
    REQUIRE( stc->ReplaceAll("more", "less") == 0);
    REQUIRE(!stc->FindNext(std::string("more text")));
    stc->SelectNone();
    REQUIRE(!stc->FindNext());
    REQUIRE( stc->FindNext(std::string("less text")));
    REQUIRE( stc->ReplaceNext("less text", ""));
    REQUIRE(!stc->ReplaceNext());
    REQUIRE(!stc->FindNext(std::string("less text")));
    REQUIRE( stc->GetFindString() != "less text");
    REQUIRE( stc->ReplaceAll("%", "percent") == 0);
  }

  SUBCASE("vi")
  {
    stc->GetVi().Command("\x1b");
    REQUIRE(stc->GetVi().GetMode() == wxExVi::MODE_NORMAL);
    stc->SetText("more text\notherline");
    stc->GetVi().Command("V");
    REQUIRE( stc->GetVi().GetMode() == wxExVi::MODE_VISUAL_LINE);
    REQUIRE( stc->FindNext(std::string("more text")));
  }

  SUBCASE("Lexer")
  {
    stc->SetText("new text");
    REQUIRE(stc->GetLexer().Set("cpp"));
    REQUIRE(stc->GetLexer().GetScintillaLexer() == "cpp");
    stc->GetLexer().Reset();
    REQUIRE(stc->GetLexer().GetScintillaLexer().empty());

    wxExLexer lexer;
    lexer.Reset();
    REQUIRE( lexer.Set("cpp", true));
    REQUIRE(!lexer.Set("xyz"));
    REQUIRE( stc->GetLexer().Set(lexer));
  }

  SUBCASE("Open")
  {
    // do the same test as with wxExFile in base for a binary file
    REQUIRE(stc->Open(GetTestDir() + "test.bin"));
    REQUIRE(stc->GetData().Flags() == 0);
    const wxCharBuffer& buffer = stc->GetTextRaw();
    REQUIRE(buffer.length() == 40);
  }
  
  SUBCASE("AddText and AppendText and ContentsChanged")
  {
    stc->AddText("added text");
    REQUIRE( stc->GetText().Contains("added text"));
    REQUIRE( stc->GetFile().GetContentsChanged());
    stc->GetFile().ResetContentsChanged();
    REQUIRE(!stc->GetFile().GetContentsChanged());

    stc->AppendText("more text");
    REQUIRE( stc->GetText() != "hello stc");
  }
  
  SUBCASE("Marker")
  {
    REQUIRE(stc->MarkerDeleteAllChange());
  }

  SUBCASE("Coverage")
  {
    stc->GetLexer().Set("cpp");
    stc->Clear();
    stc->ClearDocument();
    stc->ConfigGet();
    stc->Cut();
    //  stc->FileTypeMenu();
    stc->Fold();
    // FoldAll
    wxConfigBase::Get()->Write(_("Auto fold"), 3);
    stc->Fold(true); 
    stc->GuessType();
    stc->Paste();
    //  stc->Print();
    stc->PrintPreview();
    stc->ProcessChar(5);
    stc->PropertiesMessage();
    stc->Reload();
    stc->ResetMargins();
    stc->SelectNone();
    stc->Sync(false);
    stc->Sync(true);
    stc->ShowLineNumbers(false);
    stc->ShowLineNumbers(true);
    stc->Undo();
    stc->UseAutoComplete(true);
    stc->UseAutoComplete(false);
    stc->UseModificationMarkers(true);
    stc->UseModificationMarkers(false);
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
  }

  SUBCASE("EOL")
  {
    REQUIRE(!stc->GetEOL().empty());
  }
    
  SUBCASE("PositionRestore and Save")
  {
    REQUIRE(!stc->PositionRestore());
    stc->PositionSave();
    REQUIRE( stc->PositionRestore());
  }
    
  SUBCASE("AutoIndentation")
  {
    // first test auto indentation on next line
    wxConfigBase::Get()->Write(_("Auto indent"), 3);
    REQUIRE( wxConfigBase::Get()->ReadLong(_("Auto indent"), 1) == 3);
    stc->SetText("  \n  line with indentation");
    stc->DocumentEnd();
    REQUIRE(!stc->AutoIndentation('x'));
    REQUIRE( stc->GetText() == "  \n  line with indentation");
    REQUIRE( stc->GetLineCount() == 2);
#ifdef __WXOSX__
    stc->SetEOLMode(wxSTC_EOL_CR);
    REQUIRE( stc->AutoIndentation('\r'));
#else
    REQUIRE( stc->AutoIndentation('\n'));
#endif
    // the \n is not added, but indentation does
    REQUIRE( stc->GetText() == "  \n  line with indentation");
    REQUIRE( stc->GetLineCount() == 2);
    // test auto indentation for level change
    REQUIRE( stc->GetLexer().Set("cpp"));
    stc->SetText("\nif ()\n{\n");
    stc->DocumentEnd();
#if wxCHECK_VERSION(3,1,0)
    //  REQUIRE( stc->AutoIndentation('\n'));
#endif
  }
  
  SUBCASE("hex")
  {
    stc->Reload(STC_WIN_HEX);
    REQUIRE(stc->HexMode());
    stc->GetHexMode().AppendText("in hex mode");
    stc->Reload();
  }

  SUBCASE("events")
  {
    for (auto id : std::vector<int> {
      ID_EDIT_HEX_DEC_CALLTIP, ID_EDIT_MARKER_NEXT, ID_EDIT_MARKER_PREVIOUS,
      ID_EDIT_OPEN_LINK, ID_EDIT_SHOW_PROPERTIES, ID_EDIT_ZOOM_IN, ID_EDIT_ZOOM_OUT}) 
    {
      wxPostEvent(stc, wxCommandEvent(wxEVT_MENU, id));
    }
  }

  SUBCASE("load file")
  {
    wxExSTC stc(GetFrame(), GetTestFile());
    REQUIRE( stc.GetFileName().GetFullPath().find("test.h") != std::string::npos);
    REQUIRE( stc.Open(GetTestFile()));
    REQUIRE(!stc.Open("XXX"));
    stc.PropertiesMessage();
  }

  SUBCASE("xml complete")
  {
    REQUIRE( stc->GetLexer().Set("xml"));
    stc->GetVi().Command("i<xxxx>");
    stc->GetVi().Command("\x1b");
  }

  SUBCASE("popup")
  {
    REQUIRE( stc->GetLexer().Set("cpp"));
    REQUIRE(wxExUIAction(stc));
  }
}
