////////////////////////////////////////////////////////////////////////////////
// Name:      stc/config.cpp
// Purpose:   Implementation of config related methods of class wex::stc
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/config.h>
#include <wex/core/core.h>
#include <wex/stc/beautify.h>
#include <wex/stc/entry-dialog.h>
#include <wex/stc/link.h>
#include <wex/syntax/lexers.h>
#include <wex/ui/item-dialog.h>
#include <wex/ui/item-vector.h>
#include <wx/settings.h>
#include <wx/stockitem.h>

namespace wex
{
const std::string def(const std::string& v)
{
  return std::string(v) + ",1";
}
}; // namespace wex

int wex::stc::config_dialog(const data::window& par)
{
  const data::window data(data::window(par).title(_("Editor Options")));

  if (m_config_dialog == nullptr)
  {
    m_config_dialog = new item_dialog(
      *m_config_items,
      data::window(data).title(
        data.id() == wxID_PREFERENCES ?
          wxGetStockLabel(data.id(), 0).ToStdString() :
          data.title()));
  }
  else
  {
    m_config_dialog->reload();
  }

  return (data.button() & wxAPPLY) ? m_config_dialog->Show() :
                                     m_config_dialog->ShowModal();
}

void wex::stc::config_get()
{
  const item_vector iv(m_config_items);

  if (const auto blame_margin(iv.find<int>(_("stc.margin.Text")));
      blame_margin != -1 && margin_text_is_shown())
  {
    SetMarginWidth(m_margin_text_number, blame_margin);
  }

  SetMarginWidth(
    m_margin_divider_number,
    iv.find<int>(_("stc.margin.Divider")));

  if (m_data.flags().test(data::stc::WIN_EX))
  {
    SetUseVerticalScrollBar(false);
  }
  else
  {
    SetUseVerticalScrollBar(iv.find<bool>(_("stc.Scroll bars")));
  }

  if (!iv.find<bool>(_("stc.vi mode")))
  {
    config(_("stc.vi mode")).get(true);

    if (!m_data.flags().test(data::stc::WIN_EX))
    {
      get_vi().use(ex::mode_t::OFF);
    }
  }
  else if (!m_data.flags().test(data::stc::WIN_EX))
  {
    get_vi().use(ex::mode_t::VISUAL);
  }

  show_line_numbers(iv.find<bool>(_("stc.Line numbers")));

  generic_settings();

  get_lexer().apply(); // at end, to prioritize local xml config
}

void wex::stc::generic_settings()
{
  const item_vector iv(m_config_items);

  if (!get_lexer().is_ok())
  {
    SetEdgeMode(wxSTC_EDGE_NONE);
  }
  else
  {
    if (const auto el = iv.find<long>(_("stc.Edge line"));
        el != wxSTC_EDGE_NONE)
    {
      const auto font(iv.find<wxFont>(_("stc.Default font")));
      SetEdgeMode(font.IsFixedWidth() ? el : wxSTC_EDGE_BACKGROUND);
    }
    else
    {
      SetEdgeMode(el);
    }
  }

  SetEdgeColumn(iv.find<int>(_("stc.Edge column")));
  AnnotationSetVisible(iv.find<long>(_("stc.Annotation style")));
  AutoCompSetMaxWidth(iv.find<int>(_("stc.Autocomplete max width")));
  SetCaretLineVisible(iv.find<bool>(_("stc.Caret line")));
  SetFoldFlags(iv.find<long>(_("stc.Fold flags")));
  SetIndent(iv.find<int>(_("stc.Indent")));
  SetIndentationGuides(iv.find<bool>(_("stc.Indentation guide")));
  SetPrintColourMode(iv.find<long>(_("stc.Print flags")));
  SetTabDrawMode(iv.find<long>(_("stc.Tab draw mode")));
  SetTabWidth(iv.find<int>(_("stc.Tab width")));
  SetUseHorizontalScrollBar(iv.find<bool>(_("stc.Scroll bars")));
  SetUseTabs(!iv.find<bool>(_("stc.Expand tabs")));
  SetViewEOL(iv.find<bool>(_("stc.End of line")));
  SetViewWhiteSpace(iv.find<long>(_("stc.Whitespace visible")));
  SetWrapMode(iv.find<long>(_("stc.Wrap line")));
  SetWrapVisualFlags(iv.find<long>(_("stc.Wrap visual flags")));
}

void wex::stc::on_exit()
{
  if (item_vector(m_config_items).find<bool>(_("stc.Keep zoom")))
  {
    config("stc.zoom").set(m_zoom);
  }

  delete m_config_items;
  delete m_link;
}

void wex::stc::on_init()
{
  m_config_items = new std::vector<item>(
    {{"stc-notebook",
      {
        {_("General"),
         {{"stc-subnotebook",
           {{_("Switches"),
             {{{_("stc.End of line"),
                _("stc.Line numbers"),
                def(_("stc.Expand tabs")),
                _("stc.Ex mode show hex"),
                def(_("stc.Caret line")),
                def(_("stc.Scroll bars")),
                _("stc.Auto beautify"),
                _("stc.Auto blame"),
                _("stc.Auto complete"),
                def(_("stc.Auto indent")),
                def(_("stc.Keep zoom")),
                def(_("stc.vi mode")),
                _("stc.vi tag fullpath")}},
              {_("stc.Beautifier"), item::COMBOBOX, beautify().list()},
              {_("stc.Beautifier cmake"), item::COMBOBOX, beautify().list()},
              {_("stc.Search engine"),
               item::COMBOBOX,
               config::strings_t{{"https://duckduckgo.com"}}}}},
            {_("Choices"),
             {{_("stc.Wrap visual flags"),
               {{wxSTC_WRAPVISUALFLAG_NONE, _("None")},
                {wxSTC_WRAPVISUALFLAG_END, _("End")},
                {wxSTC_WRAPVISUALFLAG_START, _("Start")},
                {wxSTC_WRAPVISUALFLAG_MARGIN, _("Margin")}},
               true,
               data::item().columns(4)},
              {_("stc.Tab draw mode"),
               {{wxSTC_TD_LONGARROW, def(_("Longarrow"))},
                {wxSTC_TD_STRIKEOUT, _("Strikeout")}},
               true,
               data::item().columns(2)},
              {_("stc.Whitespace visible"),
               {{wxSTC_WS_INVISIBLE, _("Off")},
                {wxSTC_WS_VISIBLEAFTERINDENT, _("After indent")},
                {wxSTC_WS_VISIBLEALWAYS, _("Always")},
                {wxSTC_WS_VISIBLEONLYININDENT, _("Only indent")}},
               true,
               data::item().columns(2)},
              {_("stc.Annotation style"),
               {{wxSTC_ANNOTATION_HIDDEN, _("Hidden")},
                {wxSTC_ANNOTATION_STANDARD, _("Standard")},
                {wxSTC_ANNOTATION_BOXED, def(_("Boxed"))},
                {wxSTC_ANNOTATION_INDENTED, _("indented")}},
               true,
               data::item().columns(2)},
              {_("stc.Wrap line"),
               {{wxSTC_WRAP_NONE, _("None")},
                {wxSTC_WRAP_WORD, _("Word")},
                {wxSTC_WRAP_CHAR, _("Char")},
                {wxSTC_WRAP_WHITESPACE, _("Whitespace")}},
               true,
               data::item().columns(2)}}},
            {_("Printer"),
             {{_("stc.Print flags"),
               {{wxSTC_PRINT_NORMAL, _("Normal")},
                {wxSTC_PRINT_INVERTLIGHT, _("Invert on white")},
                {wxSTC_PRINT_BLACKONWHITE, def(_("Black on white"))},
                {wxSTC_PRINT_COLOURONWHITE, _("Colour on white")},
                {wxSTC_PRINT_COLOURONWHITEDEFAULTBG,
                 _("Colour on white normal")}},
               true,
               data::item().columns(1)}}}}}}},
        {_("Font"),
         {{_("stc.Default font"),
           item::FONTPICKERCTRL,
           wxFont(
             12,
             wxFONTFAMILY_DEFAULT,
             wxFONTSTYLE_NORMAL,
             wxFONTWEIGHT_NORMAL),
           data::item().apply(
             [=](wxWindow* user, const std::any& value, bool save)
             {
               // Doing this once is enough, not yet possible.
               lexers::get()->load_document();
             })},
          {_("stc.Text font"),
           item::FONTPICKERCTRL,
           wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT)}}},
        {_("Edge"),
         {{_("stc.Edge column"), 0, 500, 80},
          {_("stc.Edge line"),
           {{wxSTC_EDGE_NONE, _("None")},
            {wxSTC_EDGE_LINE, _("Line")},
            {wxSTC_EDGE_BACKGROUND, _("Background")},
            {wxSTC_EDGE_MULTILINE, _("Multiline")}},
           true,
           data::item().columns(1)}}},
        {_("Margin"),
         {{_("<i>Margins:</i>")},
          {_("stc.Autocomplete max width"), 0, 100, 0},
          {_("stc.Autocomplete min size"), 0, 100, 0},
          {_("stc.Indent"), 0, 500, 2},
          {_("stc.Tab width"), 1, 500, 2},
          {_("blame.Author size"), -1, 500, -1},
          {_("<i>Line Margins:</i>")},
          {_("stc.margin.Divider"), 0, 40, 16},
          {_("stc.margin.Folding"), 0, 40, 16},
          {_("stc.margin.Line number"), 0, 100, 60},
          {_("stc.margin.Text"), -1, 500, -1},
          {_("<i>Max:</i>")},
          {_("stc.max.Size visual"),
           item::TEXTCTRL_INT,
           std::string("1000000")},
          {_("stc.max.Size lexer"), item::TEXTCTRL_INT, std::string("1000000")},
          {_("Repeater"), item::TEXTCTRL_INT, std::string("1000")}}},
        {_("Folding"),
         {{_("stc.Indentation guide"), item::CHECKBOX},
          {_("stc.Auto fold"), 0, INT_MAX, 1500},
          {_("stc.Fold flags"),
           {{wxSTC_FOLDFLAG_LINEBEFORE_EXPANDED, _("Line before expanded")},
            {wxSTC_FOLDFLAG_LINEBEFORE_CONTRACTED,
             def(_("Line before contracted"))},
            {wxSTC_FOLDFLAG_LINEAFTER_EXPANDED, _("Line after expanded")},
            {wxSTC_FOLDFLAG_LINEAFTER_CONTRACTED,
             def(_("Line after contracted"))},
            {wxSTC_FOLDFLAG_LEVELNUMBERS, _("Level numbers")}},
           false}}},
        {_("Linking"),
         {{_("<i>Includes:</i>")},
          {_("stc.link.Include directory"),
           data::listview()
             .type(data::listview::FOLDER)
             .window(data::window().size({200, 200})),
           std::any(),
           data::item()
             .label_type(data::item::LABEL_NONE)
             .apply(
               [=](wxWindow* user, const std::any& value, bool save)
               {
                 m_link->config_get();
               })},
          {_("<i>Matches:</i>")},
          {_("stc.link.Pairs"),
           data::listview()
             .type(data::listview::TSV)
             .window(data::window().size({200, 200})),
           // First try to find "..", then <..>, as in next example:
           // <A HREF="http://www.scintilla.org">scintilla</A> component.
           config::strings_t(
             {"\"\t\"", "`\t`", "<\t>", "[\t]", "'\t'", "{\t}"})}}},
      }}});

  if (item_vector(m_config_items).find<bool>(_("stc.Keep zoom")))
  {
    m_zoom = config("stc.zoom").get(-1);
  }

  m_link = new link();
}
