////////////////////////////////////////////////////////////////////////////////
// Name:      stc-config.cpp
// Purpose:   Implementation of config related methods of class wxExSTC
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <wx/config.h>
#include <wx/settings.h>
#include <wx/stockitem.h>
#include <wx/extension/stc.h>
#include <wx/extension/itemdlg.h>
#include <wx/extension/lexers.h>
#include <wx/extension/util.h>

enum
{
  INDENT_NONE,
  INDENT_WHITESPACE,
  INDENT_LEVEL,
  INDENT_ALL,
};

class STCDefaults : public wxExConfigDefaults
{
public:
  STCDefaults() 
  : wxExConfigDefaults({
    {_("Auto fold"), ITEM_TEXTCTRL_INT, 1500l},
    {_("Auto indent"), ITEM_TEXTCTRL_INT, (long)INDENT_ALL},
    {_("Caret line"), ITEM_CHECKBOX, true},
    {_("Default font"), ITEM_FONTPICKERCTRL, wxSystemSettings::GetFont(wxSYS_OEM_FIXED_FONT)},
    {_("Divider"), ITEM_TEXTCTRL_INT, 16l},
    {_("Edge column"), ITEM_TEXTCTRL_INT, 80l},
    {_("Edge line"), ITEM_TEXTCTRL_INT, (long)wxSTC_EDGE_NONE},
    {_("Fold flags"), ITEM_TEXTCTRL_INT, (long)wxSTC_FOLDFLAG_LINEBEFORE_CONTRACTED | wxSTC_FOLDFLAG_LINEAFTER_CONTRACTED},
    {_("Folding"), ITEM_TEXTCTRL_INT, 16l},
    {_("Indent"), ITEM_TEXTCTRL_INT, 2l},
    {_("Line number"), ITEM_TEXTCTRL_INT, 60l},
    {_("Print flags"), ITEM_TEXTCTRL_INT, (long)wxSTC_PRINT_BLACKONWHITE},
    {_("Scroll bars"), ITEM_CHECKBOX, true},
    {_("Search engine"), ITEM_COMBOBOX, wxString("https://duckduckgo.com")},
    {_("Show mode"), ITEM_CHECKBOX, true},
#if wxCHECK_VERSION(3,1,1)
    {_("Tab draw mode"), ITEM_TEXTCTRL_INT, (long)wxSTC_TD_LONGARROW},
#endif
    {_("Tab font"), ITEM_FONTPICKERCTRL, wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT)},
    {_("Tab width"), ITEM_TEXTCTRL_INT, 2l},
    {_("Text font"), ITEM_FONTPICKERCTRL, wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT)},
    {_("vi mode"), ITEM_CHECKBOX, true},
    {_("vi tag fullpath"), ITEM_CHECKBOX, true}}) {;};
};
  
wxExItemDialog* wxExSTC::m_ConfigDialog = nullptr;
wxExSTCEntryDialog* wxExSTC::m_EntryDialog = nullptr;

bool wxExSTC::AutoIndentation(int c)
{
  const long ai = wxConfigBase::Get()->ReadLong(_("Auto indent"), INDENT_ALL);
  
  if (ai == INDENT_NONE)
  {
    return false;
  }
  
  bool is_nl = false;
  
  switch (GetEOLMode())
  {
    case wxSTC_EOL_CR:   is_nl = (c == '\r'); break;
    case wxSTC_EOL_CRLF: is_nl = (c == '\n'); break; // so ignore first \r
    case wxSTC_EOL_LF:   is_nl = (c == '\n'); break;
  }
  
  const int currentLine = GetCurrentLine();
  
  if (!is_nl || currentLine == 0)
  {
    return false;
  }

  const int level = 
    (GetFoldLevel(currentLine) & wxSTC_FOLDLEVELNUMBERMASK) 
    - wxSTC_FOLDLEVELBASE;
  
  int indent = 0;
    
  if (level <= 0)
  {
    // the current line has yet no indents, so use previous line
    indent = GetLineIndentation(currentLine - 1);
    
    if (indent == 0)
    {
      return false;
    }
  }
  else
  {
    indent = GetIndent() * level;
  }
  
  BeginUndoAction();

  SetLineIndentation(currentLine, indent);
    
  if (level < m_FoldLevel && m_AddingChars)
  {
    SetLineIndentation(currentLine - 1, indent);
  }
  
  EndUndoAction();

  m_FoldLevel = level;
    
  GotoPos(GetLineIndentPosition(currentLine));

  return true;
}

int wxExSTC::ConfigDialog(const wxExWindowData& par)
{
  const wxExWindowData data(wxExWindowData(par).
    Title(_("Editor Options").ToStdString()));

  if (m_ConfigDialog == nullptr)
  {
    STCDefaults use;
  
    m_ConfigDialog = new wxExItemDialog({{"stc-notebook", {
      {_X("General"),
        {{"stc-subnotebook", {
          {_X("Page1"), 
            {{{_X("End of line"),
               _X("Line numbers"),
               _X("Use tabs"),
               _X("Caret line"),
               _X("Scroll bars"),
               _X("Autocomplete"),
               _X("vi mode"),
               _X("vi tag fullpath")}},
            {_X("Search engine"), ITEM_COMBOBOX}}},
          {_X("Page2"), 
            {{_X("Auto indent"), {
               {INDENT_NONE, _X("None")},
               {INDENT_WHITESPACE, _X("Whitespace")},
               {INDENT_LEVEL, _X("Level")},
               {INDENT_ALL, _X("Both")}}, true, 4},
             {_X("Wrap visual flags"), {
               {wxSTC_WRAPVISUALFLAG_NONE, _X("None")},
               {wxSTC_WRAPVISUALFLAG_END, _X("End")},
               {wxSTC_WRAPVISUALFLAG_START, _X("Start")},
               {wxSTC_WRAPVISUALFLAG_MARGIN, _X("Margin")}}, true, 4},
#if wxCHECK_VERSION(3,1,1)
             {_X("Tab draw mode"), {
               {wxSTC_TD_LONGARROW, _X("Longarrow")},
               {wxSTC_TD_STRIKEOUT, _X("Strikeout")}}, true, 2},
#endif
             {_X("Whitespace visible"), {
               {wxSTC_WS_INVISIBLE, _X("Off")},
               {wxSTC_WS_VISIBLEAFTERINDENT, _X("After indent")},
               {wxSTC_WS_VISIBLEALWAYS, _X("Always")}
#if wxCHECK_VERSION(3,1,1)
               ,{wxSTC_WS_VISIBLEONLYININDENT, _X("Only indent")}},
#else
               },
#endif  
               true, 2},
             {_X("Wrap line"), {
               {wxSTC_WRAP_NONE, _X("None")},
               {wxSTC_WRAP_WORD, _X("Word")},
               {wxSTC_WRAP_CHAR, _X("Char")}
#if wxCHECK_VERSION(3,1,0)
              ,{wxSTC_WRAP_WHITESPACE, _X("Whitespace")}},
#else
               },
#endif  
              true, 4}}}} 
#ifdef __WXMSW__
            ,ITEM_NOTEBOOK_AUI
#endif
            }}},
      {_X("Font"), 
        {{_X("Default font"), ITEM_FONTPICKERCTRL},
         {_X("Tab font"), ITEM_FONTPICKERCTRL},
         {_X("Text font"), ITEM_FONTPICKERCTRL}}},
      {_X("Edge"),
        {{_X("Edge column"), 0, 500},
         { _X("Edge line"), {
           {wxSTC_EDGE_NONE, _X("None")},
           {wxSTC_EDGE_LINE, _X("Line")},
           {wxSTC_EDGE_BACKGROUND, _X("Background")}
#if wxCHECK_VERSION(3,1,0)
           ,{wxSTC_EDGE_MULTILINE, _X("Multiline")}},
#else
           },
#endif
           true, 1}}},
      {_X("Margin"),
        {{_X("Tab width"), 1, 500},
         {_X("Indent"), 0, 500},
         {_X("Divider"), 0, 40},
         {_X("Folding"), 0, 40},
         {_X("Line number"), 0, 100},
         {_X("Autocomplete maxwidth"), 0, 100}}},
      {_X("Folding"),
        {{_X("Indentation guide"), ITEM_CHECKBOX},
         {_X("Auto fold"), 0l, INT_MAX},
         {_X("Fold flags"), {
             {wxSTC_FOLDFLAG_LINEBEFORE_EXPANDED, _X("Line before expanded")},
             {wxSTC_FOLDFLAG_LINEBEFORE_CONTRACTED, _X("Line before contracted")},
             {wxSTC_FOLDFLAG_LINEAFTER_EXPANDED, _X("Line after expanded")},
             {wxSTC_FOLDFLAG_LINEAFTER_CONTRACTED, _X("Line after contracted")},
             {wxSTC_FOLDFLAG_LEVELNUMBERS, _X("Level numbers")}},
             false}}},
      {_X("Printer"),
        {{_X("Print flags"), {
           {wxSTC_PRINT_NORMAL, _X("Normal")},
           {wxSTC_PRINT_INVERTLIGHT, _X("Invert on white")},
           {wxSTC_PRINT_BLACKONWHITE, _X("Black on white")},
           {wxSTC_PRINT_COLOURONWHITE, _X("Colour on white")},
           {wxSTC_PRINT_COLOURONWHITEDEFAULTBG, _X("Colour on white normal")}}, true, 1}}},
      {_X("Directory"),
        {{_X("Include directory"), wxExListViewData().Type(LIST_FOLDER).
          Window(wxExWindowData().Size({200, 200}))}}}}}},
      wxExWindowData(data).
        Title(data.Id() == wxID_PREFERENCES ? wxGetStockLabel(data.Id(), 0).ToStdString(): data.Title()).
        Size(wxSize(510, 500)));
  }

  return (data.Button() & wxAPPLY) ? 
    m_ConfigDialog->Show(): m_ConfigDialog->ShowModal();
}

void wxExSTC::ConfigGet()
{
  STCDefaults use;
  wxConfigBase* cfg = use.Get();
  
  const wxFont font(cfg->ReadObject(
    _("Default font"), wxSystemSettings::GetFont(wxSYS_ANSI_FIXED_FONT)));

  if (m_DefaultFont != font)
  {
    m_DefaultFont = font;
    
    StyleResetDefault();
    
    // Doing this once is enough, not yet possible.
    wxExLexers::Get()->LoadDocument();
    
    m_Lexer.Apply();
  }

  if (GetFileName().GetExtension().find(".log") == 0)
  {
    SetEdgeMode(wxSTC_EDGE_NONE);
  }
  else
  {
    SetEdgeColumn(cfg->ReadLong(_("Edge column"), 0));
  
    const int el = cfg->ReadLong(_("Edge line"), wxSTC_EDGE_NONE);
  
    if (el != wxSTC_EDGE_NONE)
    {
      SetEdgeMode(font.IsFixedWidth() ? el: wxSTC_EDGE_BACKGROUND);
    }
    else
    {
      SetEdgeMode(el);
    }
  }
  
  AutoCompSetMaxWidth(cfg->ReadLong(_("Autocomplete maxwidth"), 0));
  SetCaretLineVisible(cfg->ReadBool(_("Caret line"), true));
  SetFoldFlags(cfg->ReadLong( _("Fold flags"), 0));
  SetIndent(cfg->ReadLong(_("Indent"), 0));
  SetIndentationGuides( cfg->ReadBool(_("Indentation guide"), false));
  SetMarginWidth(m_MarginDividerNumber,  cfg->ReadLong(_("Divider"), 0));
  SetPrintColourMode(cfg->ReadLong(_("Print flags"), 0));
#if wxCHECK_VERSION(3,1,1)
  SetTabDrawMode(cfg->ReadLong(_("Tab draw mode"), wxSTC_TD_LONGARROW));
#endif
  SetTabWidth(cfg->ReadLong(_("Tab width"), 0));
  SetUseHorizontalScrollBar(cfg->ReadBool(_("Scroll bars"), true));
  SetUseTabs(cfg->ReadBool(_("Use tabs"), false));
  SetUseVerticalScrollBar(cfg->ReadBool(_("Scroll bars"), true));
  SetViewEOL(cfg->ReadBool(_("End of line"), false));
  SetViewWhiteSpace(cfg->ReadLong(_("Whitespace visible"), wxSTC_WS_INVISIBLE));
  SetWrapMode(cfg->ReadLong(_("Wrap line"), wxSTC_WRAP_NONE));
  SetWrapVisualFlags(cfg->ReadLong(_("Wrap visual flags"),  wxSTC_WRAPVISUALFLAG_END));
  m_vi.Use(cfg->ReadBool(_("vi mode"), false));

  ShowLineNumbers(cfg->ReadBool(_("Line numbers"), false));

  m_Link.SetFromConfig();
}
