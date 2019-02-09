////////////////////////////////////////////////////////////////////////////////
// Name:      stc-config.cpp
// Purpose:   Implementation of config related methods of class wex::stc
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <wx/settings.h>
#include <wx/stockitem.h>
#include <wex/stc.h>
#include <wex/config.h>
#include <wex/itemdlg.h>
#include <wex/lexers.h>
#include <wex/util.h>

namespace wex
{
  enum
  {
    INDENT_NONE,
    INDENT_WHITESPACE,
    INDENT_LEVEL,
    INDENT_ALL,
  };

  class stc_defaults : public config_defaults
  {
  public:
    stc_defaults() 
    : config_defaults({
      {_("Auto fold"), item::TEXTCTRL_INT, 1500l},
      {_("Auto indent"), item::TEXTCTRL_INT, (long)INDENT_ALL},
      {_("Caret line"), item::CHECKBOX, true},
      {_("Default font"), item::FONTPICKERCTRL, wxSystemSettings::GetFont(wxSYS_OEM_FIXED_FONT)},
      {_("Divider"), item::TEXTCTRL_INT, 16l},
      {_("Edge column"), item::TEXTCTRL_INT, 80l},
      {_("Edge line"), item::TEXTCTRL_INT, (long)wxSTC_EDGE_NONE},
      {_("Fold flags"), item::TEXTCTRL_INT, (long)wxSTC_FOLDFLAG_LINEBEFORE_CONTRACTED | wxSTC_FOLDFLAG_LINEAFTER_CONTRACTED},
      {_("Folding"), item::TEXTCTRL_INT, 16l},
      {_("Indent"), item::TEXTCTRL_INT, 2l},
      {_("Keep zoom"), item::CHECKBOX, true},
      {_("Line number"), item::TEXTCTRL_INT, 60l},
      {_("Print flags"), item::TEXTCTRL_INT, (long)wxSTC_PRINT_BLACKONWHITE},
      {_("Scroll bars"), item::CHECKBOX, true},
      {_("Search engine"), item::COMBOBOX, std::string("https://duckduckgo.com")},
      {_("Show mode"), item::CHECKBOX, true},
      {_("Tab draw mode"), item::TEXTCTRL_INT, (long)wxSTC_TD_LONGARROW},
      {_("Tab font"), item::FONTPICKERCTRL, wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT)},
      {_("Tab width"), item::TEXTCTRL_INT, 2l},
      {_("Text font"), item::FONTPICKERCTRL, wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT)},
      {_("vi mode"), item::CHECKBOX, true},
      {_("vi tag fullpath"), item::CHECKBOX, false}}) {;};
  };
};
  
bool wex::stc::auto_indentation(int c)
{
  if (const auto ai = config(_("Auto indent")).get(INDENT_ALL);
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

int wex::stc::config_dialog(const window_data& par)
{
  const window_data data(window_data(par).
    title(_("Editor Options").ToStdString()));

  if (m_config_dialog == nullptr)
  {
    stc_defaults use;
  
    m_config_dialog = new item_dialog({{"stc-notebook", {
      {_("General"),
        {{"stc-subnotebook", {
          {_("Page1"), 
            {{{_("End of line"),
               _("Line numbers"),
               _("Use tabs"),
               _("Caret line"),
               _("Scroll bars"),
               _("Auto blame"),
               _("Auto complete"),
               _("Keep zoom"),
               _("vi mode"),
               _("vi tag fullpath")}},
            {_("Search engine"), item::COMBOBOX}}},
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
            ,item::NOTEBOOK_AUI
#endif
            }}},
      {_("Font"), 
        {{_("Default font"), item::FONTPICKERCTRL},
         {_("Tab font"), item::FONTPICKERCTRL},
         {_("Text font"), item::FONTPICKERCTRL}}},
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
        {{_("Indentation guide"), item::CHECKBOX},
         {_("Auto fold"), 0l, INT_MAX},
         {_("Fold flags"), {
             {wxSTC_FOLDFLAG_LINEBEFORE_EXPANDED, _("Line before expanded")},
             {wxSTC_FOLDFLAG_LINEBEFORE_CONTRACTED, _("Line before contracted")},
             {wxSTC_FOLDFLAG_LINEAFTER_EXPANDED, _("Line after expanded")},
             {wxSTC_FOLDFLAG_LINEAFTER_CONTRACTED, _("Line after contracted")},
             {wxSTC_FOLDFLAG_LEVELNUMBERS, _("Level numbers")}},
             false}}},
      {_("Directory"),
        {{_("Include directory"), listview_data().type(listview_data::FOLDER).
          window(window_data().size({200, 200}))}}},
      {_("Printer"),
        {{_("Print flags"), {
           {wxSTC_PRINT_NORMAL, _("Normal")},
           {wxSTC_PRINT_INVERTLIGHT, _("Invert on white")},
           {wxSTC_PRINT_BLACKONWHITE, _("Black on white")},
           {wxSTC_PRINT_COLOURONWHITE, _("Colour on white")},
           {wxSTC_PRINT_COLOURONWHITEDEFAULTBG, _("Colour on white normal")}}, true, 1}}}
      }}},
      window_data(data).
        title(data.id() == wxID_PREFERENCES ? wxGetStockLabel(data.id(), 0).ToStdString(): data.title()));
  }

  return (data.button() & wxAPPLY) ? 
    m_config_dialog->Show(): m_config_dialog->ShowModal();
}

void wex::stc::config_get()
{
  stc_defaults use;
  
  const wxFont font(config(_("Default font")).get( 
    wxSystemSettings::GetFont(wxSYS_ANSI_FIXED_FONT)));

  if (m_DefaultFont != font)
  {
    m_DefaultFont = font;
    
    StyleResetDefault();
    
    // Doing this once is enough, not yet possible.
    lexers::get()->load_document();
    
    m_Lexer.apply();
  }

  if (m_Lexer.edge_mode() == edge_mode_t::ABSENT)
  {
    if (!m_Lexer.is_ok())
    {
      SetEdgeMode(wxSTC_EDGE_NONE);
    }
    else
    {
      SetEdgeColumn(config(_("Edge column")).get(80l));
    
      if (const auto el = config(_("Edge line")).get(wxSTC_EDGE_NONE);
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
  
  AutoCompSetMaxWidth(config(_("Autocomplete maxwidth")).get(0));
  SetCaretLineVisible(config(_("Caret line")).get(true));
  SetFoldFlags(config( _("Fold flags")).get(0));
  SetIndent(config(_("Indent")).get(0));
  SetIndentationGuides( config(_("Indentation guide")).get(false));
  SetMarginWidth(m_MarginDividerNumber, config(_("Divider")).get(0));
  SetPrintColourMode(config(_("Print flags")).get(0));
  SetTabDrawMode(config(_("Tab draw mode")).get(wxSTC_TD_LONGARROW));
  SetTabWidth(config(_("Tab width")).get(0));
  SetUseHorizontalScrollBar(config(_("Scroll bars")).get(true));
  SetUseTabs(config(_("Use tabs")).get(false));
  SetUseVerticalScrollBar(config(_("Scroll bars")).get(true));
  SetViewEOL(config(_("End of line")).get(false));
  SetViewWhiteSpace(config(_("Whitespace visible")).get(wxSTC_WS_INVISIBLE));
  SetWrapMode(config(_("Wrap line")).get(wxSTC_WRAP_NONE));
  SetWrapVisualFlags(config(_("Wrap visual flags")).get( wxSTC_WRAPVISUALFLAG_END));
  m_vi.use(config(_("vi mode")).get(false));

  show_line_numbers(config(_("Line numbers")).get(false));

  m_Link.set_from_config();
}
