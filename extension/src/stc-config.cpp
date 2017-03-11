////////////////////////////////////////////////////////////////////////////////
// Name:      stc-config.cpp
// Purpose:   Implementation of config related methods of class wxExSTC
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <wx/combobox.h>
#include <wx/config.h>
#include <wx/settings.h>
#include <wx/stockitem.h>
#include <wx/extension/stc.h>
#include <wx/extension/itemdlg.h>
#include <wx/extension/lexers.h>

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
  : wxExConfigDefaults(std::vector<std::tuple<wxString, wxExItemType, wxAny>> {
    std::make_tuple(_("Auto fold"), ITEM_TEXTCTRL_INT, 1500),
    std::make_tuple(_("Auto indent"), ITEM_TEXTCTRL_INT, (long)INDENT_ALL),
    std::make_tuple(_("Caret line"), ITEM_CHECKBOX, true),
    std::make_tuple(_("Default font"), ITEM_FONTPICKERCTRL, wxSystemSettings::GetFont(wxSYS_OEM_FIXED_FONT)),
    std::make_tuple(_("Divider"), ITEM_TEXTCTRL_INT, 16),
    std::make_tuple(_("Edge column"), ITEM_TEXTCTRL_INT, 80),
    std::make_tuple(_("Edge line"), ITEM_TEXTCTRL_INT, wxSTC_EDGE_NONE),
    std::make_tuple(_("Fold flags"), ITEM_TEXTCTRL_INT, wxSTC_FOLDFLAG_LINEBEFORE_CONTRACTED | wxSTC_FOLDFLAG_LINEAFTER_CONTRACTED),
    std::make_tuple(_("Folding"), ITEM_TEXTCTRL_INT, 16),
    std::make_tuple(_("Indent"), ITEM_TEXTCTRL_INT, 2),
    std::make_tuple(_("Line number"), ITEM_TEXTCTRL_INT, 60),
    std::make_tuple(_("Print flags"), ITEM_TEXTCTRL_INT, wxSTC_PRINT_BLACKONWHITE),
    std::make_tuple(_("Scroll bars"), ITEM_CHECKBOX, true),
    std::make_tuple(_("Show mode"), ITEM_CHECKBOX, true),
#if wxCHECK_VERSION(3,1,1)
    std::make_tuple(_("Tab draw mode"), ITEM_TEXTCTRL_INT, wxSTC_TD_LONGARROW),
#endif
    std::make_tuple(_("Tab font"), ITEM_FONTPICKERCTRL, wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT)),
    std::make_tuple(_("Tab width"), ITEM_TEXTCTRL_INT, 2),
    std::make_tuple(_("Text font"), ITEM_FONTPICKERCTRL, wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT)),
    std::make_tuple(_("vi mode"), ITEM_CHECKBOX, true)}) {;};
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

int wxExSTC::ConfigDialog(
  wxWindow* parent,
  const wxString& title,
  long flags,
  wxWindowID id)
{
  STCDefaults use;
  wxConfigBase* cfg = use.Get();
  
  const std::vector<wxExItem> items {wxExItem{wxString("stc-notebook"), {
      {_("General"),
        {{wxString("stc-subnotebook"), {
          {_("Page1"), 
            {{{_("End of line"),
               _("Line numbers"),
               _("Use tabs"),
               _("Caret line"),
               _("Scroll bars"),
               _("Auto complete"),
               _("vi mode")}}}},
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
#if wxCHECK_VERSION(3,1,1)
             {_("Tab draw mode"), {
               {wxSTC_TD_LONGARROW, _("Longarrow")},
               {wxSTC_TD_STRIKEOUT, _("Strikeout")}}, true, 2},
#endif
             {_("Whitespace visible"), {
               {wxSTC_WS_INVISIBLE, _("Off")},
               {wxSTC_WS_VISIBLEAFTERINDENT, _("After indent")},
               {wxSTC_WS_VISIBLEALWAYS, _("Always")}
#if wxCHECK_VERSION(3,1,1)
               ,{wxSTC_WS_VISIBLEONLYININDENT, _("Only indent")}},
#else
               },
#endif  
               true, 2},
             {_("Wrap line"), {
               {wxSTC_WRAP_NONE, _("None")},
               {wxSTC_WRAP_WORD, _("Word")},
               {wxSTC_WRAP_CHAR, _("Char")}
#if wxCHECK_VERSION(3,1,0)
              ,{wxSTC_WRAP_WHITESPACE, _("Whitespace")}},
#else
               },
#endif  
              true, 4}}}} 
#ifdef __WXMSW__
            ,ITEM_NOTEBOOK_AUI
#endif
            }}},
      {_("Font"), 
#ifndef __WXOSX__
        {{_("Default font"), ITEM_FONTPICKERCTRL},
         {_("Tab font"), ITEM_FONTPICKERCTRL},
         {_("Text font"), ITEM_FONTPICKERCTRL}}},
#else
        {{_("Default font")},
         {_("Tab font")},
         {_("Text font")}}},
#endif
      {_("Edge"),
        {{_("Edge column"), 0, 500},
         { _("Edge line"), {
           {wxSTC_EDGE_NONE, _("None")},
           {wxSTC_EDGE_LINE, _("Line")},
           {wxSTC_EDGE_BACKGROUND, _("Background")}}, true, 1}}},
      {_("Margin"),
        {{_("Tab width"), 1, (int)cfg->ReadLong(_("Edge column"), 0)},
         {_("Indent"), 0, (int)cfg->ReadLong(_("Edge column"), 0)},
         {_("Divider"), 0, 40},
         {_("Folding"), 0, 40},
         {_("Line number"), 0, 100},
         {_("Auto complete maxwidth"), 0, 100}}},
      {_("Folding"),
        {{_("Indentation guide"), ITEM_CHECKBOX},
         {_("Auto fold"), 0, INT_MAX},
         {_("Fold flags"), {
             {wxSTC_FOLDFLAG_LINEBEFORE_EXPANDED, _("Line before expanded")},
             {wxSTC_FOLDFLAG_LINEBEFORE_CONTRACTED, _("Line before contracted")},
             {wxSTC_FOLDFLAG_LINEAFTER_EXPANDED, _("Line after expanded")},
             {wxSTC_FOLDFLAG_LINEAFTER_CONTRACTED, _("Line after contracted")},
             {wxSTC_FOLDFLAG_LEVELNUMBERS, _("Level numbers")}},
             false}}},
      {_("Printer"),
        {{_("Print flags"), {
           {wxSTC_PRINT_NORMAL, _("Normal")},
           {wxSTC_PRINT_INVERTLIGHT, _("Invert on white")},
           {wxSTC_PRINT_BLACKONWHITE, _("Black on white")},
           {wxSTC_PRINT_COLOURONWHITE, _("Colour on white")},
           {wxSTC_PRINT_COLOURONWHITEDEFAULTBG, _("Colour on white normal")}}, true, 1}}},
      {_("Directory"),
        {{_("Include directory"), ITEM_LISTVIEW, wxAny(), false, wxID_ANY, LABEL_NONE}}}}}};

  int buttons = wxOK | wxCANCEL;

  if (flags & STC_CONFIG_WITH_APPLY)
  {
    buttons |= wxAPPLY;
  }
  
  if (!(flags & STC_CONFIG_MODELESS))
  {
    return wxExItemDialog(parent, items, 
      id == wxID_PREFERENCES ? wxGetStockLabel(id, 0): title, 0, 1, buttons, id).ShowModal();
  }
  else
  {
    if (m_ConfigDialog == nullptr)
    {
      m_ConfigDialog = new wxExItemDialog(parent, items, 
        id == wxID_PREFERENCES ? wxGetStockLabel(id, 0): title, 0, 1, buttons, id, 
        wxDefaultPosition, wxSize(510, 500));
    }

    return m_ConfigDialog->Show();
  }
}

void wxExSTC::ConfigGet(bool init)
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

  if (GetFileName().GetExtension().find("log") == 0)
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
  
  if (init)
  {
    Fold();
  }

  AutoCompSetMaxWidth(cfg->ReadLong(_("Auto complete maxwidth"), 0));
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
