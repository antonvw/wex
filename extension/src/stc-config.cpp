////////////////////////////////////////////////////////////////////////////////
// Name:      stc-config.cpp
// Purpose:   Implementation of config related methods of class wxExSTC
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
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
    {_("Keep zoom"), ITEM_CHECKBOX, true},
    {_("Line number"), ITEM_TEXTCTRL_INT, 60l},
    {_("Print flags"), ITEM_TEXTCTRL_INT, (long)wxSTC_PRINT_BLACKONWHITE},
    {_("Scroll bars"), ITEM_CHECKBOX, true},
    {_("Search engine"), ITEM_COMBOBOX, std::string("https://duckduckgo.com")},
    {_("Show mode"), ITEM_CHECKBOX, true},
    {_("Tab draw mode"), ITEM_TEXTCTRL_INT, (long)wxSTC_TD_LONGARROW},
    {_("Tab font"), ITEM_FONTPICKERCTRL, wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT)},
    {_("Tab width"), ITEM_TEXTCTRL_INT, 2l},
    {_("Text font"), ITEM_FONTPICKERCTRL, wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT)},
    {_("vi mode"), ITEM_CHECKBOX, true},
    {_("vi tag fullpath"), ITEM_CHECKBOX, false}}) {;};
};
  
bool wxExSTC::AutoIndentation(int c)
{
  if (const auto ai = wxConfigBase::Get()->ReadLong(_("Auto indent"), INDENT_ALL);
    ai == INDENT_NONE)
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
      {_("General"),
        {{"stc-subnotebook", {
          {_("Page1"), 
            {{{_("End of line"),
               _("Line numbers"),
               _("Use tabs"),
               _("Caret line"),
               _("Scroll bars"),
               _("Auto complete"),
               _("Keep zoom"),
               _("vi mode"),
               _("vi tag fullpath")}},
            {_("Search engine"), ITEM_COMBOBOX}}},
          {_("Page2"), 
            {{_("Auto indent"), {
               {INDENT_NONE, _("None")},
               {INDENT_WHITESPACE, _("Whitespace")},
               {INDENT_LEVEL, _("Level")},
               {INDENT_ALL, _("Both")}}, true, 4},
             {_("Wrap visual flags"), {
               {wxSTC_WRAPVISUALFLAG_NONE, _("None")},
               {wxSTC_WRAPVISUALFLAG_END, _("End")},
               {wxSTC_WRAPVISUALFLAG_START, _("Start")},
               {wxSTC_WRAPVISUALFLAG_MARGIN, _("Margin")}}, true, 4},
             {_("Tab draw mode"), {
               {wxSTC_TD_LONGARROW, _("Longarrow")},
               {wxSTC_TD_STRIKEOUT, _("Strikeout")}}, true, 2},
             {_("Whitespace visible"), {
               {wxSTC_WS_INVISIBLE, _("Off")},
               {wxSTC_WS_VISIBLEAFTERINDENT, _("After indent")},
               {wxSTC_WS_VISIBLEALWAYS, _("Always")},
               {wxSTC_WS_VISIBLEONLYININDENT, _("Only indent")}}, true, 2},
             {_("Wrap line"), {
               {wxSTC_WRAP_NONE, _("None")},
               {wxSTC_WRAP_WORD, _("Word")},
               {wxSTC_WRAP_CHAR, _("Char")},
               {wxSTC_WRAP_WHITESPACE, _("Whitespace")}}, true, 4}}}} 
#ifdef __WXMSW__
            ,ITEM_NOTEBOOK_AUI
#endif
            }}},
      {_("Font"), 
        {{_("Default font"), ITEM_FONTPICKERCTRL},
         {_("Tab font"), ITEM_FONTPICKERCTRL},
         {_("Text font"), ITEM_FONTPICKERCTRL}}},
      {_("Edge"),
        {{_("Edge column"), 0, 80l},
         { _("Edge line"), {
           {wxSTC_EDGE_NONE, _("None")},
           {wxSTC_EDGE_LINE, _("Line")},
           {wxSTC_EDGE_BACKGROUND, _("Background")},
           {wxSTC_EDGE_MULTILINE, _("Multiline")}}, true, 1}}},
      {_("Margin"),
        {{_("Tab width"), 1, 500},
         {_("Indent"), 0, 500},
         {_("Divider"), 0, 40},
         {_("Folding"), 0, 40},
         {_("Line number"), 0, 100},
         {_("Autocomplete maxwidth"), 0, 100}}},
      {_("Folding"),
        {{_("Indentation guide"), ITEM_CHECKBOX},
         {_("Auto fold"), 0l, INT_MAX},
         {_("Fold flags"), {
             {wxSTC_FOLDFLAG_LINEBEFORE_EXPANDED, _("Line before expanded")},
             {wxSTC_FOLDFLAG_LINEBEFORE_CONTRACTED, _("Line before contracted")},
             {wxSTC_FOLDFLAG_LINEAFTER_EXPANDED, _("Line after expanded")},
             {wxSTC_FOLDFLAG_LINEAFTER_CONTRACTED, _("Line after contracted")},
             {wxSTC_FOLDFLAG_LEVELNUMBERS, _("Level numbers")}},
             false}}},
      {_("Directory"),
        {{_("Include directory"), wxExListViewData().Type(LIST_FOLDER).
          Window(wxExWindowData().Size({200, 200}))}}},
      {_("Printer"),
        {{_("Print flags"), {
           {wxSTC_PRINT_NORMAL, _("Normal")},
           {wxSTC_PRINT_INVERTLIGHT, _("Invert on white")},
           {wxSTC_PRINT_BLACKONWHITE, _("Black on white")},
           {wxSTC_PRINT_COLOURONWHITE, _("Colour on white")},
           {wxSTC_PRINT_COLOURONWHITEDEFAULTBG, _("Colour on white normal")}}, true, 1}}}
      }}},
      wxExWindowData(data).
        Title(data.Id() == wxID_PREFERENCES ? wxGetStockLabel(data.Id(), 0).ToStdString(): data.Title()));
  }

  return (data.Button() & wxAPPLY) ? 
    m_ConfigDialog->Show(): m_ConfigDialog->ShowModal();
}

void wxExSTC::ConfigGet()
{
  STCDefaults use;
  auto* cfg = use.Get();
  
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

  if (m_Lexer.GetEdgeMode() == wxExEdgeMode::ABSENT)
  {
    if (!m_Lexer.IsOk())
    {
      SetEdgeMode(wxSTC_EDGE_NONE);
    }
    else
    {
      SetEdgeColumn(cfg->ReadLong(_("Edge column"), 80l));
    
      if (const auto el = cfg->ReadLong(_("Edge line"), wxSTC_EDGE_NONE);
        el != wxSTC_EDGE_NONE)
      {
        SetEdgeMode(font.IsFixedWidth() ? el: wxSTC_EDGE_BACKGROUND);
      }
      else
      {
        SetEdgeMode(el);
      }
    }
  }
  
  AutoCompSetMaxWidth(cfg->ReadLong(_("Autocomplete maxwidth"), 0));
  SetCaretLineVisible(cfg->ReadBool(_("Caret line"), true));
  SetFoldFlags(cfg->ReadLong( _("Fold flags"), 0));
  SetIndent(cfg->ReadLong(_("Indent"), 0));
  SetIndentationGuides( cfg->ReadBool(_("Indentation guide"), false));
  SetMarginWidth(m_MarginDividerNumber,  cfg->ReadLong(_("Divider"), 0));
  SetPrintColourMode(cfg->ReadLong(_("Print flags"), 0));
  SetTabDrawMode(cfg->ReadLong(_("Tab draw mode"), wxSTC_TD_LONGARROW));
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
